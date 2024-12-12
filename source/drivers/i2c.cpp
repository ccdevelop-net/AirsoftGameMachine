/**
 *******************************************************************************
 * @file i2c.cpp
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

#include <cstdio>
#include <cstdint>
#include <cstdbool>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <drivers/i2c.hpp>


namespace Airsoft::Drivers {

I2C::I2C() {
  // TODO Auto-generated constructor stub

}

I2C::~I2C() {

}

//------------------------------------------------------------------------------
bool I2C::Init(std::string port) {
  // Check valid port
  if (port.empty()) {
    return false;
  }

  // Set port
  _port = port;

  // Check port
  if (!Open()) {
    return false;
  }

  // Close port
  Close();

  // Set flag
  _initilized = true;

  return true;
}
//------------------------------------------------------------------------------
void I2C::Terminate(void) {
  // Close port
  Close();

  // Reset flag
  _initilized = false;

  // Invalidate port
  _port = "";
}
//------------------------------------------------------------------------------
void I2C::SetClock(uint32_t clock) {
  // NOT IMPLEMENTED....
}
//------------------------------------------------------------------------------
void I2C::BeginTransmission(uint8_t address) {
  if (Open()) {
    if (SetSlaveAddr(address) != 0) {
      Close();
    }
  }
}
//------------------------------------------------------------------------------
void I2C::BeginTransmission(int32_t address) {
  BeginTransmission((uint8_t)address);
}
//------------------------------------------------------------------------------
uint8_t I2C::EndTransmission(void) {
  if (!_is_open) {
    return 1;
  }

  Close();

  return 0;
}
//------------------------------------------------------------------------------
uint8_t I2C::Status(void) {
  if (!_is_open) {
    return 0;
  }

  return 0;
}
//------------------------------------------------------------------------------
size_t I2C::Write(uint8_t data) {
  if (!_is_open) {
    return 0;
  }

  uint8_t dataToSend = data;

  return ::write(_fd, &dataToSend, 1);
}
//------------------------------------------------------------------------------
size_t I2C::Write(const uint8_t * data, size_t size) {
  if (!_is_open) {
    return 0;
  }

  return ::write(_fd, data, size);
}
//------------------------------------------------------------------------------
int32_t I2C::Available(void) {
  if (!_is_open) {
    return 0;
  }

  return 0;
}
//------------------------------------------------------------------------------
int32_t I2C::Read(uint8_t * buffer, size_t bufferSize, size_t numByteToRead) {
  if (!_is_open) {
    return 0;
  }

  return ::read(_fd, buffer, numByteToRead);
}
//------------------------------------------------------------------------------
int32_t I2C::Peek(void) {
  if (!_is_open) {
    return 0;
  }

  return 0;
}
//------------------------------------------------------------------------------
void I2C::Flush(void) {
  if (!_is_open) {
    return;
  }

  //::flush(_fd);

}
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
bool I2C::Open(void) {
  // Open port
  _fd = ::open(_port.c_str(), O_RDWR);

  if (_fd < 0) {
    if (errno == ENOENT) {
      std::cout << "Error: Could not open file " << _port << strerror(ENOENT) << std::endl;
    } else {
      std::cout << "Error: Could not open file " << _port << strerror(errno) << std::endl;
      if (errno == EACCES) {
        std::cout << "Run as root?" << std::endl;
      }
    }

    return false;
  }

  // Set address at 7 bits and 5 retries
  ::ioctl(_fd, I2C_TENBIT, 0);
  ::ioctl(_fd, I2C_RETRIES, 5);

  _is_open = true;

  return true;
}
//------------------------------------------------------------------------------
void I2C::Close(void) {
  if (_fd > -1) {
    ::close((int)_fd);
  }

  _fd = -1;
  _is_open = false;
}
//------------------------------------------------------------------------------
int32_t I2C::SetSlaveAddr(uint8_t address, bool force) {
  // With force, let the user read from/write to the registers even when a driver is also running
  if (::ioctl(_fd, force ? I2C_SLAVE_FORCE : I2C_SLAVE, address) < 0) {
    std::cout << "Error: Could not set address to 0x" << std::hex << address << std::dec << strerror(errno) << std::endl;
    return -errno;
  }

  return 0;
}
//------------------------------------------------------------------------------

} // namespace Airsoft::Drivers
