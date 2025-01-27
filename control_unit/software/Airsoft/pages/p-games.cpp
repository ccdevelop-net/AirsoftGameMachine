/**
 *******************************************************************************
 * @file p-games.hpp
 *
 * @brief Games page
 *
 * @author  Cristian Croci - ccdevelop.net
 *
 * @version 1.00
 *
 * @date Dec 24, 2024
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

#include "p-games.hpp"

namespace Airsoft::Pages {

constexpr int32_t TIME_GAME                   = 1;
constexpr int32_t RED_SCORPION_THE_REBIRTH    = 2;


static Airsoft::Templates::Game games[] {
  { "Time Game           ", "Simple time game    ",  TIME_GAME,                 nullptr },
  { "Red S: The Rebirth  ", "Please see game book",  RED_SCORPION_THE_REBIRTH,  nullptr },
  { "",                     "",                     -1,                         nullptr }
};

//======================================================================================================================
// Implement DisplayPage Interface *************************************************************************************
//======================================================================================================================

/**
 * @brief Time for periodic function
 */
constexpr uint32_t _periodicTime = 100;

//-----------------------------------------------------------------------------
bool PGames::Load(Airsoft::Templates::DisplayEngine * engine) {
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
void PGames::Refresh(void) {

}
//-----------------------------------------------------------------------------
void PGames::KeyHandle(const char key, const uint8_t keyCode) {

}
//-----------------------------------------------------------------------------
void PGames::Periodic(void) {

}
//-----------------------------------------------------------------------------
uint32_t PGames::PeriodicTime(void) const {
  return _periodicTime;
}
//-----------------------------------------------------------------------------
std::string PGames::Name(void) {
  return "Page Gps";
}
//-----------------------------------------------------------------------------

} // namespace Airsoft::Pages
