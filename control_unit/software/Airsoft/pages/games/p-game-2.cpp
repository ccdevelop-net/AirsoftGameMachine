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
#include <functional>
#include <templates/display-page.hpp>
#include <templates/display-engine.hpp>
#include <templates/games.hpp>
#include <utility.hpp>
#include <inout.hpp>

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
  _engine->PrintAt(0, 3, "Press 'A' for next  ");

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
  // Reset Backlight and switch On
  if (_backlight >= PG2_BACKLIGHT_ON_OFF) {
    _engine->Backlight(Airsoft::Templates::DisplayStatus::On);
  }
  _backlight = 0;

  // Select KEY
  if (key == 'B' && !_running) {
    // Reset System Ready
    InOut::Instance().Led(LED_SYSTEM_READY, false);

    _engine->ActivatePage(nullptr);
  } else {
    char dataDisplay[21];

    if (_programStep == PG2_SETUP_TIME) {
      if (key == '*') {
        if (_gameTimeMinutes > 5) {
          _gameTimeMinutes -= 5;
        }
      } else if (key == '#') {
        if (_gameTimeMinutes < 480) {
          _gameTimeMinutes += 5;
        }
      } else if (key == 'A') {
        _programStep = PG2_TIME_SEQUENCE;
        UpdateRow(1, "Time seq. change:");
        sprintf(dataDisplay, "%u sec", _sequenceTimeSeconds);
        UpdateRow(2, dataDisplay);
        return;
      }

      sprintf(dataDisplay, "%u min", _gameTimeMinutes);
      UpdateRow(2, dataDisplay);
    } else if (_programStep == PG2_TIME_SEQUENCE) {
      if (key == '*') {
        if (_sequenceTimeSeconds > 30) {
          _sequenceTimeSeconds -= 30;
        }
      } else if (key == '#') {
        if (_sequenceTimeSeconds < 600) {
          _sequenceTimeSeconds += 30;
        }
      } else if (key == 'A') {
        _programStep = PG2_NUM_OF_RETRY;
        UpdateRow(1, "Number of retry:     ");
        sprintf(dataDisplay, "%u", _codeRetry);
        UpdateRow(2, dataDisplay);
        return;
      }

      sprintf(dataDisplay, "%u sec", _sequenceTimeSeconds);
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
        sprintf(dataDisplay, "Time: %u Retry: %u", _gameTimeMinutes, (_codeRetry - _numOfRetry));
        UpdateRow(2, dataDisplay);
        UpdateRow(3, " Press 'A' to START  ");

        // System Ready
        InOut::Instance().Led(LED_SYSTEM_READY, true);
        return;
      }
      sprintf(dataDisplay, "%u", _codeRetry);
      UpdateRow(2, dataDisplay);

    } else if (_programStep == PG2_READY) {
      if (key == 'A') {
        _programStep = PG2_RUNNING;
        _coutdown = _gameTimeMinutes * 60;
        _coutdownSequence = _sequenceTimeSeconds;


        UpdateRow(0, "   Game Running...   ");
        sprintf(dataDisplay, " %s - Retry: %u", Airsoft::Utility::CalculateHMS(_coutdown).c_str(), (_codeRetry - _numOfRetry));
        UpdateRow(1, dataDisplay);
        sprintf(dataDisplay, "Seq: %s - %03u", _sequences[_currentSequence].sequenceName.c_str(), _coutdownSequence);
        UpdateRow(2, dataDisplay);
        UpdateRow(3, " Code: ");

        // Start Timers
        _gameTime.SetInterval(std::bind(&PGame2::CountDownTimer, this), 1000);
        _sequenceTime.SetInterval(std::bind(&PGame2::SequenceTimer, this), 1000);

        // System Armed
        InOut::Instance().Led(LED_SYSTEM_READY, false);
        InOut::Instance().Led(LED_SYSTEM_ARMED, true);

        _running = true;
      }
    } else if (_programStep == PG2_RUNNING) {
      if (key == 'A' || key == 'B' || key == 'C' ||
          key == 'D'|| key == '*' || key == '#'     ) {
        if (key == 'A') {
          if (_code == _sequences[_currentSequence].passCode) {
            TerminateGame();

            _gameWin = true;

            // Clean screen
            _engine->Clean();
            //                     "                    "
            _engine->PrintAt(0, 0, "********************");
            _engine->PrintAt(0, 1, "*  Congratulation  *");
            _engine->PrintAt(0, 2, "*   YOU HAVE WIN   *");
            _engine->PrintAt(0, 3, "********************");

          } else {
            if (_gameWin) {
              return;
            }

            _numOfRetry++;
            _code.clear();
            UpdateRow(3, " Code: ");
          }
        } else if (key == 'C') {
          if (_detonated) {
            TerminateGame();
          } else {
            _code.clear();
          }
        }
      } else {
        if (_code.length() < 4) {
          _code += key;
          sprintf(dataDisplay, " Code: %s", _code.c_str());
          UpdateRow(3, dataDisplay);
        }
      }
    }
  }
}
//-----------------------------------------------------------------------------
void PGame2::Periodic(void) {
  static bool first {};
  static uint8_t refreshRunning {};

  if (_detonated) {


    if (!first) {
      first = true;
      // Clean screen
      _engine->Clean();
      //                     "                    "
      _engine->PrintAt(0, 0, "********************");
      _engine->PrintAt(0, 1, "*     You have     *");
      _engine->PrintAt(0, 2, "*     LOSE!!!!!    *");
      _engine->PrintAt(0, 3, "********************");
    }

    return;
  } else {
    first = false;
  }

  if (_backlight == PG2_BACKLIGHT_ON_OFF) {
    _engine->Backlight(Airsoft::Templates::DisplayStatus::Off);
  } else {
    _backlight++;
  }

  // Check if running
  if (_running) {
    if (++refreshRunning == 5) {
      char dataDisplay[21];
      sprintf(dataDisplay, " %s - Retry: %u", Airsoft::Utility::CalculateHMS(_coutdown).c_str(), (_codeRetry - _numOfRetry));
      UpdateRow(1, dataDisplay);
      sprintf(dataDisplay, "Seq: %s - %03u", _sequences[_currentSequence].sequenceName.c_str(), _coutdownSequence);
      UpdateRow(2, dataDisplay);
      refreshRunning = 0;
    }
  }
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
void PGame2::CountDownTimer(void) {
  if (_coutdown == 0 || _numOfRetry == _codeRetry) {
    _sequenceTime.Stop();
    InOut::Instance().Led(LED_SYSTEM_ARMED, false);
    InOut::Instance().Led(LED_SYSTEM_ACTIVE, true);
    InOut::Instance().Rele(RELE_SYSTEM_SIREN, true);

    _detonated = true;
    _gameTime.Stop();
    return;
  }

  _coutdown--;
}
//-----------------------------------------------------------------------------
void PGame2::SequenceTimer(void) {
  if (_coutdownSequence == 0) {
    _coutdownSequence = _sequenceTimeSeconds;
    if (_currentSequence + 1 < PG2_NUM_OF_SEQUENCES) {
      _currentSequence++;
    } else {
      _currentSequence = 0;
    }
    return;
  }

  _coutdownSequence--;

}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void PGame2::UpdateRow(uint8_t row, const char * str) {
  _engine->CleanRow(row);
  _engine->PrintAt(0, row, str);
}
//-----------------------------------------------------------------------------
void PGame2::TerminateGame(void) {
  // Terminate Timers
  _sequenceTime.Stop();
  _gameTime.Stop();

  // Reset all IO
  InOut::Instance().Led(LED_SYSTEM_ARMED, false);
  InOut::Instance().Led(LED_SYSTEM_ACTIVE, false);
  InOut::Instance().Rele(RELE_SYSTEM_SIREN, false);

  _detonated = false;

  _running = false;

}
//-----------------------------------------------------------------------------


} // namespace Airsoft::Pages::Games
