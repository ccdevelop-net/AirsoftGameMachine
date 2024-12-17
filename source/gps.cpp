/**
 *******************************************************************************
 * @file Gps.cpp
 *
 * @brief Manage GPS module header file
 *
 * @author  Cristian Croci - CCDevelop.net
 *
 * @version 1.00
 *
 * @date Dec 3, 2024
 *
 *******************************************************************************
 * This file is part of the Airsoft Game Machine project
 * https://github.com/ccdevelop-net/AirsoftGameMachine.
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

#include <drivers/uarts.hpp>
#include <utility.hpp>

#include <gps.hpp>

using namespace std::chrono_literals;

// Un-comment for GPS debug
//#define DEBUG_GPS

namespace Airsoft {

//======================================================================================================================
// Public Functions ****************************************************************************************************
//======================================================================================================================

//------------------------------------------------------------------------------
bool Gps::Init(std::string port) {
  // Check valid port
  if (port.empty()) {
    return false;
  }

  // Set port
  _port = port;

  // Set flag of thread running
  _threadRunning = true;

  // Create Engine Thread
  return ((_process = new std::thread(std::bind(&Gps::Engine, this))) != nullptr);
}
//------------------------------------------------------------------------------
void Gps::Terminate(void) {
  // Check valid thread
  if (_process != nullptr) {
    // Reset thread flag
    _threadRunning = false;

    // Wait 1.5 second for thread terminate
    std::this_thread::sleep_for(1500ms);

    _process = nullptr;
  }
}
//------------------------------------------------------------------------------

//======================================================================================================================
// GPS Thread Engine ***************************************************************************************************
//======================================================================================================================

//------------------------------------------------------------------------------
void Gps::Engine(void) {
  // Thread Variables
  Airsoft::Drivers::Uarts serial(_port, 9600);
  std::string gpsData;

  std::cout << "GPS Engine: Started." << std::endl;

  try {
    // Open serial
    serial.Open();
  } catch (...) {
    std::cout << "GPS : Error open serial port." << std::endl;
  }

  // Set ready flag
  _ready = true;

  // Thread loop
  while(_threadRunning) {
    // Loop available serial data
    while (_nema.Available(serial)) {
      _fix = _nema.Read();
    }

#if DEBUG_GPS
    std::cout << Utility::Trim(gpsData) << std::endl;
#endif  // DEBUG_GPS

    std::this_thread::sleep_for(100ms);
  }

  std::cout << "GPS Engine: Terminated." << std::endl;
}
//------------------------------------------------------------------------------

} // namespace Airsoft
