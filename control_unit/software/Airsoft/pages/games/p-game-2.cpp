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
#include <utility.hpp>

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
  _engine->PrintAt(0, 0, "   [The Rebirth]    ");
  _engine->PrintAt(0, 1, "Insert game time:   ");
  _engine->PrintAt(0, 2, "                    ");
  _engine->PrintAt(0, 3, "Press '*' to select ");

  char data[21];
  sprintf(data, "%u min", _gameTimeMinutes);
  UpdateRow(2, data);

  return true;
}
//-----------------------------------------------------------------------------
void PGame2::Refresh(void) {

}
//-----------------------------------------------------------------------------
void PGame2::KeyHandle(const char key, const uint8_t keyCode) {
  if (key == 'B' && !_running) {
    _engine->ActivatePage(nullptr);
  } else {
    char dataDisplay[21];

    if (_programStep == PG2_SETUP_TIME) {
      if (key == '*') {
        if (_gameTimeMinutes > 0) {
          _gameTimeMinutes -= 30;
        }
      } else if (key == '#') {
        if (_gameTimeMinutes < 480) {
          _gameTimeMinutes += 30;
        }
      } else if (key == 'A') {
        _programStep = PG2_NUM_OF_RETRY;
        UpdateRow(1, "Number of retry:     ");
        return;
      }

      sprintf(dataDisplay, "%u min", _gameTimeMinutes);
      UpdateRow(2, dataDisplay);
    } else if (_programStep == PG2_NUM_OF_RETRY) {
      if (key == '*') {
        if (_codeRetry > 1) {
          _codeRetry--;
        }
      } else if (key == '#') {
        if (_codeRetry < 10) {
          _codeRetry++;
        }
      } else if (key == 'A') {
        _programStep = PG2_READY;
        UpdateRow(1, " Game ready to START ");
        sprintf(dataDisplay, "Time: %u Retry: %u", _gameTimeMinutes, _numOfRetry);
        UpdateRow(2, dataDisplay);
        UpdateRow(3, " Press 'A' to START  ");
        return;
      }
      sprintf(dataDisplay, "%u", _gameTimeMinutes);
      UpdateRow(2, dataDisplay);
    } else if (_programStep == PG2_READY) {
      if (key == 'A') {
        _programStep = PG2_RUNNING;
        _coutdown = _gameTimeMinutes * 60;
        _coutdownSequence = _sequenceTimeSeconds;
        UpdateRow(0, "   Game Running...   ");
        sprintf(dataDisplay, " %s - Retry: %u", Airsoft::Utility::CalculateHMS(_coutdown).c_str(), (_codeRetry - _numOfRetry));
        UpdateRow(1, dataDisplay);
        sprintf(dataDisplay, "Sequence: %s - %03u", _sequences[_currentSequence].sequenceName.c_str(), _coutdownSequence);
        UpdateRow(2, dataDisplay);
        UpdateRow(3, " Code: ");
        _running = true;
      }
    } else if (_programStep == PG2_RUNNING) {
      if (key == 'A' || key == 'B' || key == 'C' ||
          key == 'D'|| key == '*' || key == '#'     ) {
        if (key == 'A') {
          if (_code == _sequences[_currentSequence].passCode) {
            TerminateGame();
          }
        } else if (key == 'C') {
          _code.clear();
        }
      } else {
        if (_code.length() < 4) {
          char kcode = key;
          _code.append(&kcode);
        }
      }
    }
  }
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

//-----------------------------------------------------------------------------
void PGame2::UpdateRow(uint8_t row, const char * str) {
  _engine->CleanRow(2);
  _engine->PrintAt(0, 2, str);
}
//-----------------------------------------------------------------------------
void PGame2::TerminateGame(void) {
}
//-----------------------------------------------------------------------------


} // namespace Airsoft::Pages::Games
