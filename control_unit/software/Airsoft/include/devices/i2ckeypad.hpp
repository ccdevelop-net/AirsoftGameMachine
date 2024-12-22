/**
 *******************************************************************************
 * @file i2ckeypad.hpp
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

#ifndef _I2CKEYPAD_HPP_
#define _I2CKEYPAD_HPP_

#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstdbool>
#include <string>
#include <cstring>
#include <sstream>

#include <drivers/i2c.hpp>

namespace Airsoft::Devices {

#define I2C_KEYPAD_LIB_VERSION    ("0.0.1"))

#define I2C_KEYPAD_NOKEY          16
#define I2C_KEYPAD_FAIL           17
//
#define I2C_KEYPAD_THRESHOLD      255

//  experimental
#define I2C_KEYPAD_4x4            44
#define I2C_KEYPAD_5x3            53
#define I2C_KEYPAD_6x2            62
#define I2C_KEYPAD_8x1            81

class I2CKeyPad final {
public:
  I2CKeyPad(Drivers::I2C * wire, const uint8_t deviceAddress);

  bool Begin(void);
  bool IsConnected(void);

  // Get raw key's 0..15
  uint8_t GetKey(void);
  uint8_t GetLastKey(void);
  bool IsPressed(void);

  //  Get 'translated' keys
  //  User must load KeyMap, there is no check.
  uint8_t GetChar(void);
  uint8_t GetLastChar(void);
  void LoadKeyMap(char * keyMap);

  void SetKeyPadMode(uint8_t mode = I2C_KEYPAD_4x4);
  uint8_t GetKeyPadMode(void);

  // Value in milliseconds, max 65535 ms
  void SetDebounceThreshold(uint16_t value = 0);
  uint16_t GetDebounceThreshold(void);
  uint64_t GetLastTimeRead(void);

private:
  uint8_t         _address {};
  uint8_t         _lastKey {};
  uint8_t         _mode { I2C_KEYPAD_4x4 };
  uint16_t        _debounceThreshold { 100 };
  uint64_t        _lastTimeRead {};
  uint32_t        _pressed {};

  Drivers::I2C  * _wire {};

  char    *       _keyMap {};

private:
  uint8_t  _read(uint8_t mask);
  uint8_t  _getKey4x4();
  uint8_t  _getKey5x3();
  uint8_t  _getKey6x2();
  uint8_t  _getKey8x1();

};

} // namespace Airsoft::Devices

#endif // _I2CKEYPAD_HPP_
