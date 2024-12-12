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

#include <cstdlib>

#include <classes/print.hpp>
#include <neo/neo-time.hpp>

Airsoft::Classes::Print & operator<<(Airsoft::Classes::Print& outs, const Airsoft::Neo::TimeT & t )
{
  outs.Write(t.FullYear(t.Year));
  outs.Write('-');
  if (t.Month < 10) {
    outs.Write('0');
  }
  outs.Write(t.Month);
  outs.Write('-');
  if (t.Date < 10) {
    outs.Write('0');
  }
  outs.Write(t.Date);
  outs.Write(' ');
  if (t.Hours < 10) {
    outs.Write('0');
  }
  outs.Write(t.Hours);
  outs.Write(':');
  if (t.Minutes < 10) {
    outs.Write('0');
  }
  outs.print(t.Minutes);
  outs.Write(':');
  if (t.Seconds < 10) {
    outs.Write('0');
  }
  outs.print(t.Seconds);

  return outs;
}

namespace Airsoft::Neo {

bool TimeT::Parse(std::string s) {
  static size_t BUF_MAX = 32;
  char buf[BUF_MAX];
  strcpy(buf, s.c_str());
  char* sp = &buf[0];
  uint16_t value = strtoul(sp, &sp, 10);

  if (*sp != '-') {
    return false;
  }
  Year = value % 100;
  if (FullYear() != value) {
    return false;
  }

  value = strtoul(sp + 1, &sp, 10);
  if (*sp != '-') {
    return false;
  }
  Month = value;

  value = strtoul(sp + 1, &sp, 10);
  if (*sp != ' ') {
    return false;
  }
  Date = value;

  value = strtoul(sp + 1, &sp, 10);
  if (*sp != ':') {
    return false;
  }
  Hours = value;

  value = strtoul(sp + 1, &sp, 10);
  if (*sp != ':') {
    return false;
  }
  Minutes = value;

  value = strtoul(sp + 1, &sp, 10);
  if (*sp != 0) {
    return false;
  }
  Seconds = value;

  return IsValid();
}

#ifdef TIME_EPOCH_MODIFIABLE
uint16_ time_t::_epochYear    = Y2K_EPOCH_YEAR;
uint8_t time_t::_epochOffset  = 0;
uint8_t time_t::_epochWeekday = Y2K_EPOCH_WEEKDAY;
uint8_t time_t::_pivotYear    = 0;
#endif

const uint8_t TimeT::DaysIn[] = {
  0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

TimeT::TimeT(clock_t c) {
  uint16_t dayno { (uint16_t)(c / SECONDS_PER_DAY) };
  c -= dayno * (uint32_t) SECONDS_PER_DAY;
  Day = WeekdayFor(dayno);

  uint16_t y { EpochYear() };
  for (;;) {
    uint16_t days = DaysPer( y );
    if (dayno < days) {
      break;
    }
    dayno -= days;
    y++;
  }
  bool leapYear { IsLeap(y) };
  y -= EpochYear();
  y += EpochOffset();
  while (y > 100) {
    y -= 100;
  }
  Year = y;

  Month = 1;
  for (;;) {
    uint8_t days = DaysIn[Month];
    if (leapYear && (Month == 2)) {
      days++;
    }
    if (dayno < days) {
      break;
    }
    dayno -= days;
    Month++;
  }
  Date = dayno + 1;

  Hours = c / SECONDS_PER_HOUR;

  uint16_t c_ms;
  if (Hours < 18) { // save 16uS
    c_ms = (uint16_t) c - (Hours * (uint16_t) SECONDS_PER_HOUR);
  } else {
    c_ms = c - (Hours * (uint32_t) SECONDS_PER_HOUR);
  }
  Minutes = c_ms / SECONDS_PER_MINUTE;
  Seconds = c_ms - (Minutes * SECONDS_PER_MINUTE);
}

void TimeT::Init(void) {
  Seconds =
  Hours   =
  Minutes = 0;
  Date    = 1;
  Month   = 1;
  Year    = EpochYear() % 100;
  Day     = EpochWeekday();
}

TimeT::operator clock_t(void) const {
  clock_t c = Days() * SECONDS_PER_DAY;
  if (Hours < 18) {
    c += Hours * (uint16_t) SECONDS_PER_HOUR;
  } else {
    c += Hours * (uint32_t) SECONDS_PER_HOUR;
  }
  c += Minutes * (uint16_t) SECONDS_PER_MINUTE;
  c += Seconds;

  return (c);
}

uint16_t TimeT::Days(void) const {
  uint16_t dayCount = DayOfYear();

  uint16_t y = FullYear();
  while (y-- > EpochYear()) {
    dayCount += DaysPer(y);
  }

  return dayCount;
}

uint16_t TimeT::DayOfYear(void) const {
  uint16_t dayno = Date - 1;
  bool leapYear = IsLeap();

  for (uint8_t m = 1; m < Month; m++) {
    dayno += DaysIn[m];
    if (leapYear && (m == 2)) {
      dayno++;
    }
  }

  return dayno;
}

#ifdef TIME_EPOCH_MODIFIABLE
void TimeT::UseFastestEpoch(void) {
  // Figure out when we were compiled and use the year for a really
  // fast epoch_year. Format "MMM DD YYYY"
  const char* compile_date = (const char *) PSTR(__DATE__);
  uint16_t    compile_year = 0;
  for (uint8_t i = 7; i < 11; i++)
    compile_year = compile_year*10 + (pgm_read_byte(&compile_date[i]) - '0');

  // Temporarily set a Y2K epoch so we can figure out the day for
  // January 1 of this year
  epoch_year      ( Y2K_EPOCH_YEAR );
  epoch_weekday   ( Y2K_EPOCH_WEEKDAY );

  time_t this_year(0);
  this_year.year = compile_year % 100;
  this_year.set_day();
  uint8_t compile_weekday = this_year.day;

  epoch_year   ( compile_year );
  epoch_weekday( compile_weekday );
  pivot_year   ( this_year.year );
}
#endif

} // namespace Airsoft::Neo
