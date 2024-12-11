/**
 *******************************************************************************
 * @file streamers.Cpp
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

#include <neo/streamers.hpp>
#include <neo/gps-nema.hpp>
#include <neo/gps-fix.hpp>

namespace Airsoft::Neo {

//#define USE_FLOAT

Airsoft::Classes::Print& operator <<(Airsoft::Classes::Print &outs, const bool b) {
  outs.print( b ? 't' : 'f' );
  return outs;
}
//-----------------------------------------------------------------------------
Airsoft::Classes::Print& operator <<(Airsoft::Classes::Print &outs, const char c) {
  outs.print(c);
  return outs;
}
//-----------------------------------------------------------------------------
Airsoft::Classes::Print& operator <<(Airsoft::Classes::Print &outs, const uint16_t v) {
  outs.print(v); return outs;
}
//-----------------------------------------------------------------------------
Airsoft::Classes::Print& operator <<(Airsoft::Classes::Print &outs, const uint32_t v) {
  outs.print(v); return outs;
}
//-----------------------------------------------------------------------------
Airsoft::Classes::Print& operator <<(Airsoft::Classes::Print &outs, const int32_t v) {
  outs.print(v); return outs;
}
//-----------------------------------------------------------------------------
Airsoft::Classes::Print& operator <<(Airsoft::Classes::Print &outs, const uint8_t v) {
  outs.print(v);
  return outs;
}
//-----------------------------------------------------------------------------

const char gpsFixHeader[] =
  "Status,"

#if defined(GPS_FIX_DATE) | defined(GPS_FIX_TIME)

  "UTC "

#if defined(GPS_FIX_DATE)
  "Date"
#endif
#if defined(GPS_FIX_DATE) & defined(GPS_FIX_TIME)
  "/"
#endif
#if defined(GPS_FIX_TIME)
  "Time"
#endif

#else
  "s"
#endif

","

#ifdef GPS_FIX_LOCATION
  "Lat,Lon,"
#endif

#ifdef GPS_FIX_LOCATION_DMS
  "DMS,"
#endif

#if defined(GPS_FIX_HEADING)
  "Hdg,"
#endif

#if defined(GPS_FIX_SPEED)
  "Spd,"
#endif

#ifdef GPS_FIX_VELNED
  "Vel N,E,D,"
#endif

#if defined(GPS_FIX_ALTITUDE)
  "Alt,"
#endif

#if defined(GPS_FIX_HDOP)
  "HDOP,"
#endif

#if defined(GPS_FIX_VDOP)
  "VDOP,"
#endif

#if defined(GPS_FIX_PDOP)
  "PDOP,"
#endif

#if defined(GPS_FIX_LAT_ERR)
  "Lat err,"
#endif

#if defined(GPS_FIX_LON_ERR)
  "Lon err,"
#endif

#if defined(GPS_FIX_ALT_ERR)
  "Alt err,"
#endif

#if defined(GPS_FIX_SPD_ERR)
  "Spd err,"
#endif

#if defined(GPS_FIX_HDG_ERR)
  "Hdg err,"
#endif

#if defined(GPS_FIX_TIME_ERR)
  "Time err,"
#endif

#if defined(GPS_FIX_GEOID_HEIGHT)
  "Geoid Ht,"
#endif

#if defined(GPS_FIX_SATELLITES)
  "Sats,"
#endif
;

static const char GpsNmeaHeader[] =
#if defined(NMEAGPS_TIMESTAMP_FROM_INTERVAL) | defined(NMEAGPS_TIMESTAMP_FROM_PPS)
  "micros(),"
#endif

#if defined(NMEAGPS_PARSE_SATELLITES)
  "[sat"
#if defined(NMEAGPS_PARSE_SATELLITE_INFO)
    " elev/az @ SNR"
#endif
  "],"
#endif

#ifdef NMEAGPS_STATS
  "Rx ok,Rx err,Rx chars,"
#endif

  "";


//...............

#ifdef GPS_FIX_LOCATION_DMS
static void PrintDMS(Airsoft::Classes::Print & outs, const DMS_t & dms) {
  if (dms.degrees < 10) {
    outs.write('0');
  }
  outs.print(dms.degrees);
  outs.write(' ');

  if (dms.minutes < 10) {
    outs.write('0');
  }
  outs.print(dms.minutes);
  outs.print("\' ");

  if (dms.seconds_whole < 10) {
    outs.write('0');
  }
  outs.print( dms.seconds_whole );
  outs.write('.');

  if (dms.seconds_frac < 100) {
    outs.write('0');
  }
  if (dms.seconds_frac < 10) {
    outs.write('0');
  }
  outs.print( dms.seconds_frac );
  outs.print("\" ");

}
#endif
//...............

Airsoft::Classes::Print & operator <<(Airsoft::Classes::Print &outs, const GpsFix & fix) {
  if (fix.valid.Status)
    outs << (uint8_t) fix.status;
  outs << ',';

  #if defined(GPS_FIX_DATE) | defined(GPS_FIX_TIME)
    bool someTime = false;

    #if defined(GPS_FIX_DATE)
      someTime |= fix.valid.Date;
    #endif

    #if defined(GPS_FIX_TIME)
      someTime |= fix.valid.Time;
    #endif

    if (someTime) {
      outs << fix.dateTime << '.';
      uint16_t ms = fix.dateTime_ms();
      if (ms < 100)
        outs << '0';
      if (ms < 10)
        outs << '0';
      outs << ms;
    }
    outs << ',';

  #else

    //  Date/Time not enabled, just output the interval number
    static uint32_t sequence = 0L;
    outs << sequence++ << ',';

  #endif

  #ifdef USE_FLOAT
    #ifdef GPS_FIX_LOCATION
      if (fix.valid.location) {
        outs.print( fix.latitude(), 6 );
        outs << ',';
        outs.print( fix.longitude(), 6 );
      } else
        outs << ',';
      outs << ',';
    #endif
    #ifdef GPS_FIX_LOCATION_DMS
      if (fix.valid.location) {
        printDMS( outs, fix.latitudeDMS );
        outs.print( fix.latitudeDMS.NS() );
        outs.write( ' ' );
        if (fix.longitudeDMS.degrees < 100)
          outs.write( '0' );
        printDMS( outs, fix.longitudeDMS );
        outs.print( fix.longitudeDMS.EW() );
      }
      outs << ',';
    #endif
    #ifdef GPS_FIX_HEADING
      if (fix.valid.heading)
        outs.print( fix.heading(), 2 );
      outs << ',';
    #endif
    #ifdef GPS_FIX_SPEED
      if (fix.valid.speed)
        outs.print( fix.speed(), 3 ); // knots
      outs << ',';
    #endif
    #ifdef GPS_FIX_VELNED
      if (fix.valid.velned)
        outs.print( fix.velocity_north ); // cm/s
      outs << ',';
      if (fix.valid.velned)
        outs.print( fix.velocity_east  ); // cm/s
      outs << ',';
      if (fix.valid.velned)
        outs.print( fix.velocity_down  ); // cm/s
      outs << ',';
    #endif
    #ifdef GPS_FIX_ALTITUDE
      if (fix.valid.altitude)
        outs.print( fix.altitude(), 2 );
      outs << ',';
    #endif

    #ifdef GPS_FIX_HDOP
      if (fix.valid.hdop)
        outs.print( (fix.hdop * 0.001), 3 );
      outs << ',';
    #endif
    #ifdef GPS_FIX_VDOP
      if (fix.valid.vdop)
        outs.print( (fix.vdop * 0.001), 3 );
      outs << ',';
    #endif
    #ifdef GPS_FIX_PDOP
      if (fix.valid.pdop)
        outs.print( (fix.pdop * 0.001), 3 );
      outs << ',';
    #endif

    #ifdef GPS_FIX_LAT_ERR
      if (fix.valid.lat_err)
        outs.print( fix.lat_err(), 2 );
      outs << ',';
    #endif
    #ifdef GPS_FIX_LON_ERR
      if (fix.valid.lon_err)
        outs.print( fix.lon_err(), 2 );
      outs << ',';
    #endif
    #ifdef GPS_FIX_ALT_ERR
      if (fix.valid.alt_err)
        outs.print( fix.alt_err(), 2 );
      outs << ',';
    #endif
    #ifdef GPS_FIX_SPD_ERR
      if (fix.valid.spd_err)
        outs.print( fix.spd_err(), 2 );
      outs << ',';
    #endif
    #ifdef GPS_FIX_HDG_ERR
      if (fix.valid.hdg_err)
        outs.print( fix.hdg_err(), 2 );
      outs << ',';
    #endif
    #ifdef GPS_FIX_TIME_ERR
      if (fix.valid.time_err)
        outs.print( fix.time_err(), 2 );
      outs << ',';
    #endif

    #ifdef GPS_FIX_GEOID_HEIGHT
      if (fix.valid.geoidHeight)
        outs.print( fix.geoidHeight(), 2 );
      outs << ',';
    #endif

  #else

    // not USE_FLOAT ----------------------

    #ifdef GPS_FIX_LOCATION
      if (fix.valid.Location) {
        outs << fix.LatitudeL() << ',' << fix.LongitudeL();
      } else {
        outs << ',';
      }
      outs << ',';
    #endif
    #ifdef GPS_FIX_LOCATION_DMS
      if (fix.valid.location) {
        printDMS( outs, fix.latitudeDMS );
        outs.print( fix.latitudeDMS.NS() );
        outs.write( ' ' );
        if (fix.longitudeDMS.degrees < 100)
          outs.write( '0' );
        printDMS( outs, fix.longitudeDMS );
        outs.print( fix.longitudeDMS.EW() );
      }
      outs << ',';
    #endif
    #ifdef GPS_FIX_HEADING
      if (fix.valid.Heading)
        outs << fix.HeadingCd();
      outs << ',';
    #endif
    #ifdef GPS_FIX_SPEED
      if (fix.valid.Speed)
        outs << fix.SpeedMkn();
      outs << ',';
    #endif
    #ifdef GPS_FIX_VELNED
      if (fix.valid.velned)
        outs.print( fix.velocity_north ); // cm/s
      outs << ',';
      if (fix.valid.velned)
        outs.print( fix.velocity_east  ); // cm/s
      outs << ',';
      if (fix.valid.velned)
        outs.print( fix.velocity_down  ); // cm/s
      outs << ',';
    #endif
    #ifdef GPS_FIX_ALTITUDE
      if (fix.valid.Altitude)
        outs << fix.Altitude_cm();
      outs << ',';
    #endif

    #ifdef GPS_FIX_HDOP
      if (fix.valid.Hdop)
        outs << fix.hdop;
      outs << ',';
    #endif
    #ifdef GPS_FIX_VDOP
      if (fix.valid.Vdop)
        outs << fix.vdop;
      outs << ',';
    #endif
    #ifdef GPS_FIX_PDOP
      if (fix.valid.Pdop)
        outs << fix.pdop;
      outs << ',';
    #endif

    #ifdef GPS_FIX_LAT_ERR
      if (fix.valid.LatError)
        outs << fix.latErrCm;
      outs << ',';
    #endif
    #ifdef GPS_FIX_LON_ERR
      if (fix.valid.LonError)
        outs << fix.lonErrCm;
      outs << ',';
    #endif
    #ifdef GPS_FIX_ALT_ERR
      if (fix.valid.AltError)
        outs << fix.altErrCm;
      outs << ',';
    #endif
    #ifdef GPS_FIX_SPD_ERR
      if (fix.valid.SpdError)
        outs.print( fix.spdErrMmps);
      outs << ',';
    #endif
    #ifdef GPS_FIX_HDG_ERR
      if (fix.valid.HdgError)
        outs.print( fix.hdgErrE5);
      outs << ',';
    #endif
    #ifdef GPS_FIX_TIME_ERR
      if (fix.valid.TimeError)
        outs.print( fix.timeErrNs);
      outs << ',';
    #endif

    #ifdef GPS_FIX_GEOID_HEIGHT
      if (fix.valid.GeoidHeight)
        outs << fix.GeoidHeightCm();
      outs << ',';
    #endif
    
  #endif

  #ifdef GPS_FIX_SATELLITES
    if (fix.valid.Satellites)
      outs << fix.satellites;
    outs << ',';
  #endif

  return outs;
}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void TraceHeader(Airsoft::Classes::Print & outs ) {
  outs.print(&gpsFixHeader[0]);
  outs.print(&GpsNmeaHeader[0]);

  outs << '\n';
}
//-----------------------------------------------------------------------------
void TraceAll(Airsoft::Classes::Print & outs, const GpsNema & gps, const GpsFix & fix) {
  outs << fix;

#if defined(NMEAGPS_TIMESTAMP_FROM_INTERVAL) | defined(NMEAGPS_TIMESTAMP_FROM_PPS)
  outs << gps.UTCsecondStart();
  outs << ',';
#endif

#if defined(NMEAGPS_PARSE_SATELLITES)
  outs << '[';

  for (uint8_t i = 0; i < gps.SatCount; i++) {
    outs << gps.Satellites[i].id;

#if defined(NMEAGPS_PARSE_SATELLITE_INFO)
    outs << ' ' <<
      gps.Satellites[i].elevation << '/' << gps.Satellites[i].azimuth;
    outs << '@';
    if (gps.Satellites[i].tracked) {
      outs << gps.Satellites[i].snr;
    } else {
      outs << '-';
    }
#endif

    outs << ',';
  }

  outs << "],";
#endif

#ifdef NMEAGPS_STATS
  outs << gps.statistics.ok     << ',' << gps.statistics.errors << ',' << gps.statistics.chars  << ',';
#endif

  outs << '\n';

}

} // namespace Airsoft::Neo
