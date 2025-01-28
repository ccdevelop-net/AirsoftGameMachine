/**
 *******************************************************************************
 * @file p-test-led.hpp
 *
 * @brief Test Led page
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
#include <templates/display-page.hpp>
#include <templates/display-engine.hpp>

#include <inout.hpp>

#include "p-test-led.hpp"

namespace Airsoft::Pages {

//======================================================================================================================
// Implement DisplayPage Interface *************************************************************************************
//======================================================================================================================

/**
 * @brief Time for periodic function
 */
constexpr uint32_t _periodicTime = 100;

//-----------------------------------------------------------------------------
bool PTestLeds::Load(Airsoft::Templates::DisplayEngine * engine) {
  // Set Engine
  _engine = engine;

  // Write welcome screen
  _engine->Clean();
  //                     "                    "
  _engine->PrintAt(0, 0, "    [Test Leds]     ");
  _engine->PrintAt(0, 1, " Press keys 1 to 5  ");
  _engine->PrintAt(0, 2, "     for ON/OFF     ");
  _engine->PrintAt(0, 3, "                    ");

  // Reset Leds
  Reset();

  return true;
}
//-----------------------------------------------------------------------------
void PTestLeds::Refresh(void) {

}
//-----------------------------------------------------------------------------
void PTestLeds::KeyHandle(const char key, const uint8_t keyCode) {
  // Return to previous Page
  if (key == 'B') {
    // Reset Leds
    Reset();
    _engine->ActivatePage(nullptr);
  } else {
    switch (key) {
      case '1':
        _ledStatus[LED1 - LED1] = !_ledStatus[LED1 - LED1];
        InOut::Instance().Led(LED1, _ledStatus[LED1 - LED1]);
        break;
      case '2':
        _ledStatus[LED1 - LED1] = !_ledStatus[LED2 - LED1];
        InOut::Instance().Led(LED2, _ledStatus[LED2 - LED1]);
        break;
      case '3':
        _ledStatus[LED1 - LED1] = !_ledStatus[LED3 - LED1];
        InOut::Instance().Led(LED3, _ledStatus[LED3 - LED1]);
        break;
      case '4':
        _ledStatus[LED1 - LED1] = !_ledStatus[LED4 - LED1];
        InOut::Instance().Led(LED4, _ledStatus[LED4 - LED1]);
        break;
      case '5':
        _ledStatus[LED1 - LED1] = !_ledStatus[LED5 - LED1];
        InOut::Instance().Led(RELE5, _ledStatus[LED5 - LED1]);
        break;
    }
  }
}
//-----------------------------------------------------------------------------
void PTestLeds::Periodic(void) {

}
//-----------------------------------------------------------------------------
uint32_t PTestLeds::PeriodicTime(void) const {
  return _periodicTime;
}
//-----------------------------------------------------------------------------
std::string PTestLeds::Name(void) {
  return "Test Leds";
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void PTestLeds::Reset(void) {
  // Reset array
  memset(_ledStatus, 0x0, sizeof(_ledStatus));

  // Reset Leds
  InOut::Instance().Led(LED1, _ledStatus[LED1 - LED1]);
  InOut::Instance().Led(LED2, _ledStatus[LED2 - LED1]);
  InOut::Instance().Led(LED3, _ledStatus[LED3 - LED1]);
  InOut::Instance().Led(LED4, _ledStatus[LED4 - LED1]);
  InOut::Instance().Led(LED5, _ledStatus[LED5 - LED1]);
}
//-----------------------------------------------------------------------------

} // namespace Airsoft::Pages
