/**
 *******************************************************************************
 * @file AirsoftManager.cpp
 *
 * @brief Description
 *
 * @author  Cristian Croci
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

#include <airsoftmanager.hpp>
#include <iostream>
#include <chrono>
#include <functional>
#include <regex>
#include <filesystem>
#include <string>

#include <config.hpp>
#include <drivers/gpio.hpp>
#include <drivers/uarts.hpp>
#include <classes/timer.hpp>

using namespace std::chrono_literals;

namespace Airsoft {

//-----------------------------------------------------------------------------
bool AirsoftManager::Init(void) {
  // Set flag of thread running
  _threadRunning = true;

  // Create Engine Thread
  return ((_process = new std::thread(std::bind(&AirsoftManager::Engine, this))) != nullptr);
}
//-----------------------------------------------------------------------------
void AirsoftManager::Terminate(void) {
  // Check valid thread
  if (_process != nullptr) {
    // Reset thread flag
    _threadRunning = false;

    // Wait 1.5 second for thread terminate
    std::this_thread::sleep_for(1500ms);

    //delete _process;
    _process = nullptr;
  }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
bool AirsoftManager::LoadConfiguration(void) {
  // Function Variables
  std::filesystem::path folder = std::filesystem::current_path();

  folder /= "asm-config.cfg";

  std::ifstream fconf(folder.c_str());
  std::string line;
  while(std::getline(fconf, line)) {
    std::istringstream is_line(line);
    std::string key;
    if (std::getline(is_line, key, '=') ) {
      std::string value;
      if(std::getline(is_line, value)) {
        if (key == "address_high") {
          Configuration.AddressH = atoi(value.c_str());
        } else if (key == "address_low") {
          Configuration.AddressL = atoi(value.c_str());
        }
      }
    }
  }
  return true;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void AirsoftManager::Engine(void) {
  // Thread Variables
  int count = 0;
  Airsoft::Drivers::Gpio  led(Airsoft::Drivers::BANK_1, Airsoft::Drivers::GROUP_C, Airsoft::Drivers::ID_4);

  std::cout << "Engine Manager: Started." << std::endl;

  LoadConfiguration();

  led.Open(Airsoft::Drivers::Direction::Output);

  // Initialize GPS Module
  //_gps.Init("/dev/ttyS3");

  // Initialize Wireless
  /*_wireless.Init("/dev/ttyS0", Airsoft::Drivers::Gpio::CalculateGpioId(Airsoft::Drivers::BANK_1, Airsoft::Drivers::GROUP_C, Airsoft::Drivers::ID_1),
                               Airsoft::Drivers::Gpio::CalculateGpioId(Airsoft::Drivers::BANK_1, Airsoft::Drivers::GROUP_C, Airsoft::Drivers::ID_2),
                               Airsoft::Drivers::Gpio::CalculateGpioId(Airsoft::Drivers::BANK_1, Airsoft::Drivers::GROUP_C, Airsoft::Drivers::ID_3));
*/

  //_inout.Init("/dev/i2c-3");

  //bool value = ON;

  if (_wire.Init("/dev/i2c-4")) {
    if ((_display = new Airsoft::Devices::I2CDisplay(&_wire, 0x27)) != nullptr) {
      if (!_display->Begin()) {
        while(true) {
          std::this_thread::sleep_for(500ms);
        }
      }
    }

  }



  // Thread loop
  while(_threadRunning) {
    std::cout << "Count : " << count++  << std::endl;
    led.Set();
    std::this_thread::sleep_for(500ms);
    led.Reset();
    std::this_thread::sleep_for(500ms);

    std::stringstream test;

    test << "Count: " << count;
    _display->SetCursor(0, 0);
    _display->print(test.str());

    //_inout.Led(LED1, value);
    //value = !value;

  }

  // Terminate Wireless
  _wireless.Terminate();

  // Terminate GPS
  _gps.Terminate();

  std::cout << "Engine Manager: Terminated." << std::endl;
}

} // namespace Airsoft
