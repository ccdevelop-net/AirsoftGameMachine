/**
 *******************************************************************************
 * @file p-game-2.hpp
 *
 * @brief Game Title: "Red Scoprion - The Rebirth" page
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
#include <templates/games.hpp>

#include "p-game-2.hpp"

namespace Airsoft::Pages::Games {

//======================================================================================================================
// Implement DisplayPage Interface *************************************************************************************
//======================================================================================================================

/**
 * @brief Time for periodic function
 */
constexpr uint32_t _periodicTime = 100;

//-----------------------------------------------------------------------------
bool PGame2::Load(Airsoft::Templates::DisplayEngine * engine) {
  // Set Engine
  _engine = engine;

  // Clean screen
  _engine->Clean();
  //                     "                    "
  _engine->PrintAt(0, 0, "  Select the Game   ");
  _engine->PrintAt(0, 1, "                    ");
  _engine->PrintAt(0, 2, "                    ");
  _engine->PrintAt(0, 3, "Press '*' to select ");

  return true;
}
//-----------------------------------------------------------------------------
void PGame2::Refresh(void) {

}
//-----------------------------------------------------------------------------
void PGame2::KeyHandle(const char key, const uint8_t keyCode) {

}
//-----------------------------------------------------------------------------
void PGame2::Periodic(void) {

}
//-----------------------------------------------------------------------------
uint32_t PGame2::PeriodicTime(void) const {
  return _periodicTime;
}
//-----------------------------------------------------------------------------
std::string PGame2::Name(void) {
  return "Red Scoprion - The Rebirth";
}
//-----------------------------------------------------------------------------

} // namespace Airsoft::Pages::Games
