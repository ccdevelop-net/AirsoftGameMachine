/**
 *******************************************************************************
 * @file display-engine.hpp
 *
 * @brief Abstract class interface for manage display from a page
 *
 * @author  Cristian Croci - CCDevelop.net
 *
 * @version 1.00
 *
 * @date Dec 9, 2024
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
#ifndef DISPLAY_ENGINE_HPP_
#define DISPLAY_ENGINE_HPP_

#include <cstdint>
#include <string>
#include <cstring>

namespace Airsoft::Templates {

/**
 * @brief Status of backlight
 */
enum class DisplayStatus {
  On,  /**< On */
  Off, /**< Off */
};

class DisplayPage;

class DisplayEngine {

protected:
  DisplayEngine(void) = default;
  virtual ~DisplayEngine(void) = default;

public:
  virtual void Clean(void) = 0;
  virtual void CleanRow(uint8_t row) = 0;

  virtual void Print(char val) = 0;
  virtual void Print(const char * str) = 0;
  virtual void Print(std::string str) = 0;

  virtual void PrintAt(uint8_t col, uint8_t row, char val) = 0;
  virtual void PrintAt(uint8_t col, uint8_t row, const char * str) = 0;
  virtual void PrintAt(uint8_t col, uint8_t row, std::string str) = 0;

  virtual void MoveCursor(uint8_t col, uint8_t row) = 0;

  virtual void Backlight(DisplayStatus status) = 0;
  virtual void Display(DisplayStatus status) = 0;

  /**
   * @brief A page activate another page, if /pageToActivate/ is NULL
   *        previous page is restored
   * @param pageToActivate - Pointer to a page for activation
   * @retval Return true if new page is set
   */
  virtual bool ActivatePage(DisplayPage * pageToActivate) = 0;
};

} // namespace Airsoft::Templates

#endif  // DISPLAY_ENGINE_HPP_
