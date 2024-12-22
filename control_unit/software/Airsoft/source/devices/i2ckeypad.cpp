/**
 *******************************************************************************
 * @file i2ckeypad.cpp
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

#include <utility.hpp>
#include <devices/i2ckeypad.hpp>

namespace Airsoft::Devices {

//------------------------------------------------------------------------------
I2CKeyPad::I2CKeyPad(Drivers::I2C * wire, const uint8_t deviceAddress) {
  _lastKey = I2C_KEYPAD_NOKEY;
  _address = deviceAddress;
  _wire = wire;
  _mode = I2C_KEYPAD_4x4;
  _debounceThreshold = 100;
  _lastTimeRead = 0;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
bool I2CKeyPad::Begin() {
  // Enable interrupts
  _read(0xF0);

  return IsConnected();
}
//------------------------------------------------------------------------------
bool I2CKeyPad::IsConnected() {
  _wire->BeginTransmission(_address);
  return (_wire->EndTransmission() == 0);
}
//------------------------------------------------------------------------------
uint8_t I2CKeyPad::GetKey(void) {
  uint64_t now = Utility::TimeSinceEpochMillisec();

  if (_debounceThreshold > 0) {
    if (now - _debounceThreshold < _lastTimeRead) {
      return I2C_KEYPAD_THRESHOLD;
    }
  }

  uint8_t key {};

  if (_mode == I2C_KEYPAD_5x3) {
    key = _getKey5x3();
  } else if (_mode == I2C_KEYPAD_6x2) {
    key = _getKey6x2();
  } else if (_mode == I2C_KEYPAD_8x1) {
    key = _getKey8x1();
  } else {
    key = _getKey4x4();  //  default.
  }

  if (key == I2C_KEYPAD_FAIL) {
    return key;
  }

  // valid keys + NOKEY
  _lastKey = key;
  _lastTimeRead = now;

  if (key == I2C_KEYPAD_NOKEY) {
    _pressed = 0;
  } else {
    if (_pressed == 0) {
      _pressed++;
    } else {
      key = I2C_KEYPAD_NOKEY;
    }
  }

  return key;
}
//------------------------------------------------------------------------------
uint8_t I2CKeyPad::GetLastKey(void) {
  return _lastKey;
}
//------------------------------------------------------------------------------
bool I2CKeyPad::IsPressed(void) {
  uint8_t a = _read(0xF0);

  if (a == 0xFF) {
    return false;
  }

  return a != 0xF0;
}
//------------------------------------------------------------------------------
uint8_t I2CKeyPad::GetChar(void) {
  uint8_t key = GetKey();
  if (_keyMap == nullptr) {
    return I2C_KEYPAD_THRESHOLD;
  }

  if (key == I2C_KEYPAD_NOKEY) {
    return I2C_KEYPAD_NOKEY;
  }

  if (key != I2C_KEYPAD_THRESHOLD) {
    return _keyMap[key];
  }

  return I2C_KEYPAD_THRESHOLD;
}
//------------------------------------------------------------------------------
uint8_t I2CKeyPad::GetLastChar(void) {
  return _keyMap[_lastKey];
}
//------------------------------------------------------------------------------
void I2CKeyPad::LoadKeyMap(char *keyMap) {
  _keyMap = keyMap;
}
//------------------------------------------------------------------------------
void I2CKeyPad::SetKeyPadMode(uint8_t mode) {
  if ((mode == I2C_KEYPAD_5x3) || (mode == I2C_KEYPAD_6x2) || (mode == I2C_KEYPAD_8x1)) {
    _mode = mode;
    return;
  }

  _mode = I2C_KEYPAD_4x4;
}
//------------------------------------------------------------------------------
uint8_t I2CKeyPad::GetKeyPadMode() {
  return _mode;
}
//------------------------------------------------------------------------------
void I2CKeyPad::SetDebounceThreshold(uint16_t value) {
  _debounceThreshold = value;
}
//------------------------------------------------------------------------------
uint16_t I2CKeyPad::GetDebounceThreshold() {
  return _debounceThreshold;
}
//------------------------------------------------------------------------------
uint64_t I2CKeyPad::GetLastTimeRead() {
  return _lastTimeRead;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
uint8_t I2CKeyPad::_read(uint8_t mask) {
  _wire->BeginTransmission(_address);
  _wire->Write(mask);
  uint8_t rdData {};
  _wire->Read(&rdData, 1, 1);
  _wire->EndTransmission();

  return rdData;
}
//------------------------------------------------------------------------------
uint8_t I2CKeyPad::_getKey4x4() {
  uint8_t key = 0;    // key = row + 4 x column

  // Mask = 4 rows as input pull up, 4 columns as output
  uint8_t rows = _read(0xF0);

  //  check if single line has gone low.
  if (rows == 0xF0) {
    return I2C_KEYPAD_NOKEY;
  } else if (rows == 0xE0) {
    key = 0;
  } else if (rows == 0xD0) {
    key = 1;
  } else if (rows == 0xB0) {
    key = 2;
  } else if (rows == 0x70) {
    key = 3;
  } else {
    return I2C_KEYPAD_FAIL;
  }

  // 4 columns as input pull up, 4 rows as output
  uint8_t cols = _read(0x0F);

  // Check if single line has gone low.
  if (cols == 0x0F) {
    return I2C_KEYPAD_NOKEY;
  } else if (cols == 0x0E) {
    key += 0;
  } else if (cols == 0x0D) {
    key += 4;
  } else if (cols == 0x0B) {
    key += 8;
  } else if (cols == 0x07) {
    key += 12;
  } else {
    return I2C_KEYPAD_FAIL;
  }

  return key;
}
//------------------------------------------------------------------------------
uint8_t I2CKeyPad::_getKey5x3() {
  uint8_t key = 0;  // key = row + 5 x column

  // Mask = 5 rows as input pull up, 3 columns as output
  uint8_t rows = _read(0xF8);

  // Check if single line has gone low.
  if (rows == 0xF8) {
    return I2C_KEYPAD_NOKEY;
  } else if (rows == 0xF0) {
    key = 0;
  } else if (rows == 0xE8) {
    key = 1;
  } else if (rows == 0xD8) {
    key = 2;
  } else if (rows == 0xB8) {
    key = 3;
  } else if (rows == 0x78) {
    key = 4;
  } else {
    return I2C_KEYPAD_FAIL;
  }

  // 3 columns as input pull up, 5 rows as output
  uint8_t cols = _read(0x07);

  // Check if single line has gone low.
  if (cols == 0x07) {
    return I2C_KEYPAD_NOKEY;
  } else if (cols == 0x06) {
    key += 0;
  } else if (cols == 0x05) {
    key += 5;
  } else if (cols == 0x03) {
    key += 10;
  } else {
    return I2C_KEYPAD_FAIL;
  }

  return key;   //  0..14
}
//------------------------------------------------------------------------------
uint8_t I2CKeyPad::_getKey6x2() {
  uint8_t key = 0;    // key = row + 6 x column

  // Mask = 6 rows as input pull up, 2 columns as output
  uint8_t rows = _read(0xFC);

  // Check if single line has gone low.
  if (rows == 0xFC) {
    return I2C_KEYPAD_NOKEY;
  } else if (rows == 0xF8) {
    key = 0;
  } else if (rows == 0xF4) {
    key = 1;
  } else if (rows == 0xEC) {
    key = 2;
  } else if (rows == 0xDC) {
    key = 3;
  } else if (rows == 0xBC) {
    key = 4;
  } else if (rows == 0x7C) {
    key = 5;
  } else {
    return I2C_KEYPAD_FAIL;
  }

  // 2 columns as input pull up, 6 rows as output
  uint8_t cols = _read(0x03);

  // Check if single line has gone low.
  if (cols == 0x03) {
    return I2C_KEYPAD_NOKEY;
  } else if (cols == 0x02) {
    key += 0;
  } else if (cols == 0x01) {
    key += 6;
  } else {
    return I2C_KEYPAD_FAIL;
  }

  return key;   //  0..11
}
//------------------------------------------------------------------------------
uint8_t I2CKeyPad::_getKey8x1() {
  uint8_t key = 0;    //  key = row

  // Mask = 8 rows as input pull up, 0 columns as output
  uint8_t rows = _read(0xFF);

  // Check if single line has gone low.
  if (rows == 0xFF) {
    return I2C_KEYPAD_NOKEY;
  } else if (rows == 0xFE) {
    key = 0;
  } else if (rows == 0xFD) {
    key = 1;
  } else if (rows == 0xFB) {
    key = 2;
  } else if (rows == 0xF7) {
    key = 3;
  } else if (rows == 0xEF) {
    key = 4;
  } else if (rows == 0xDF) {
    key = 5;
  } else if (rows == 0xBF) {
    key = 6;
  } else if (rows == 0x7F) {
    key = 7;
  } else {
    return I2C_KEYPAD_FAIL;
  }

  return key;
}
//------------------------------------------------------------------------------

} // namespace Airsoft::Devices
