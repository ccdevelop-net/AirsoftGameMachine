/**
 *******************************************************************************
 * @file p-test-led.hpp
 *
 * @brief Test Led page header file
 *
 * @author  Cristian Croci - ccdevelop.net
 *
 * @version 1.00
 *
 * @date Jan 28, 2025
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
#ifndef P_TEST_LEDS_HPP_
#define P_TEST_LEDS_HPP_

#include <templates/display-page.hpp>

namespace Airsoft::Pages {

class PTestLeds : public Airsoft::Templates::DisplayPage {
public:
  PTestLeds(void) = default;
  virtual ~PTestLeds(void) = default;

public:
  bool Load(Airsoft::Templates::DisplayEngine * engine) override;

  void Refresh(void) override;

  void KeyHandle(const char key, const uint8_t keyCode) override;

  void Periodic(void) override;

  uint32_t PeriodicTime(void) const override;

  std::string Name(void) override;

};

} // namespace Airsoft::Pages

#endif // P_TEST_LEDS_HPP_
