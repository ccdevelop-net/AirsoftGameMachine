/**
 *******************************************************************************
 * @file display-page.hpp
 *
 * @brief Abstract class interface for display page
 *
 * @author  Cristian Croci - ccdevelop.net
 *
 * @version 1.00
 *
 * @date Dec 9, 2024
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
#ifndef DISPLAY_PAGE_HPP_
#define DISPLAY_PAGE_HPP_

#include <cstdint>
#include <string>
#include <cstring>

namespace Airsoft::Templates {

class DisplayEngine;

class DisplayPage {
public:
  virtual ~DisplayPage(void) = default;

protected:
  DisplayPage(void) = default;


public:
  /**
   * @brief Load page
   * @param engine - Pointer to display engine
   */
  virtual bool Load(DisplayEngine* engine) = 0;

  /**
   * @brief Refresh a page, usually the is reactivate
   */
  virtual void Refresh(void) = 0;

  /**
   * @brief Handle a key
   * @param key - Key char
   * @param keyCode - Key code
   */
  virtual void KeyHandle(const char key, const uint8_t keyCode) = 0;

  /**
   * @brief
   */
  virtual void Periodic(void) = 0;

  /**
   * @brief Time interval for periodic function
   * @return Periodic function call time interval expressed in milliseconds
   */
  virtual uint32_t PeriodicTime(void) const = 0;

  /**
   * @brief Name of the page
   * @return Name of the current page
   */
  virtual std::string Name(void) = 0;

protected:
  /**
   * @brief Pointer to page engine
   */
  DisplayEngine *     _engine {};

};


} // namespace Airsoft::Templates

#endif  // DISPLAY_ENGINE_HPP_
