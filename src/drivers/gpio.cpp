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

#include <drivers/gpio.hpp>
#include <fstream>

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
  _gpioPin = (bank * 32) + ((group * 8) + id);

}
//-----------------------------------------------------------------------------
Gpio::~Gpio() {
  // TODO Auto-generated destructor stub
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
bool Gpio::Open(Direction direction) {
  if (!_isOpen) {
    // Assign direction
    _direction = direction;

    //const std::string direction = "out";
    /*const std::string path1 = "/sys/class/gpio/export";

    std::ofstream extStream(path1.c_str(), std::ofstream::trunc);
    if (extStream) {
      extStream << "55";
    } else {
      // LOG error here
      return false;
    }
    extStream.flush();
    extStream.close();*/

    // Export GPIO
    FILE * exportFile = fopen("/sys/class/gpio/export", "w");
    if (exportFile == nullptr) {
      perror("Failed to open GPIO export file");
      return false;
    }
    fprintf(exportFile, "%d", _gpioPin);
    fflush(exportFile);
    fclose(exportFile);

    //const std::string direction = "out";
    const std::string path = "/sys/class/gpio/gpio" + std::to_string(_gpioPin) + "/direction";

    /*std::ofstream dirStream(path.c_str(), std::ofstream::trunc);
    if (dirStream) {
      dirStream << direction;
    } else {
      // LOG error here
      return false;
    }*/

    // Set direction
    char directionPath[75];
    snprintf(directionPath, sizeof(directionPath), "/sys/class/gpio/gpio%d/direction", _gpioPin);
    FILE * directionFile = fopen(directionPath, "w");
    if (directionFile == nullptr) {
      perror("Failed to open GPIO direction file");
      return false;
    }
    fprintf(directionFile, _direction == Direction::Output ? "out" : "in");
    fflush(directionFile);
    fclose(directionFile);

    char value_path[50];
    char cat_command[100];
    snprintf(value_path, sizeof(value_path), "/sys/class/gpio/gpio%d/value", _gpioPin);
    snprintf(cat_command, sizeof(cat_command), "cat %s", value_path);
    _valuePath = value_path;
    _catCommand = cat_command;

    // Open File
    if ((_value = fopen(_valuePath.c_str(), "w")) == nullptr) {
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
  if (_isOpen) {
    fflush(_value);
    fclose(_value);

    FILE * unexportFile = fopen("/sys/class/gpio/unexport", "w");
    if (unexportFile == NULL) {
      perror("Failed to open GPIO unexport file");
      return;
    }

    fprintf(unexportFile, "%d", _gpioPin);
    fflush(unexportFile);

    fclose(unexportFile);
  }
}
//-----------------------------------------------------------------------------
void Gpio::Set(void) {
  fprintf(_value, "%d", 1);
  fflush(_value);
}
//-----------------------------------------------------------------------------
void Gpio::Reset(void) {
  fprintf(_value, "%d", 0);
  fflush(_value);
}
//-----------------------------------------------------------------------------
bool Gpio::Read(void) {

  return false;
}
//-----------------------------------------------------------------------------

} // namespace Airsoft::Drivers
