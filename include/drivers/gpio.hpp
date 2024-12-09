/**
 *******************************************************************************
 * @file gpio.hpp
 *
 * @brief Description
 *
 * @author  Cristian
 *
 * @version 1.00
 *
 * @date November 27, 2024
 *
 *******************************************************************************
 * This file is part of the Airsoft project 
 * https://github.com/xxxx or http://xxx.github.io.
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

#ifndef DRIVERS_GPIO_HPP_
#define DRIVERS_GPIO_HPP_

#include <cstdint>
#include <cstdio>
#include <string>
#include <fstream>

namespace Airsoft::Drivers {

/**
 * @brief Define GPIO Banks
 */
constexpr uint32_t BANK_0 = 0;
constexpr uint32_t BANK_1 = 1;
constexpr uint32_t BANK_2 = 2;
constexpr uint32_t BANK_3 = 3;
constexpr uint32_t BANK_4 = 4;

/**
 * @brief Define GPIO Groups
 */
constexpr uint8_t GROUP_A = 0;
constexpr uint8_t GROUP_B = 1;
constexpr uint8_t GROUP_C = 2;
constexpr uint8_t GROUP_D = 3;

/**
 * @brief Define GPIO pins ID
 */
constexpr uint32_t ID_0 = 0;
constexpr uint32_t ID_1 = 1;
constexpr uint32_t ID_2 = 2;
constexpr uint32_t ID_3 = 3;
constexpr uint32_t ID_4 = 4;
constexpr uint32_t ID_5 = 5;
constexpr uint32_t ID_6 = 6;
constexpr uint32_t ID_7 = 7;

enum class Direction {
  Input,
  Output
};

enum class Level {
  Low,
  High
};


class Gpio final {
public:
  Gpio(uint32_t bank, uint8_t group, uint32_t id);
  Gpio(uint32_t gpioPin);
  virtual ~Gpio();

public:
  bool Open(Direction direction, Level level = Level::Low);
  void Close(void);

  void Set(void);
  void Reset(void);
  void Toggle(void);
  bool Read(void);

public:
  static inline uint32_t CalculateGpioId(uint32_t bank, uint8_t group, uint32_t id) {
    return (bank * 32) + ((group * 8) + id);
  }

private:
  bool          _isOpen {};
  uint32_t      _gpioPin {};
  Direction     _direction { Direction::Input };
  std::ofstream _valueOutput;
  std::string   _valuePath;
  std::string   _catCommand;
  Level         _currentLevel { Level::Low };

  bool          _outState {};

};

} // namespace Airsoft::Drivers

#endif // DRIVERS_GPIO_HPP_
