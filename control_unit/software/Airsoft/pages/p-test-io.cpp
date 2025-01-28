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
  _engine->PrintAt(0, 0, "Airsoft Game Machine");
  _engine->PrintAt(0, 1, "         by         ");
  _engine->PrintAt(0, 2, "   CCDevelop.NET    ");
  _engine->PrintAt(0, 3, "Press '*' to start  ");

  return true;
}
//-----------------------------------------------------------------------------
void PTestIO::Refresh(void) {

}
//-----------------------------------------------------------------------------
void PTestIO::KeyHandle(const char key, const uint8_t keyCode) {

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

} // namespace Airsoft::Pages
