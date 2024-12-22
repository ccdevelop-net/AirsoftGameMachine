#ifndef GPSTIME_H
#define GPSTIME_H

//  Copyright (C) 2014-2017, SlashDevin
//
//  This file is part of NeoGPS
//
//  NeoGPS is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  NeoGPS is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with NeoGPS.  If not, see <http://www.gnu.org/licenses/>.

#include "neo-time.hpp"

namespace Airsoft::Neo {

class GPSTime {
  GPSTime();

  static clock_t startOfWeek;

public:

    /**
     * GPS time is offset from UTC by a number of leap seconds.  To convert a GPS
     * time to UTC time, the current number of leap seconds must be known.
     * See http://en.wikipedia.org/wiki/Global_Positioning_System#Leap_seconds
     */
    static uint8_t leapSeconds;

    /**
     * Some receivers report time WRT start of the current week, defined as
     * Sunday 00:00:00.  To save fairly expensive date/time calculations,
     * the UTC start of week is cached
     */
    static void StartOfWeek(TimeT & now) {
      now.SetDay();
      startOfWeek = (clock_t)now - (clock_t)((((now.Day-1) * 24L + now.Hours) * 60L + now.Minutes) * 60L + now.Seconds);
    }

    static clock_t StartOfWeek(void) {
      return startOfWeek;
    }

    /**
     * @brief Convert a GPS time-of-week to UTC.
     *        Requires leap_seconds and start_of_week.
     */
    static clock_t TOW2UTC( uint32_t timeOfWeek ) {
      return (clock_t)(StartOfWeek() + timeOfWeek - leapSeconds);
    }

    /**
     * Set /fix/ timestamp from a GPS time-of-week in milliseconds.
     * Requires /leap_seconds/ and /start_of_week/.
     **/
    static bool FromTOWms(uint32_t timeOfWeekMs, TimeT & dt, uint16_t &ms) {
      bool ok { (StartOfWeek() != 0) && (leapSeconds != 0) };
      if (ok) {
        clock_t tow_s { timeOfWeekMs / 1000UL };
        dt = TOW2UTC(tow_s);
        ms = (uint16_t)(timeOfWeekMs - tow_s * 1000UL);
      }

      return ok;
    }
};

} // namespace Airsoft::Neo

#endif
