/**
 *******************************************************************************
 * @file games.hpp
 *
 * @brief Data for games
 *
 * @author  Cristian Croci - ccdevelop.net
 *
 * @version 1.00
 *
 * @date Dec 9, 2024
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
#ifndef GAMES_HPP_
#define GAMES_HPP_

#include <cstdint>
#include <string>
#include <cstring>

namespace Airsoft::Templates {

class DisplayPage;

struct Game {
  std::string     Name;
  std::string     Description;
  int32_t         GameID;
  DisplayPage   * Page;
};


} // namespace Airsoft::Templates

#endif  // GAMES_HPP_
