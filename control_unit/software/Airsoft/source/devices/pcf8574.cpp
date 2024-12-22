/**
 *******************************************************************************
 * @file pcf8574.cpp
 *
 * @brief Description
 *
 * @author  Cristian
 *
 * @version 1.00
 *
 * @date Dec 9, 2024
 *
 *******************************************************************************
 * This file is part of the Airsoft project 
 * https://github.com/xxxx or http://xxx.github.io.
 * Copyright (c) 2024 CCDevelop.NET
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */

#include <devices/pcf8574.hpp>

namespace Airsoft::Devices {


PCF8574::PCF8574(Drivers::I2C * wire, const uint8_t deviceAddress)
: _address {deviceAddress}, _wire {wire} {

}

//------------------------------------------------------------------------------
bool PCF8574::Begin(uint8_t value) {
  if (!IsConnected()) {
    return false;
  }

  Write8(value);

  return true;
}
//------------------------------------------------------------------------------
bool PCF8574::IsConnected() {
  _wire->BeginTransmission(_address);
  return _wire->EndTransmission() == 0;
}
//------------------------------------------------------------------------------
bool PCF8574::SetAddress(const uint8_t deviceAddress) {
  _address = deviceAddress;
  return IsConnected();
}

//  removed _wire->beginTransmission(_address);
//  with    @100 KHz -> 265 micros()
//  without @100 KHz -> 132 micros()
//  without @400 KHz -> 52 micros()
//  TODO    @800 KHz -> ??
uint8_t PCF8574::Read8(void) {
  _wire->BeginTransmission(_address);
  _wire->Read(&_dataIn, 1, 1);
  _wire->EndTransmission();
  return _dataIn;
}
//------------------------------------------------------------------------------
void PCF8574::Write8(const uint8_t value) {
  _dataOut = value;
  _wire->BeginTransmission(_address);
  _wire->Write(_dataOut);
  _error = _wire->EndTransmission();
}
//------------------------------------------------------------------------------
uint8_t PCF8574::Read(const uint8_t pin) {
  if (pin > 7) {
    _error = PCF8574_PIN_ERROR;
    return 0;
  }
  Read8();
  return (_dataIn & (1 << pin)) > 0;
}
//------------------------------------------------------------------------------
void PCF8574::Write(const uint8_t pin, const bool value) {
  if (pin > 7) {
    _error = PCF8574_PIN_ERROR;
    return;
  }

  if (!value) {
    _dataOut &= ~(1 << pin);
  } else {
    _dataOut |= (1 << pin);
  }

  Write8(_dataOut);
}
//------------------------------------------------------------------------------
void PCF8574::Toggle(const uint8_t pin) {
  if (pin > 7) {
    _error = PCF8574_PIN_ERROR;
    return;
  }
  ToggleMask(1 << pin);
}
//------------------------------------------------------------------------------
void PCF8574::ToggleMask(const uint8_t mask) {
  _dataOut ^= mask;
  Write8(_dataOut);
}
//------------------------------------------------------------------------------
void PCF8574::ShiftRight(const uint8_t n) {
  if ((n == 0) || (_dataOut == 0)) {
    return;
  }

  if (n > 7) {
    _dataOut = 0;     // Shift 8++ clears all, valid...
  }

  if (_dataOut != 0) {
    _dataOut >>= n;   // Only shift if there are bits set
  }

  Write8(_dataOut);
}
//------------------------------------------------------------------------------
void PCF8574::ShiftLeft(const uint8_t n) {
  if ((n == 0) || (_dataOut == 0)) {
    return;
  }

  if (n > 7) {
    _dataOut = 0;    // Shift 8++ clears all, valid...
  }

  if (_dataOut != 0) {
    _dataOut <<= n;  // Only shift if there are bits set
  }

  Write8(_dataOut);
}
//------------------------------------------------------------------------------
int32_t PCF8574::LastError(void) {
  int e = _error;
  _error = PCF8574_OK;  // Reset error after read, is this wise?
  return e;
}
//------------------------------------------------------------------------------
void PCF8574::RotateRight(const uint8_t n) {
  uint8_t r = n & 7;

  if (r == 0) {
    return;
  }

  _dataOut = (_dataOut >> r) | (_dataOut << (8 - r));

  Write8(_dataOut);
}
//------------------------------------------------------------------------------
void PCF8574::RotateLeft(const uint8_t n) {
  RotateRight(8 - (n & 7));
}
//------------------------------------------------------------------------------
void PCF8574::Reverse(void) {
  uint8_t x = _dataOut;

  x = (((x & 0xAA) >> 1) | ((x & 0x55) << 1));
  x = (((x & 0xCC) >> 2) | ((x & 0x33) << 2));
  x =          ((x >> 4) | (x << 4));

  Write8(x);
}
//------------------------------------------------------------------------------
uint8_t PCF8574::ReadButton8(const uint8_t mask) {
  uint8_t temp = _dataOut;

  Write8(mask | _dataOut);  // Read only selected lines
  Read8();
  Write8(temp);             // Restore

  return _dataIn;
}
//------------------------------------------------------------------------------
uint8_t PCF8574::ReadButton(const uint8_t pin) {
  if (pin > 7) {
    _error = PCF8574_PIN_ERROR;
    return 0;
  }

  uint8_t temp = _dataOut;

  Write(pin, true);

  uint8_t value = Read(pin);

  Write8(temp);

  return value;
}
//------------------------------------------------------------------------------
void PCF8574::Select(const uint8_t pin) {
  uint8_t n = 0x00;

  if (pin < 8) {
    n = 1 << pin;
  }

  Write8(n);
}
//------------------------------------------------------------------------------
void PCF8574::SelectN(const uint8_t pin) {
  uint8_t n = 0xFF;

  if (pin < 8) {
    n = (2 << pin) - 1;
  }

  Write8(n);
}
//------------------------------------------------------------------------------

} // namespace Airsoft::Devices
