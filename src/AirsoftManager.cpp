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

#include <iostream>
#include <chrono>
#include <functional>

#include <drivers/gpio.hpp>

#include "AirsoftManager.hpp"


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

    delete _process;
    _process = nullptr;
  }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void AirsoftManager::Engine(void) {
  // Thread Variables
  int count = 0;
  Airsoft::Drivers::Gpio  led(Airsoft::Drivers::BANK_1, Airsoft::Drivers::GROUP_C, Airsoft::Drivers::ID_7);

  std::cout << "Engine Manager: Started." << std::endl;

  led.Open(Airsoft::Drivers::Direction::Output);

  while(_threadRunning) {
    std::cout << "Count : " << count++  << std::endl;
    std::this_thread::sleep_for(1000ms);
  }

  std::cout << "Engine Manager: Terminated." << std::endl;
}

} // namespace Airsoft
