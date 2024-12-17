/**
 *******************************************************************************
 * @file Gps.hpp
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
#ifndef GPS_HPP_
#define GPS_HPP_

#include <string>
#include <cstring>
#include <thread>

#include <neo/gps-nema.hpp>

namespace Airsoft {

class Gps final {
private:
  Gps(void) = default;  // Only one instance
public:
  virtual ~Gps(void) = default;

public:
  static Gps & Instance(void) {
    static Gps _instance;
    return _instance;
  }

public:
  bool Init(std::string port);
  void Terminate(void);

  bool inline IsReady(void) {
    return _ready;
  }

  inline Airsoft::Neo::GpsFix & Data(void) {
    return _fix;
  }

private:
  std::thread     *       _process {};          // Pointer to thread
  bool                    _threadRunning {};    // Running flag for thread
  std::string             _port;

  bool                    _ready {};

  Airsoft::Neo::GpsNema   _nema;                // Gps Nema string parser

  //  Define a set of GPS fix information.  It will
  //  hold on to the various pieces as they are received from
  //  an RMC sentence.  It can be used anywhere in your sketch.
  Airsoft::Neo::GpsFix    _fix;

private:
  void Engine(void);

};

} // namespace Airsoft

#endif // GPS_HPP_
