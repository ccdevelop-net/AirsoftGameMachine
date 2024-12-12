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

#include <classes/print.hpp>

#include <neo/dms.hpp>

namespace Airsoft::Neo {

//-----------------------------------------------------------------------------
void Dms::From(int32_t deg_1E7) {
  const uint32_t E7 { 10000000UL };

  if (deg_1E7 < 0) {
    deg_1E7 = -deg_1E7;
    hemisphere = Hemisphere::SOUTH_H; // or WEST_H
  } else {
    hemisphere = Hemisphere::NORTH_H; // or EAST_H
  }

  const uint32_t div_E32 { 429 }; // 1e-07 * 2^32
  degrees = ((deg_1E7 >> 16) * div_E32) >> 16;
  uint32_t remainder = deg_1E7 - degrees * E7;

  remainder *= 60; // to minutes * E7
  minutes = ((remainder >> 16) * div_E32) >> 16;
  remainder -= minutes * E7;

  remainder *= 60; // to seconds * E7
  uint32_t secs = ((remainder >> 16) * div_E32) >> 16;
  remainder -= secs * E7;

  const uint32_t div_1E4_E24 { 1677 }; // 0.00001 * 2^24
  secondsFrac  = (((remainder >> 8) * div_1E4_E24) >> 16); // thousand
  secondsWhole = secs;

  // Carry if thousand too big
  if (secondsFrac >= 1000) {
    secondsFrac -= 1000;
    secondsWhole++;
    if (secondsWhole >= 60) {
      secondsWhole -= 60;
      minutes++;
      if (minutes >= 60) {
        minutes -= 60;
        degrees++;
      }
    }
  }
}
//-----------------------------------------------------------------------------
Airsoft::Classes::Print & operator << (Airsoft::Classes::Print & outs, const Dms & dms) {
  if (dms.degrees < 10) {
    outs.Write('0');
  }
  outs.print(dms.degrees);
  outs.Write(' ');
  if (dms.minutes < 10) {
    outs.Write('0');
  }
  outs.print(dms.minutes);
  outs.print("\' ");
  if (dms.secondsWhole < 10) {
    outs.Write('0');
  }
  outs.print(dms.secondsWhole);
  outs.Write('.');
  if (dms.secondsFrac < 100) {
    outs.Write('0');
  }
  if (dms.secondsFrac < 10) {
    outs.Write( '0');
  }
  outs.print( dms.secondsFrac );
  outs.print("\" ");

  return outs;
}
//-----------------------------------------------------------------------------
void Dms::PrintDDDMMmmmm(Airsoft::Classes::Print & outs ) const {
  outs.print( degrees );

  if (minutes < 10) {
    outs.print('0');
  }
  outs.print(minutes);
  outs.print('.');

  //  Calculate the fractional minutes from the seconds, without using floating-point numbers.

  uint16_t mmmm { static_cast<uint16_t>(secondsWhole * 166) };  // same as 10000/60, less .66666...
  mmmm += (secondsWhole * 2 + secondsFrac / 2) / 3;  // ... plus the remaining .66666

  // Print leading zeroes, if necessary
  if (mmmm < 1000) {
    outs.print('0');
  }
  if (mmmm <  100) {
    outs.print('0');
  }
  if (mmmm <   10){
    outs.print('0');
  }
  outs.print(mmmm);
}
//-----------------------------------------------------------------------------

} // namespace Airsoft::Neo {
