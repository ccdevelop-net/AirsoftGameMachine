/**
 *******************************************************************************
 * @file wireless.cpp
 *
 * @brief Description
 *
 * @author  Cristian
 *
 * @version 1.00
 *
 * @date Dec 6, 2024
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

#include <drivers/uarts.hpp>
#include <devices/ebytelorae220.hpp>
#include <wireless.hpp>
#include <config.hpp>

using namespace std::chrono_literals;

namespace Airsoft {

Wireless::Wireless() {

}

Wireless::~Wireless() {

}

//------------------------------------------------------------------------------
bool Wireless::Init(std::string port, int32_t auxPin, int32_t m0Pin, int32_t m1Pin) {
  // Check valid port
  if (port.empty()) {
    return false;
  }

  // Set port
  _port = port;
  _auxPin = auxPin;
  _m0Pin = m0Pin;
  _m1Pin = m1Pin;

  // Set flag of thread running
  _threadRunning = true;

  // Create Engine Thread
  return ((_process = new std::thread(std::bind(&Wireless::Engine, this))) != nullptr);
}
//------------------------------------------------------------------------------
void Wireless::Terminate(void) {
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
void Wireless::SendMessage(std::string message) {
  _outLock.lock();
  _out.push(message);
  _outLock.unlock();
}
//------------------------------------------------------------------------------
bool Wireless::ReceiveMessage(std::string & message) {
  // Function variables
  bool ret {};

  _inLock.lock();
  if (!_in.empty()) {
    message = _in.front();
   _in.pop();
   ret = true;
  }
  _inLock.unlock();

  return ret;
}
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
void Wireless::Engine(void) {
  // Thread Variables
  Airsoft::Drivers::Uarts         serial(_port, 9600);
  Airsoft::Devices::EByteLoRaE220 lora(&serial, _auxPin, _m0Pin, _m1Pin);

  std::cout << "Wireless Engine: Started." << std::endl;

  serial.Open();

  // Initialize module
  if (lora.Begin()) {
    Airsoft::Devices::ResponseStructContainer currentConfig = lora.GetConfiguration();

    // Check if command success
    if (currentConfig.status.code == E220_SUCCESS) {
      Airsoft::Devices::Configuration * config = reinterpret_cast<Airsoft::Devices::Configuration*>(currentConfig.data);

      // Verify configuration
      if (config != nullptr) {
        // Verify address
        if ((config->AddrH == 0 && config->AddrL == 0) ||
            (config->AddrH != Configuration.AddressH && config->AddrL != Configuration.AddressL)){

          // Set new address
          config->AddrH = Configuration.AddressH;
          config->AddrL = Configuration.AddressL;

          // Set new configuration
          Airsoft::Devices::ResponseStatus status = lora.SetConfiguration(*config);
          if (status.code == E220_SUCCESS) {
            std::cout << "Wireless: New address configured...." << std::endl;
          } else {
            std::cout << "Wireless: ERROR to configure wireless module...." << std::endl;
          }
        }

        delete static_cast<uint8_t*>(currentConfig.data);
      }
    }

    std::cout << currentConfig.status.code << std::endl;
  }

  // Set ready flag
  _ready = true;

  // Thread loop
  while(_threadRunning) {
    Airsoft::Devices::ResponseContainer response;
    try {
      response = lora.ReceiveMessage();
      if (response.status.code == E220_SUCCESS) {
        _inLock.lock();
        _in.push(response.data);
        _inLock.unlock();
      }
    } catch(...) { }

    _outLock.lock();
    if (!_out.empty()) {
      std::string dataOut = _out.front();
     _out.pop();
     if (lora.SendBroadcastFixedMessage(0x04, dataOut).code == E220_SUCCESS) {

     }
    }
    _outLock.unlock();

    std::this_thread::sleep_for(100ms);
  }

  std::cout << "Wireless Engine: Terminated." << std::endl;
}
//------------------------------------------------------------------------------

} // namespace Airsoft
