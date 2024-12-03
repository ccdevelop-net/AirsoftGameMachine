/**
 *******************************************************************************
 * @file Uarts.cpp
 *
 * @brief Description
 *
 * @author  Cristian
 *
 * @version 1.00
 *
 * @date Nov 27, 2024
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

#include <drivers/uarts.hpp>

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

namespace Airsoft::Drivers {

Uarts::Uarts() {

}

Uarts::~Uarts() {

}

//-----------------------------------------------------------------------------
bool Uarts::Open(uint8_t serialPortNumber, uint32_t baudrate) {
  _serialPort = "/dev/ttyS" + std::to_string(serialPortNumber);

  _serialHandle = open(_serialPort.c_str(), O_RDWR | O_NOCTTY);
  if (_serialHandle == -1) {
    perror("Failed to open serial port");
    return 1;
  }

  struct termios tty;
  std::memset(&tty, 0, sizeof(tty));

  if (tcgetattr(_serialHandle, &tty) != 0) {
    perror("Error from tcgetattr");
    return 1;
  }

  cfsetospeed(&tty, B9600);
  cfsetispeed(&tty, B9600);

  tty.c_cflag &= ~PARENB;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;

  if (tcsetattr(_serialHandle, TCSANOW, &tty) != 0) {
    perror("Error from tcsetattr");
    return 1;
  }

  return false;
}
//-----------------------------------------------------------------------------
void Uarts::Close(void) {
  close(_serialHandle);
}
//-----------------------------------------------------------------------------
bool Uarts::Write(std::string message) {

  return false;
}
//-----------------------------------------------------------------------------
bool Uarts::Write(uint8_t * data, size_t size) {

  return false;
}
//-----------------------------------------------------------------------------
size_t Uarts::Read(uint8_t * buffer, size_t bufferLenght) {
  return 0;
}
//-----------------------------------------------------------------------------
bool Uarts::Read(std::string * text) {
  char buffer[4096] {};
  int32_t bytesRead {};
  if ((bytesRead = read(_serialHandle, buffer, sizeof(buffer))) > 0) {
    buffer[bytesRead] = '\0';
    *text = buffer;
    return true;
  }

  return false;
}
//-----------------------------------------------------------------------------


} // namespace Airsoft::Drivers
