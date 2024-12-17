/**
 *******************************************************************************
 * @file AirsoftManager.cpp
 *
 * @brief Main application
 *
 * @author  Cristian Croci - ccdevelop.net
 *
 * @version 1.00
 *
 * @date November 27, 2024
 *
 *******************************************************************************
 * This file is part of the Airsoft project 
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

#include <airsoftmanager.hpp>
#include <iostream>
#include <chrono>
#include <functional>
#include <regex>
#include <filesystem>
#include <string>
#include <unistd.h>

#include <config.hpp>
#include <drivers/gpio.hpp>
#include <drivers/uarts.hpp>
#include <classes/timer.hpp>

#include <pages.hpp>

using namespace std::chrono_literals;

namespace Airsoft {

constexpr int32_t MAX_THREAD_WAIT_ON_EXIT = 10;

//======================================================================================================================
// Public Functions ****************************************************************************************************
//======================================================================================================================

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
    int32_t maxWait = MAX_THREAD_WAIT_ON_EXIT;

    // Reset thread flag
    _threadRunning = false;

    // Wait terminate for max \MAX_THREAD_WAIT_ON_EXIT\ seconds
    while(!_threadTerminated && maxWait-- > 0) {
      // Wait 1 second
      std::this_thread::sleep_for(1000ms);
    }

    // Null process
    _process = nullptr;
  }
}
//-----------------------------------------------------------------------------

//======================================================================================================================
// Private Functions ***************************************************************************************************
//======================================================================================================================

//-----------------------------------------------------------------------------
bool AirsoftManager::LoadConfiguration(void) {
  // Function Variables
  char folder[1024];

  // Get current folder
  if (getcwd(folder, 1024) != nullptr) {
    // print the current working directory
    std::cout << "Current working directory: " << folder << std::endl;
  } else {
    // If _getcwd returns NULL, print an error message
    std::cerr << "Error getting current working directory" << std::endl;
    return false;
  }

  std::string configFile = folder;
  configFile += "/airsoft/asm-config.cfg";

  std::ifstream fconf(configFile.c_str());
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

//======================================================================================================================
// Main Thread Engine **************************************************************************************************
//======================================================================================================================

// Pointer to GPIO led for use with timer
static Airsoft::Drivers::Gpio * t_led {};

//-----------------------------------------------------------------------------
void AirsoftManager::Engine(void) {
  // Thread Variables
  Airsoft::Classes::Timer ledTimer;
  //int count = 0;
  Airsoft::Drivers::Gpio  led(Airsoft::Drivers::BANK_1, Airsoft::Drivers::GROUP_C, Airsoft::Drivers::ID_4);

  std::cout << "Engine Manager: Started." << std::endl;

  // Load configuration
  LoadConfiguration();

  // Configure GPIO for led status
  led.Open(Airsoft::Drivers::Direction::Output);

  // Start blink timer
  t_led = &led;
  ledTimer.SetInterval([]() {
    t_led->Toggle();
  }, 500);

  // Initialize GPS Module
  _Gps().Init("/dev/ttyS3");

  // Initialize Wireless
  _Wireless().Init("/dev/ttyS0", Airsoft::Drivers::Gpio::CalculateGpioId(Airsoft::Drivers::BANK_1, Airsoft::Drivers::GROUP_C, Airsoft::Drivers::ID_1),
                               Airsoft::Drivers::Gpio::CalculateGpioId(Airsoft::Drivers::BANK_1, Airsoft::Drivers::GROUP_C, Airsoft::Drivers::ID_2),
                               Airsoft::Drivers::Gpio::CalculateGpioId(Airsoft::Drivers::BANK_1, Airsoft::Drivers::GROUP_C, Airsoft::Drivers::ID_3));


  // Initialize In/Out
  _InOut().Init("/dev/i2c-3");

  // Initialize Display
  if (_Display().Init("/dev/i2c-4", 0x27)) {
    if (!_Display().Begin()) {
      std::cout << "Error start Display!!!!" << std::endl;
      while(true) {
        std::this_thread::sleep_for(500ms);
      }
    }
  }

  // Create Main Page
  _currentPage = new Airsoft::Pages::PMain();
  _pages.push_back(_currentPage);

  // Load Page
  _currentPage->Load(this);

  // Begin program thread ===========================================

  // Thread loop
  while(_threadRunning) {
    // Handle keys
    while(_InOut().KeysOnQueue() > 0) {
      // Local variables
      char    key {};
      uint8_t keyCode {};

      // Read Key info
      if (_InOut().GetKeyFromQueue(key, keyCode)) {
        _currentPage->KeyHandle(key, keyCode);
      }
    }

    // Verify current page
    if (_currentPage) {
      // Call periodic function
      _currentPage->Periodic();
    }

    // Check if new page
    if (_newPageAvailable) {
      _pages.push_back(_newPage);
      _currentPage = _newPage;
      _newPageAvailable = false;

      continue;
    }

    // Sleep for periodic time
    std::this_thread::sleep_for(std::chrono::milliseconds(_currentPage->PeriodicTime()));
  }

  // Terminate program ==============================================

  // Remove all pages from queue
  while (!_pages.empty()) {
    Airsoft::Templates::DisplayPage * page { _pages.back() };
    _pages.pop_back();
    delete page;
  }

  // Terminate In/Out
  _InOut().Terminate();

  // Terminate Wireless
  _Wireless().Terminate();

  // Terminate GPS
  _Gps().Terminate();

  // Stop
  ledTimer.Stop();

  std::cout << "Engine Manager: Terminated." << std::endl;
}
//-----------------------------------------------------------------------------

//======================================================================================================================
// Implement DisplayEngine Interface ***********************************************************************************
//======================================================================================================================

//-----------------------------------------------------------------------------
void AirsoftManager::Clean(void) {
  _Display().Clear();
}
//-----------------------------------------------------------------------------
void AirsoftManager::CleanRow(uint8_t row) {
  // Function Variables
  std::string clean = "                    ";

  _Display().SetCursor(0, row);
  _Display().print(clean.c_str());
}
//-----------------------------------------------------------------------------
void AirsoftManager::Print(char val) {
  _Display().print(val);
}
//-----------------------------------------------------------------------------
void AirsoftManager::Print(const char * str) {
  _Display().print(str);
}
//-----------------------------------------------------------------------------
void AirsoftManager::Print(std::string str) {
  _Display().print(str);
}
//-----------------------------------------------------------------------------
void AirsoftManager::PrintAt(uint8_t col, uint8_t row, char val) {
  _Display().SetCursor(col, row);
  _Display().print(val);
}
//-----------------------------------------------------------------------------
void AirsoftManager::PrintAt(uint8_t col, uint8_t row, const char * str) {
  _Display().SetCursor(col, row);
  _Display().print(str);
}
//-----------------------------------------------------------------------------
void AirsoftManager::PrintAt(uint8_t col, uint8_t row, std::string str) {
  _Display().SetCursor(col, row);
  _Display().print(str);
}
//-----------------------------------------------------------------------------
void AirsoftManager::MoveCursor(uint8_t col, uint8_t row) {
  _Display().SetCursor(col, row);
}
//-----------------------------------------------------------------------------
void AirsoftManager::Backlight(Airsoft::Templates::DisplayStatus status) {
  _Display().SetBacklight(status == Airsoft::Templates::DisplayStatus::On);
}
//-----------------------------------------------------------------------------
void AirsoftManager::Display(Airsoft::Templates::DisplayStatus status) {
  // TODO: To Reuse
}
//-----------------------------------------------------------------------------
bool AirsoftManager::ActivatePage(Airsoft::Templates::DisplayPage * pageToActivate) {
  if (_newPageAvailable) {
    return false;
  }

  _newPage = pageToActivate;
  _newPageAvailable = true;

  return true;
}
//-----------------------------------------------------------------------------
} // namespace Airsoft
