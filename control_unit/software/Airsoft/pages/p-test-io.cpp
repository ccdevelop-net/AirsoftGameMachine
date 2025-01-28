/**
 *******************************************************************************
 * @file p-test-io.hpp
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

#include "p-test-io.hpp"

namespace Airsoft::Pages {

//======================================================================================================================
// Implement DisplayPage Interface *************************************************************************************
//======================================================================================================================

/**
 * @brief Time for periodic function
 */
constexpr uint32_t _periodicTime = 100;

//-----------------------------------------------------------------------------
bool PTestIO::Load(Airsoft::Templates::DisplayEngine * engine) {
  // Set Engine
  _engine = engine;

  // Write welcome screen
  _engine->Clean();
  //                     "                    "
  _engine->PrintAt(0, 0, "     [Test IO]      ");
  _engine->PrintAt(0, 1, " Press keys 1 to 6  ");
  _engine->PrintAt(0, 2, "     for ON/OFF     ");
  _engine->PrintAt(0, 3, "                    ");

  // Reset Relè
  Reset();

  return true;
}
//-----------------------------------------------------------------------------
void PTestIO::Refresh(void) {

}
//-----------------------------------------------------------------------------
void PTestIO::KeyHandle(const char key, const uint8_t keyCode) {
  // Return to previous Page
  if (key == 'B') {
    // Reset IO before exit
    Reset();
    _engine->ActivatePage(nullptr);
  } else {
    switch (key) {
      case '1':
        _releStatus[RELE1 - RELE1] = !_releStatus[RELE1 - RELE1];
        InOut::Instance().Rele(RELE1, _releStatus[RELE1 - RELE1]);
        break;
      case '2':
        _releStatus[RELE2 - RELE1] = !_releStatus[RELE2 - RELE1];
        InOut::Instance().Rele(RELE2, _releStatus[RELE2 - RELE1]);
        break;
      case '3':
        _releStatus[RELE3 - RELE1] = !_releStatus[RELE3 - RELE1];
        InOut::Instance().Rele(RELE3, _releStatus[RELE3 - RELE1]);
        break;
      case '4':
        _releStatus[RELE4 - RELE1] = !_releStatus[RELE4 - RELE1];
        InOut::Instance().Rele(RELE4, _releStatus[RELE4 - RELE1]);
        break;
      case '5':
        _releStatus[RELE5 - RELE1] = !_releStatus[RELE5 - RELE1];
        InOut::Instance().Rele(RELE5, _releStatus[RELE5 - RELE1]);
        break;
      case '6':
        _releStatus[RELE6 - RELE1] = !_releStatus[RELE6 - RELE1];
        InOut::Instance().Rele(RELE6, _releStatus[RELE6 - RELE1]);
        break;
    }
  }
}
//-----------------------------------------------------------------------------
void PTestIO::Periodic(void) {

}
//-----------------------------------------------------------------------------
uint32_t PTestIO::PeriodicTime(void) const {
  return _periodicTime;
}
//-----------------------------------------------------------------------------
std::string PTestIO::Name(void) {
  return "Test IOs";
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void PTestIO::Reset(void) {
  // Reset array
  memset(_releStatus, 0x0, sizeof(_releStatus));

  // Reset Relè
  InOut::Instance().Rele(RELE1, _releStatus[RELE1 - RELE1]);
  InOut::Instance().Rele(RELE2, _releStatus[RELE2 - RELE1]);
  InOut::Instance().Rele(RELE3, _releStatus[RELE3 - RELE1]);
  InOut::Instance().Rele(RELE4, _releStatus[RELE4 - RELE1]);
  InOut::Instance().Rele(RELE5, _releStatus[RELE5 - RELE1]);
  InOut::Instance().Rele(RELE6, _releStatus[RELE6 - RELE1]);
}
//-----------------------------------------------------------------------------

} // namespace Airsoft::Pages
