/**
 *******************************************************************************
 * @file i2c-display.hpp
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

#ifndef DEVICES_I2C_DISPLAY_HPP_
#define DEVICES_I2C_DISPLAY_HPP_

#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstdbool>
#include <string>
#include <cstring>
#include <sstream>

#include <classes/print.hpp>
#include <drivers/i2c.hpp>

namespace Airsoft::Devices {

#define I2C_LCD_LIB_VERSION     ("0.0.1"))

constexpr uint8_t POSITIVE = 1;
constexpr uint8_t NEGATIVE = 0;

class I2CDisplay final : public Airsoft::Classes::Print {
public:
  I2CDisplay(Drivers::I2C * wire, uint8_t address);
  virtual ~I2CDisplay() = default;

public:
  // Adjust pins
  void Config(uint8_t address, uint8_t enable, uint8_t readWrite, uint8_t registerSelect,
              uint8_t data4, uint8_t data5, uint8_t data6, uint8_t data7,
              uint8_t backLight, uint8_t polarity);

  // Only supports 5x8 char set for now.
  // Blocks up to 100 milliseconds to give LCD time to boot
  bool Begin(uint8_t cols = 20, uint8_t rows = 4);
  bool IsConnected(void);

  //  BACKLIGHT
  void SetBacklightPin(uint8_t pin, uint8_t polarity);
  void SetBacklight(bool on);
  void Backlight(void) {
    SetBacklight(true);
  }
  void NoBacklight(void) {
    SetBacklight(false);
  }


  //  DISPLAY ON OFF
  void Display(void);
  void NoDisplay(void);
  void On(void) {
    Display();
  }
  void Off(void) {
    NoDisplay();
  }

  //  POSITIONING & CURSOR
  void Clear(void);      // Clears whole screen
  void ClearEOL(void);   // Clears line from current pos.
  void Home(void);
  bool SetCursor(uint8_t col, uint8_t row);

  void NoBlink(void);
  void Blink(void);
  void NoCursor(void);
  void Cursor(void);

  void ScrollDisplayLeft(void);
  void ScrollDisplayRight(void);
  void MoveCursorRight(uint8_t n = 1);
  void MoveCursorLeft(uint8_t n = 1);

  // Next 4 limited support
  void Autoscroll(void);
  void NoAutoscroll(void);
  void LeftToRight(void);
  void RightToLeft(void);

  // 8 definable characters
  void CreateChar(uint8_t index, uint8_t * charmap);
  // Clean way to print them
  inline size_t Special(uint8_t index) {
    return Write((uint8_t)index);
  }

  // PRINT INTERFACE ++
  size_t Write(uint8_t c);
  size_t Write(const uint8_t * buffer, size_t size);
  size_t Center(uint8_t row, const char * message);
  size_t Right(uint8_t col, uint8_t row, const char * message);
  size_t Repeat(uint8_t c, uint8_t times);

  // DEBUG development
  uint8_t GetColumn(void) {
    return _pos;
  }
  uint32_t GetWriteCount(void)  {
    return _count;
  }

private:

  void SendData(uint8_t value);
  void SendCommand(uint8_t value);
  void Send(uint8_t value, bool dataFlag);
  void Write4bits(uint8_t value);

  uint8_t         _address {};
  Drivers::I2C *  _wire {};

  uint8_t         _enable         { 4 };
  uint8_t         _readWrite      { 2 };
  uint8_t         _registerSelect { 1 };

  uint8_t         _dataPin[4] { 16, 32, 64, 128 };  //  == pin 4, 5, 6, 7

  // Minor optimization only for pins = 4,5,6,7
  bool            _pin4567 = true;


  uint8_t         _backLightPin   { 8 };
  uint8_t         _backLightPol   { 1 };
  uint8_t         _backLight      { 1 };

  uint8_t         _cols { 20 };
  uint8_t         _rows { 4 };

  // DISPLAYCONTROL bit always on, set in constructor.
  uint8_t   _displayControl {};

  // Overflow protection
  uint8_t   _pos {};

  uint32_t  _count {};

};

} // namespace Airsoft::Devices

#endif // DEVICES_I2C_DISPLAY_HPP_
