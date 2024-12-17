/**
 *******************************************************************************
 * @file p-main.hpp
 *
 * @brief Main display page
 *
 * @author  Cristian Croci - ccdevelop.net
 *
 * @version 1.00
 *
 * @date Dec 12, 2024
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
#ifndef P_MAIN_HPP_
#define P_MAIN_HPP_

#include <templates/display-page.hpp>

namespace Airsoft::Pages {

class PMain : public Airsoft::Templates::DisplayPage {
public:
  PMain(void) = default;
  virtual ~PMain(void) = default;

public:
  bool Load(Airsoft::Templates::DisplayEngine * engine) override;

  void Refresh(void) override;

  void KeyHandle(const char key, const uint8_t keyCode) override;

  void Periodic(void) override;

  uint32_t PeriodicTime(void) const override;

  std::string Name(void) override;

private:
  bool          _isStarted {};
  bool          _start {};

  uint16_t      _scrollPosition {};

};

} // namespace Airsoft::Pages

#endif // P_MAIN_HPP_
