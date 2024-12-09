/**
 *******************************************************************************
 * @file i2c-display.cpp
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

#include <thread>
#include <chrono>
#include <functional>

#include <devices/i2c-display.hpp>

using namespace std::chrono_literals;

namespace Airsoft::Devices {

//  40 us is a save value at any speed.
//  20 us is a save value for I2C at 400K.
constexpr uint8_t I2C_LCD_CHAR_DELAY = 0;

///////////////////////////////////////////////////////
//
//  DO NOT CHANGE BELOW THIS LINE
//
//  keep defines compatible / recognizable
//  the zero valued defines are not used.
constexpr uint8_t I2C_LCD_CLEARDISPLAY        = 0x01;
constexpr uint8_t I2C_LCD_RETURNHOME          = 0x02;
constexpr uint8_t I2C_LCD_ENTRYMODESET        = 0x04;
constexpr uint8_t I2C_LCD_DISPLAYCONTROL      = 0x08;
constexpr uint8_t I2C_LCD_CURSORSHIFT         = 0x10;
constexpr uint8_t I2C_LCD_FUNCTIONSET         = 0x20;
constexpr uint8_t I2C_LCD_SETCGRAMADDR        = 0x40;
constexpr uint8_t I2C_LCD_SETDDRAMADDR        = 0x80;

constexpr uint8_t I2C_LCD_ENTRYLEFT           = 0x02;
constexpr uint8_t I2C_LCD_ENTRYSHIFTINCREMENT = 0x01;

constexpr uint8_t I2C_LCD_DISPLAYON           = 0x04;
constexpr uint8_t I2C_LCD_CURSORON            = 0x02;
constexpr uint8_t I2C_LCD_BLINKON             = 0x01;

constexpr uint8_t I2C_LCD_DISPLAYMOVE         = 0x08;
constexpr uint8_t I2C_LCD_MOVERIGHT           = 0x04;

constexpr uint8_t I2C_LCD_8BITMODE            = 0x10;
constexpr uint8_t I2C_LCD_2LINE               = 0x08;
constexpr uint8_t I2C_LCD_5x10DOTS            = 0x04;

//------------------------------------------------------------------------------
I2CDisplay::I2CDisplay(Drivers::I2C * wire, uint8_t address) {
  _address = address;
  _wire = wire;
  _displayControl = I2C_LCD_DISPLAYCONTROL;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void I2CDisplay::Config (uint8_t address, uint8_t enable, uint8_t readWrite, uint8_t registerSelect,
                      uint8_t data4, uint8_t data5, uint8_t data6, uint8_t data7,
                      uint8_t backLight, uint8_t polarity) {
  if (_address != address) {
    return;
  }

  _enable         = ( 1 << enable);
  _readWrite      = ( 1 << readWrite);
  _registerSelect = ( 1 << registerSelect);
  _dataPin[0]     = ( 1 << data4);
  _dataPin[1]     = ( 1 << data5);
  _dataPin[2]     = ( 1 << data6);
  _dataPin[3]     = ( 1 << data7);
  _backLightPin   = ( 1 << backLight);
  _backLightPol   = polarity;

  _pin4567 = ((data4 == 4) && (data5 == 5) && (data6 == 6) && (data7 == 7));

  // If pins are 0,1,2,3 they are also in order but the shift/mask in send()
  // should be different 4,5,6,7 is most used afaik.
}
//------------------------------------------------------------------------------
bool I2CDisplay::Begin(uint8_t cols, uint8_t rows) {
  // No check for range, user responsibility, defaults are 20x4
  _cols = cols;
  _rows = rows;

  if (!IsConnected()) {
    return false;
  }

  // All lines low
  _wire->BeginTransmission(_address);
  _wire->Write(0x00);
  _wire->EndTransmission();

  // Figure 24 for procedure on 4-bit initialization wait for more than 15 ms
  // if other objects initialize earlier there will be less blocking time.
  // => assumes display is started at same time as MCU
  std::this_thread::sleep_for(100ms);

  // Force 4 bit mode, see datasheet.
  // Times are taken longer for robustness.
  // Note: this is typically only called once.
  Write4bits(0x03);
  std::this_thread::sleep_for(5ms);
  Write4bits(0x03);
  std::this_thread::sleep_for(200us);
  Write4bits(0x03);
  std::this_thread::sleep_for(200us);

  // Command to set 4 bit interface
  Write4bits(0x02);
  std::this_thread::sleep_for(200us);

  // Set "two" lines LCD - fixed for now.
  SendCommand(I2C_LCD_FUNCTIONSET | I2C_LCD_2LINE);

  // Default enable display
  Display();
  Clear();

  return true;
}
//------------------------------------------------------------------------------
bool I2CDisplay::IsConnected(void) {
  _wire->BeginTransmission(_address);
  return _wire->EndTransmission() == 0;
}
//------------------------------------------------------------------------------
void I2CDisplay::SetBacklightPin(uint8_t pin, uint8_t polarity) {
  _backLightPin = (1 << pin);
  _backLightPol = polarity;
}
//------------------------------------------------------------------------------
void I2CDisplay::SetBacklight(bool on) {
  _backLight = (on == _backLightPol);

  if (_backLight) {
    Display();
  } else{
    NoDisplay();
  }
}
//------------------------------------------------------------------------------
void I2CDisplay::Display(void) {
  _displayControl |= I2C_LCD_DISPLAYON;
  SendCommand(_displayControl);
}
//------------------------------------------------------------------------------
void I2CDisplay::NoDisplay(void) {
  _displayControl &= ~I2C_LCD_DISPLAYON;
  SendCommand(_displayControl);
}
//------------------------------------------------------------------------------
void I2CDisplay::Clear(void) {
  SendCommand(I2C_LCD_CLEARDISPLAY);
  _pos = 0;
  std::this_thread::sleep_for(2ms);
}
//------------------------------------------------------------------------------
void I2CDisplay::ClearEOL(void) {
  while(_pos  < _cols) {
    print(' ');
  }
}
//------------------------------------------------------------------------------
void I2CDisplay::Home(void) {
  SendCommand(I2C_LCD_RETURNHOME);
  _pos = 0;
  std::this_thread::sleep_for(1600us);  // Datasheet states 1520.
}
//------------------------------------------------------------------------------
bool I2CDisplay::SetCursor(uint8_t col, uint8_t row) {
  if ((col >= _cols) || (row >= _rows)) {
    return false;
  }

  // More efficient address / offset calculation (no lookup so far).
  uint8_t offset {};

  if (row & 0x01) {
    offset += 0x40;
  }
  if (row & 0x02) {
    offset += _cols;
  }

  offset += col;
  _pos = col;

  SendCommand(I2C_LCD_SETDDRAMADDR | offset );

  return true;
}
//------------------------------------------------------------------------------
void I2CDisplay::Blink(void) {
  _displayControl |= I2C_LCD_BLINKON;
  SendCommand(_displayControl);
}
//------------------------------------------------------------------------------
void I2CDisplay::NoBlink(void) {
  _displayControl &= ~I2C_LCD_BLINKON;
  SendCommand(_displayControl);
}
//------------------------------------------------------------------------------
void I2CDisplay::Cursor(void) {
  _displayControl |= I2C_LCD_CURSORON;
  SendCommand(_displayControl);
}
//------------------------------------------------------------------------------
void I2CDisplay::NoCursor(void) {
  _displayControl &= ~I2C_LCD_CURSORON;
  SendCommand(_displayControl);
}
//------------------------------------------------------------------------------
void I2CDisplay::ScrollDisplayLeft(void) {
  SendCommand(I2C_LCD_CURSORSHIFT | I2C_LCD_DISPLAYMOVE);
}
//------------------------------------------------------------------------------
void I2CDisplay::ScrollDisplayRight(void) {
  SendCommand(I2C_LCD_CURSORSHIFT | I2C_LCD_DISPLAYMOVE | I2C_LCD_MOVERIGHT);
}
//------------------------------------------------------------------------------
void I2CDisplay::MoveCursorLeft(uint8_t n) {
  while ((_pos > 0) && (n--)) {
    SendCommand(I2C_LCD_CURSORSHIFT);
    _pos--;
  }
}
//------------------------------------------------------------------------------
void I2CDisplay::MoveCursorRight(uint8_t n) {
  while ((_pos < _cols) && (n--)) {
    SendCommand(I2C_LCD_CURSORSHIFT | I2C_LCD_MOVERIGHT);
    _pos++;
  }
}
//------------------------------------------------------------------------------
void I2CDisplay::Autoscroll(void) {
  SendCommand(I2C_LCD_ENTRYMODESET | I2C_LCD_ENTRYSHIFTINCREMENT);
}
//------------------------------------------------------------------------------
void I2CDisplay::NoAutoscroll(void) {
  SendCommand(I2C_LCD_ENTRYMODESET);
}
//------------------------------------------------------------------------------
void I2CDisplay::LeftToRight(void) {
  SendCommand(I2C_LCD_ENTRYMODESET | I2C_LCD_ENTRYLEFT);
}
//------------------------------------------------------------------------------
void I2CDisplay::RightToLeft(void) {
  SendCommand(I2C_LCD_ENTRYMODESET);
}
//------------------------------------------------------------------------------
void I2CDisplay::CreateChar(uint8_t index, uint8_t * charmap) {
  SendCommand(I2C_LCD_SETCGRAMADDR | ((index & 0x07) << 3));

  uint8_t tmp { _pos };

  for (uint8_t i {}; i < 8; i++) {
    _pos = 0;
    SendData(charmap[i]);
  }
  _pos = tmp;
}
//------------------------------------------------------------------------------
size_t I2CDisplay::Write(uint8_t c) {
  size_t n {};

  // Handle TAB char
  if (c == (uint8_t)'\t') {
    while (((_pos % 4) != 0) && (_pos < _cols)) {
      MoveCursorRight();   // Increases _pos.
      n++;
    }

    return n;
  }

  // Overflow protect.
  if (_pos < _cols) {
    SendData(c);

    _pos++;

    return 1;
  }

  // Not allowed to print beyond display, so return 0.
  return 0;
}
//------------------------------------------------------------------------------
size_t I2CDisplay::Write(const uint8_t * buffer, size_t size) {
  uint32_t c;

  for (c = 0; c < size; c++) {
    Write(buffer[c]);
  }

  return c;
}
//------------------------------------------------------------------------------
size_t I2CDisplay::Center(uint8_t row, const char * message) {
  size_t len { strlen(message) + 1 };

  SetCursor((_cols - len) / 2, row);

  return print(message);
}
//------------------------------------------------------------------------------
size_t I2CDisplay::Right(uint8_t col, uint8_t row, const char * message) {
  size_t len { strlen(message) };

  SetCursor(col - len, row);

  return print(message);
}
//------------------------------------------------------------------------------
size_t I2CDisplay::Repeat(uint8_t c, uint8_t times) {
  size_t n {};

  while((times--) && (_pos < _cols)) {
    n += Write(c);
  }

  return n;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void I2CDisplay::SendCommand(uint8_t value) {
  Send(value, false);
}
//------------------------------------------------------------------------------
void I2CDisplay::SendData(uint8_t value) {
  Send(value, true);
}
//------------------------------------------------------------------------------
void I2CDisplay::Send(uint8_t value, bool dataFlag) {
  // Calculate both
  // MSN == most significant nibble and
  // LSN == least significant nibble
  uint8_t MSN {};

  if (dataFlag) {
    MSN = _registerSelect;
  }

  if (_backLight) {
    MSN |= _backLightPin;
  }

  uint8_t LSN { MSN };

  // 4,5,6,7 only == most used.
  if (_pin4567) {
    MSN |= value & 0xF0;
    LSN |= value << 4;
  } else {  // ~ 1.7% slower UNO.  (adds 4 us / char)
    for (uint8_t i {}; i < 4; i++ ) {
      if (value & 0x01 ) {
        LSN |= _dataPin[i];
      }
      if (value & 0x10) {
        MSN |= _dataPin[i];
      }
      value >>= 1;
    }
  }

  _wire->BeginTransmission(_address);
  _wire->Write(MSN | _enable);
  _wire->Write(MSN);
  _wire->Write(LSN | _enable);
  _wire->Write(LSN);
  _wire->EndTransmission();
  if (I2C_LCD_CHAR_DELAY) {
    std::this_thread::sleep_for(std::chrono::microseconds(I2C_LCD_CHAR_DELAY));
  }
}
//------------------------------------------------------------------------------
void I2CDisplay::Write4bits(uint8_t value) {
  uint8_t cmd {};

  for (uint8_t i {}; i < 4; i++ ) {
    if (value & 0x01) {
      cmd |= _dataPin[i];
    }
    value >>= 1;
  }

  _wire->BeginTransmission(_address);
  _wire->Write(cmd | _enable);
  _wire->EndTransmission();
  _wire->BeginTransmission(_address);
  _wire->Write(cmd);
  _wire->EndTransmission();
}
//------------------------------------------------------------------------------

} // namespace Airsoft::Devices
