/**
 *******************************************************************************
 * @file pcf8574.hpp
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

#ifndef _PCF8574_HPP_
#define _PCF8574_HPP_

#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstdbool>
#include <string>
#include <cstring>
#include <sstream>

#include <drivers/i2c.hpp>

namespace Airsoft::Devices {


#define PCF8574_LIB_VERSION         ("0.0.1"))

#ifndef PCF8574_INITIAL_VALUE
#define PCF8574_INITIAL_VALUE       0xFF
#endif

#define PCF8574_OK                  0x00
#define PCF8574_PIN_ERROR           0x81
#define PCF8574_I2C_ERROR           0x82


class PCF8574 final {
public:
  explicit PCF8574(Drivers::I2C * wire, const uint8_t deviceAddress = 0x20);

  bool    Begin(uint8_t value = PCF8574_INITIAL_VALUE);
  bool    IsConnected(void);


  // Note: setting the address corrupt internal buffer values a read8() / write8() call updates them.
  bool SetAddress(const uint8_t deviceAddress);
  uint8_t GetAddress(void) const {
    return _address;
  }

  uint8_t Read8(void);
  uint8_t Read(const uint8_t pin);
  uint8_t Value(void) const {
    return _dataIn;
  }


  void Write8(const uint8_t value);
  void Write(const uint8_t pin, const bool value);
  uint8_t ValueOut(void) const {
    return _dataOut;
  }

  uint8_t ReadButton8(void) {
    return ReadButton8(_buttonMask);
  }
  uint8_t ReadButton8(const uint8_t mask);
  uint8_t ReadButton(const uint8_t pin);
  void    SetButtonMask(const uint8_t mask) {
    _buttonMask = mask;
  }
  uint8_t GetButtonMask(void) const {
    return _buttonMask;
  }

  // Rotate, shift, toggle, reverse expect all lines are output
  void Toggle(const uint8_t pin);

  // Default 0xFF ==> invertAll()
  void ToggleMask(const uint8_t mask = 0xFF);
  void ShiftRight(const uint8_t n = 1);
  void ShiftLeft(const uint8_t n = 1);
  void RotateRight(const uint8_t n = 1);
  void RotateLeft(const uint8_t n = 1);
  void Reverse(void);


  void Select(const uint8_t pin);
  void SelectN(const uint8_t pin);
  void SelectNone(void) {
    Write8(0x00);
  }
  void SelectAll(void) {
    Write8(0xFF);
  }

  int32_t LastError(void);

private:
  int32_t         _error { PCF8574_OK };
  uint8_t         _address;
  uint8_t         _dataIn { 0 };
  uint8_t         _dataOut { 0xFF };
  uint8_t         _buttonMask { 0xFF };


  Drivers::I2C *  _wire;
};


} // namespace Airsoft::Devices

#endif // _PCF8574_HPP_
