/**
 *******************************************************************************
 * @file gpio.cpp
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
 * This file is part of the Airsoft Game Machine project
 * https://github.com/ccdevelop-net/AirsoftGameMachine
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

#include <drivers/gpio.hpp>

namespace Airsoft::Drivers {

//-----------------------------------------------------------------------------
Gpio::Gpio(uint32_t bank, uint8_t group, uint32_t id) {
  // Check if bank is in range
  if (bank > BANK_4) {
    return;
  }

  // Check if group is in range
  if (group > GROUP_D) {
    return;
  }

  // Check if id is in range
  if (id > ID_7) {
    return;
  }

  // Calculate GPIO Pin
  _gpioPin = Gpio::CalculateGpioId(bank, group, id);
}
//-----------------------------------------------------------------------------
Gpio::Gpio(uint32_t gpioPin) {
  // Assign GPIO Pin
  _gpioPin = gpioPin;

}
//-----------------------------------------------------------------------------
Gpio::~Gpio() {
  // Close GPIO
  Close();
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
bool Gpio::Open(Direction direction, Level level) {
  if (!_isOpen) {
    // Assign direction
    _direction = direction;

    // Export pin
    std::ofstream extStream("/sys/class/gpio/export", std::ofstream::trunc);

    // Check if open
    if (extStream.is_open()) {
      // Write number of the pin, flush and close
      extStream << std::to_string(_gpioPin);
      extStream.flush();
      extStream.close();
    } else {
      // LOG error here
      perror("Failed to open GPIO export file");
      return false;
    }

    // Direction of the GPIO
    const std::string directionPath = "/sys/class/gpio/gpio" + std::to_string(_gpioPin) + "/direction";

    // Open direction
    std::ofstream dirStream(directionPath.c_str(), std::ofstream::trunc);

    // Check if open
    if (dirStream.is_open()) {
      // Write direction of the pin, flush and close
      dirStream << (_direction == Direction::Output ? "out" : "in");
      dirStream.flush();
      dirStream.close();
    } else {
      // LOG error here
      perror("Failed to open GPIO direction file");
      return false;
    }

    // GPIO value path
    _valuePath = "/sys/class/gpio/gpio" + std::to_string(_gpioPin) + "/value";
    _catCommand = "cat " + _valuePath;

    // Open File
    _valueOutput.open(_valuePath.c_str(), std::ofstream::trunc);
    if (!_valueOutput.is_open()) {
      perror("Failed to open GPIO value file");
      return false;
    }

    _isOpen = true;

    return true;
  }

  return false;
}
//-----------------------------------------------------------------------------
void Gpio::Close(void) {
  // Check if opened
  if (_isOpen) {
    // Flush and close
    _valueOutput.flush();
    _valueOutput.close();

    // Open 'unexport' control and disable GPIO
    std::ofstream unexportStream("/sys/class/gpio/unexport", std::ofstream::trunc);
    if (unexportStream.is_open()) {
      unexportStream << std::to_string(_gpioPin);
      unexportStream.flush();
      unexportStream.close();
    } else {
      // LOG error here
      perror("Failed to open GPIO unexport file");
      return;
    }
  }
}
//-----------------------------------------------------------------------------
void Gpio::Set(void) {
  // Only if output
  if (_direction == Direction::Output) {
    // Set pin and flush
    _valueOutput << "1";
    _valueOutput.flush();
    _currentLevel = Level::High;
  }
}
//-----------------------------------------------------------------------------
void Gpio::Reset(void) {
  // Only if output
  if (_direction == Direction::Output) {
    // Reset pin and flush
    _valueOutput << "0";
    _valueOutput.flush();
    _currentLevel = Level::Low;
  }
}
//-----------------------------------------------------------------------------
void Gpio::Toggle(void) {
  // Only if output
  if (_direction == Direction::Output) {
    // Reset pin and flush
    _valueOutput << ((_currentLevel == Level::Low) ? "1" : "0");
    _valueOutput.flush();
    _currentLevel = (_currentLevel == Level::Low) ? Level::High : Level::Low;
  }
}
//-----------------------------------------------------------------------------
bool Gpio::Read(void) {
  std::string value;
  // Only if output
  if (_direction == Direction::Input) {
    FILE * fp {};
    char buffer[10] {};

    fp = popen(_catCommand.c_str(), "r");
    if (fp != nullptr) {
      if (fgets(buffer, 10, fp) != nullptr) {
        //printf("%s", buffer);
      }
      pclose(fp);
    }

    return buffer[0] == '1';
  }

  return _currentLevel == Level::High;
}
//-----------------------------------------------------------------------------

} // namespace Airsoft::Drivers
