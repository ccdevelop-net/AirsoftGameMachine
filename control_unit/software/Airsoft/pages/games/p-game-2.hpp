/**
 *******************************************************************************
 * @file p-game-2.hpp
 *
 * @brief Game Title: "Red Scoprion - The Rebirth" page header file
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
#ifndef P_GAME_2_HPP_
#define P_GAME_2_HPP_

#include <templates/display-page.hpp>

namespace Airsoft::Pages::Games {

struct CodeSequences {
  std::string     passCode;
  std::string     sequenceName;
};

constexpr uint8_t PG2_NUM_OF_SEQUENCES = 4;

constexpr uint8_t PG2_SETUP_TIME = 1;
constexpr uint8_t PG2_NUM_OF_RETRY = 2;
constexpr uint8_t PG2_READY = 3;
constexpr uint8_t PG2_RUNNING = 4;


class PGame2 : public Airsoft::Templates::DisplayPage {
public:
  PGame2(void) = default;
  virtual ~PGame2(void) = default;

public:
  bool Load(Airsoft::Templates::DisplayEngine * engine) override;

  void Refresh(void) override;

  void KeyHandle(const char key, const uint8_t keyCode) override;

  void Periodic(void) override;

  uint32_t PeriodicTime(void) const override;

  std::string Name(void) override;

private:
  bool      _running {};

  uint32_t      _gameTimeMinutes { 240 };
  uint32_t      _codeRetry { 3 };
  uint32_t      _sequenceTimeSeconds { 180 };

  uint32_t      _coutdown {};
  uint32_t      _coutdownSequence {};
  uint8_t       _numOfRetry {};
  uint8_t       _currentSequence {};
  std::string   _code;

  uint8_t       _programStep { PG2_SETUP_TIME };


  CodeSequences _sequences[PG2_NUM_OF_SEQUENCES] {
    { "9035", "Alpha"   },
    { "5039", "Charlie" },
    { "3095", "Yenkee"  },
    { "0953", "Zulu"    }
  };

private:
  void UpdateRow(uint8_t row, const char * str);
  void TerminateGame(void);
};

} // namespace Airsoft::Pages::Games

#endif // P_GAME_2_HPP_
