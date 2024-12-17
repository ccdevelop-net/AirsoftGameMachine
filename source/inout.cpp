/**
 *******************************************************************************
 * @file inout.cpp
 *
 * @brief Input/Output/Leds and Keyboard Engine
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

#include <drivers/i2c.hpp>
#include <utility.hpp>

#include <inout.hpp>

using namespace std::chrono_literals;

// To debug IO class set flag to 1
#define DEBUG_IO 0

namespace Airsoft {

//======================================================================================================================
// Public Functions ****************************************************************************************************
//======================================================================================================================

//------------------------------------------------------------------------------
bool InOut::Init(std::string port) {
  // Check valid port
  if (port.empty()) {
    return false;
  }

  // Set port
  _port = port;

  // Set flag of thread running
  _threadRunning = true;

  // Create Engine Thread
  return ((_process = new std::thread(std::bind(&InOut::Engine, this))) != nullptr);
}
//------------------------------------------------------------------------------
void InOut::Terminate(void) {
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
void InOut::Led(uint8_t ledId, bool value) {
  if (ledId >= MAX_IO) {
    return;
  }

  ManageIo(&_map[ledId], value);
}
//------------------------------------------------------------------------------
void InOut::Rele(uint8_t releId, bool value) {
  if (releId >= MAX_IO) {
    return;
  }

  ManageIo(&_map[releId], value);
}
//------------------------------------------------------------------------------
size_t InOut::KeysOnQueue(void) {
  // Function Variables
  size_t    size {};

  // Lock access to queue
  _queueLock.lock();

  // Get queue size
  size = _keysQueue.size();

  // Unlock access to queue
  _queueLock.unlock();

  return size;
}
//------------------------------------------------------------------------------
bool InOut::GetKeyFromQueue(char & key, uint8_t & keyCode) {
  // Function Variables
  KeyData * data {};
  bool      valid {};

  // Lock access to queue
  _queueLock.lock();

  // Check if
  if (_keysQueue.size() > 0) {
    // Get key from queue
    data = _keysQueue.front();

    // Set data to return
    key = data->Key;
    keyCode = data->KeyCode;

    // Remove from queue
    _keysQueue.pop();

    valid = true;

    delete data;
    data = nullptr;
  } else {
    key = '\0';
    keyCode = 0xFF;
  }

  // Unlock access to queue
  _queueLock.unlock();

  return valid;
}
//------------------------------------------------------------------------------

//======================================================================================================================
// Private Functions ***************************************************************************************************
//======================================================================================================================

//------------------------------------------------------------------------------
void InOut::ManageIo(IOMap * map, bool value) {
  _wireLock.lock();

  if (map->Address == IO0__7_ADDR) {
    _out0_7->Write(map->Pin, value);
    map->Value = value;
  } else if (map->Address == IO0__7_ADDR) {
    _out8_15->Write(map->Pin, value);
    map->Value = value;
  }

  _wireLock.unlock();
}
//------------------------------------------------------------------------------

//======================================================================================================================
// In/Out Thread Engine ************************************************************************************************
//======================================================================================================================

//------------------------------------------------------------------------------
void InOut::Engine(void) {
  // Thread Variables
  Airsoft::Drivers::I2C wire;

  std::cout << "In/Out: Engine Started." << std::endl;

    // Open serial
  if (!wire.Init(_port)) {
    std::cout << "In/Out: Error open I2C port." << std::endl;
  }

  // Initialize IO Drivers
  _keyboard = new Airsoft::Devices::I2CKeyPad(&wire, KEYBOARD_ADDR);
  _out0_7 = new Airsoft::Devices::PCF8574(&wire, IO0__7_ADDR);
  _out8_15 = new Airsoft::Devices::PCF8574(&wire, IO8_15_ADDR);

  _wireLock.lock();
  _keyboard->Begin();
  _keyboard->LoadKeyMap(_keymap);
  _wireLock.unlock();

  // Set ready flag
  _ready = true;

  // Thread loop
  while(_threadRunning) {
    uint8_t key {};

    _wireLock.lock();
    key = _keyboard->GetChar();
    _wireLock.unlock();

    if (key != I2C_KEYPAD_THRESHOLD && key != I2C_KEYPAD_NOKEY) {
      // Lock access to queue
      _queueLock.lock();

      // Read and queue key
      KeyData * keyData = new KeyData;
      keyData->Key = static_cast<char>(key);
      keyData->KeyCode = _keyboard->GetLastKey();
      _keysQueue.push(keyData);

      // Unlock access to queue
      _queueLock.unlock();

#if DEBUG_IO
      std::cout << (char)key << std::endl;
#endif
    }

    std::this_thread::sleep_for(100ms);
  }

  std::cout << "In/Out: Terminated." << std::endl;
}
//------------------------------------------------------------------------------

} // namespace Airsoft
