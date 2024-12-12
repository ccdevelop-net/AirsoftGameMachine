/**
 *******************************************************************************
 * @file AirsoftManager.hpp
 *
 * @brief Main application header file
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
#ifndef AIRSOFTMANAGER_HPP_
#define AIRSOFTMANAGER_HPP_

#include <list>
#include <thread>

#include <templates/display-engine.hpp>
#include <templates/display-page.hpp>

#include <gps.hpp>
#include <wireless.hpp>
#include <inout.hpp>
#include <devices/i2c-display.hpp>

namespace Airsoft {

class AirsoftManager final : public Airsoft::Templates::DisplayEngine {
public:
  AirsoftManager() = default;
  virtual ~AirsoftManager() = default;

public:
  bool Init(void);
  void Terminate(void);

public:
  void Clean(void) override;
  void CleanRow(uint8_t row) override;

  void Print(char val) override;
  void Print(const char * str) override;
  void Print(std::string str) override;

  void PrintAt(uint8_t col, uint8_t row, char val) override;
  void PrintAt(uint8_t col, uint8_t row, const char * str) override;
  void PrintAt(uint8_t col, uint8_t row, std::string str) override;

  void MoveCursor(uint8_t col, uint8_t row) override;

  void Backlight(Airsoft::Templates::DisplayStatus status) override;
  void Display(Airsoft::Templates::DisplayStatus status) override;

  bool ActivatePage(Airsoft::Templates::DisplayPage * pageToActivate) override;

private:
  std::thread           *                       _process {};          // Pointer to thread
  bool                                          _threadRunning {};    // Running flag for thread
  bool                                          _threadTerminated {}; // Terminated flag for thread

  std::mutex                                    _pagesLock;
  bool                                          _newPageAvailable {}; // Indicate new page must be called
  Airsoft::Templates::DisplayPage *             _newPage {};          // New page

  Airsoft::Templates::DisplayPage *             _currentPage {};      // Current page
  std::list<Airsoft::Templates::DisplayPage*>   _pages;               // List of the pages

private:
  void Engine(void);

private:
  bool LoadConfiguration(void);

  inline Airsoft::InOut & _InOut(void) { return Airsoft::InOut::Instance(); }
  inline Airsoft::Wireless & _Wireless(void) { return Airsoft::Wireless::Instance(); }
  inline Airsoft::Gps & _Gps(void) { return Airsoft::Gps::Instance(); }
  inline Airsoft::Devices::I2CDisplay & _Display(void) { return Airsoft::Devices::I2CDisplay::Instance(); }


};

} // namespace Airsoft

#endif // AIRSOFTMANAGER_HPP_
