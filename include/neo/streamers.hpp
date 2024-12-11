/**
 *******************************************************************************
 * @file streamers.hpp
 *
 * @brief Description
 *
 * @author  Cristian
 *
 * @version 1.00
 *
 * @date Dec 3, 2024
 *
 *******************************************************************************
 * This file is part of the Airsoft project
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

#ifndef STREAMERS_HPP_
#define STREAMERS_HPP_

#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstdbool>
#include <string>
#include <cstring>

#include <classes/print.hpp>

namespace Airsoft::Neo {

extern Airsoft::Classes::Print & operator <<(Airsoft::Classes::Print & outs, const bool b );
extern Airsoft::Classes::Print & operator <<(Airsoft::Classes::Print & outs, const char c );
extern Airsoft::Classes::Print & operator <<(Airsoft::Classes::Print & outs, const uint16_t v );
extern Airsoft::Classes::Print & operator <<(Airsoft::Classes::Print & outs, const uint32_t v );
extern Airsoft::Classes::Print & operator <<(Airsoft::Classes::Print & outs, const int32_t v );
extern Airsoft::Classes::Print & operator <<(Airsoft::Classes::Print & outs, const uint8_t v );

class gps_fix;

/**
 * Print valid fix data to the given stream with the format
 *   "status,dateTime,lat,lon,heading,speed,altitude,satellites,
 *       hdop,vdop,pdop,lat_err,lon_err,alt_err"
 * The "header" above contains the actual compile-time configuration.
 * A comma-separated field will be empty if the data is NOT valid.
 * @param[in] outs output stream.
 * @param[in] fix gps_fix instance.
 * @return iostream.
 */
extern Airsoft::Classes::Print & operator <<(Airsoft::Classes::Print &outs, const gps_fix &fix );

class NMEAGPS;

extern void TraceHeader(Airsoft::Classes::Print & outs );
extern void TraceAll(Airsoft::Classes::Print & outs, const NMEAGPS &gps, const gps_fix &fix );

} // namespace Airsoft::Neo

#endif
