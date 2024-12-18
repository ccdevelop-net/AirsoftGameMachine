/**
 *******************************************************************************
 * @file AirsoftManager.hpp
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

#ifndef AIRSOFTMANAGER_HPP_
#define AIRSOFTMANAGER_HPP_

#include <thread>
#include <gps.hpp>
#include <wireless.hpp>
#include <inout.hpp>
#include <devices/i2c-display.hpp>

namespace Airsoft {

class AirsoftManager final {
public:
  AirsoftManager() = default;
  virtual ~AirsoftManager() = default;

public:
  bool Init(void);
  void Terminate(void);

private:
  // Pointer to thread
  std::thread * _process {};
  // Running flag for thread
  bool          _threadRunning {};

  Gps                             _gps;
  Wireless                        _wireless;
  InOut                           _inout;
  Airsoft::Devices::I2CDisplay  * _display {};
  Airsoft::Drivers::I2C           _wire;

private:
  void Engine(void);

private:
  bool LoadConfiguration(void);


};

} // namespace Airsoft

#endif // AIRSOFTMANAGER_HPP_
