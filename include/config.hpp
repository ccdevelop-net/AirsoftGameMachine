/**
 *******************************************************************************
 * @file config.hpp
 *
 * @brief Configuration of application
 *
 * @author  Cristian Croci - CCDevelop.net
 *
 * @version 1.00
 *
 * @date Dec 3, 2024
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
#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <cstdint>
#include <list>

namespace Airsoft {

struct Game {
  std::string Name;

};

struct AsmConfiguration {
  uint8_t         AddressH;         // Wireless Address part high
  uint8_t         AddressL;         // Wireless Address part low

  std::list<Game> Games;
};

extern AsmConfiguration   Configuration;

}

#endif /* CONFIG_HPP_ */
