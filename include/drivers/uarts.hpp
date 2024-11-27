/**
 *******************************************************************************
 * @file Uarts.hpp
 *
 * @brief Description
 *
 * @author  Cristian
 *
 * @version 1.00
 *
 * @date Nov 27, 2024
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

#ifndef DRIVERS_UARTS_HPP_
#define DRIVERS_UARTS_HPP_

namespace Airsoft::Drivers {

class Uarts final {
public:
  Uarts();
  virtual ~Uarts();
};

} // namespace Airsoft::Drivers

#endif // DRIVERS_UARTS_HPP_
