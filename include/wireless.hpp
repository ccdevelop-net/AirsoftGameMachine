/**
 *******************************************************************************
 * @file wireless.hpp
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

#ifndef WIRELESS_HPP_
#define WIRELESS_HPP_

#include <thread>
#include <queue>
#include <mutex>

namespace Airsoft {



class Wireless final {
private:
  Wireless(void) = default;

public:
  virtual ~Wireless(void) = default;

public:
  static Wireless & Instance(void) {
    static Wireless _instance;
    return _instance;
  }

public:
  bool Init(std::string port, int32_t auxPin = -1, int32_t m0Pin = -1, int32_t m1Pin = -1);
  void Terminate(void);

  void SendMessage(std::string message);
  bool ReceiveMessage(std::string & message);

  bool inline IsReady(void) {
    return _ready;
  }

private:
  std::thread * _process {};      // Pointer to thread
  bool          _threadRunning {};         // Running flag for thread
  std::string   _port;
  int32_t       _auxPin {};
  int32_t       _m0Pin {};
  int32_t       _m1Pin {};
  bool          _ready {};

  std::mutex              _outLock;
  std::queue<std::string> _out;
  std::mutex              _inLock;
  std::queue<std::string> _in;

private:
  void Engine(void);

};

} // namespace Airsoft

#endif /* WIRELESS_HPP_ */
