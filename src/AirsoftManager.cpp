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
#include <regex>

#include <drivers/gpio.hpp>
#include <drivers/uarts.hpp>

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
std::string AirsoftManager::Trim(const std::string & source) {
  std::string s(source);
  s.erase(0,s.find_first_not_of(" \n\r\t"));
  s.erase(s.find_last_not_of(" \n\r\t")+1);
  return s;
}
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
void AirsoftManager::Engine(void) {
  // Thread Variables
  int count = 0;
  Airsoft::Drivers::Gpio  led(Airsoft::Drivers::BANK_1, Airsoft::Drivers::GROUP_C, Airsoft::Drivers::ID_4);
  Airsoft::Drivers::Gpio  aux(Airsoft::Drivers::BANK_1, Airsoft::Drivers::GROUP_C, Airsoft::Drivers::ID_1);
  Airsoft::Drivers::Uarts gps;
  std::string gpsData;

  std::cout << "Engine Manager: Started." << std::endl;

  led.Open(Airsoft::Drivers::Direction::Output);
  aux.Open(Airsoft::Drivers::Direction::Input);

  gps.Open(3, 9600);

  while(_threadRunning) {
    //std::cout << "Count : " << count++  << std::endl;
    //led.Set();
    //std::this_thread::sleep_for(1000ms);
    if (gps.Read(&gpsData)) {
      //std::cout << "GPS Data" << std::endl;
      if (gpsData.find("$GPTXT", 0)) {
        std::cout << std::regex_replace(gpsData, std::regex{R"(^\s+|\s+$)"}, "") << std::endl;
        //std::cout << gpsData << std::endl;
      }
      gpsData.clear();
    }
    //led.Reset();
    //std::this_thread::sleep_for(1000ms);
    //if (aux.Read()) {
    //  std::cout << "AUX : High" << std::endl;
    //} else {
    //  std::cout << "AUX : Low" << std::endl;
    //}
    std::this_thread::sleep_for(100ms);
  }

  std::cout << "Engine Manager: Terminated." << std::endl;
}

} // namespace Airsoft
