/**
 *******************************************************************************
 * @file print.hpp
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

#ifndef CLASSES_PRINT_HPP_
#define CLASSES_PRINT_HPP_

#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstdbool>
#include <string>
#include <cstring>

namespace Airsoft::Classes {

constexpr uint8_t DEC = 10;
constexpr uint8_t HEX = 16;
constexpr uint8_t OCT = 8;
#ifdef BIN // Prevent warnings if BIN is previously defined in "iotnx4.h" or similar
#undef BIN
#endif
constexpr uint8_t BIN = 2;


class Print {
public:
  Print();// = default;
  virtual ~Print();// = default;

public:
  int32_t GetWriteError(void) {
    return _write_error;
  }
  void ClearWriteError(void) {
    SetWriteError(0);
  }

  virtual size_t Write(uint8_t) = 0;
  size_t Write(const char * str) {
    if (str == nullptr) {
      return 0;
    }
    return Write((const uint8_t *)str, strlen(str));
  }
  virtual size_t Write(const uint8_t * buffer, size_t size) = 0;
  size_t Write(const char * buffer, size_t size) {
    return Write((const uint8_t *)buffer, size);
  }

  // Default to zero, meaning "a single write may block" should be overridden
  // by subclasses with buffering
  virtual int32_t AvailableForWrite() {
    return 0;
  }

  size_t print(const std::string & message);
  size_t print(const char msg[]);
  size_t print(char c);
  size_t print(uint8_t value, int32_t format = DEC);
  size_t print(int32_t, int32_t format = DEC);
  size_t print(uint32_t, int32_t format = DEC);
  size_t print(int64_t value, int32_t format = DEC);
  size_t print(uint64_t value, int32_t format = DEC);
  size_t print(double, int32_t format = 2);

  size_t println(const std::string & msg);
  size_t println(const char msg[]);
  size_t println(char c);
  size_t println(uint8_t, int32_t format = DEC);
  size_t println(int32_t, int32_t format = DEC);
  size_t println(uint32_t, int32_t format = DEC);
  size_t println(int64_t, int32_t format = DEC);
  size_t println(uint64_t, int32_t format = DEC);
  size_t println(double, int32_t format = 2);
  size_t println(void);

private:
  int32_t _write_error {};

private:
  size_t PrintNumber(uint64_t number, uint8_t);
  size_t PrintFloat(double number, uint8_t);

protected:
  void SetWriteError(int err = 1) {
    _write_error = err;
  }
};

} // namespace Airsoft::Classes

#endif // CLASSES_PRINT_HPP_
