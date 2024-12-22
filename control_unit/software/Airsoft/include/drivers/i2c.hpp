/**
 *******************************************************************************
 * @file i2c.hpp
 *
 * @brief Description
 *
 * @author  Cristian
 *
 * @version 1.00
 *
 * @date November 27, 2024
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

#ifndef DRIVERS_I2C_HPP_
#define DRIVERS_I2C_HPP_

namespace Airsoft::Drivers {

class I2C final {
public:
  I2C();
  virtual ~I2C();

private:
  std::string       _port;                                // Path to the file descriptor
  int32_t           _fd { -1 };                           // The current file descriptor

  bool              _initilized {};

  bool              _is_open {};

public:
  bool Init(std::string port);
  void Terminate(void);

  bool inline IsInitialized(void) const {
    return _initilized;
  }

  bool inline IsOpen(void) const {
    return _is_open;
  }

  void SetClock(uint32_t clock);
  void BeginTransmission(uint8_t address);
  void BeginTransmission(int32_t address);
  uint8_t EndTransmission(void);
  uint8_t Status(void);

  size_t Write(uint8_t data);
  size_t Write(const uint8_t * data, size_t size);
  int32_t Available(void);
  int32_t Read(uint8_t * buffer, size_t bufferSize, size_t numByteToRead);
  int32_t Peek(void);
  void Flush(void);

private:
  bool Open(void);
  void Close(void);
  int32_t SetSlaveAddr(uint8_t address, bool force = false);
};

} // namespace Airsoft::Drivers

#endif // DRIVERS_I2C_HPP_
