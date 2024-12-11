#ifndef GPSFIX_H
#define GPSFIX_H

#include <cstdint>
#include <cstdbool>
#include <cstdio>
#include <string>
#include <cstring>
#include <cmath>

#include <gps-config.hpp>

#if defined( GPS_FIX_DATE ) | defined( GPS_FIX_TIME )
#include <neo/neo-time.hpp>
#endif

#ifdef GPS_FIX_LOCATION_DMS
#include <neo/dms.hpp>
#endif

#ifdef GPS_FIX_LOCATION
#include <neo/location.hpp>
#endif

namespace Airsoft::Neo {

constexpr float KM_PER_NMI { 1.852 };
constexpr uint32_t M_PER_NMI { 1852 };
constexpr float MI_PER_NMI { 1.150779 };

/**
 * A structure for holding a GPS fix: time, position, velocity, etc.
 *
 * Because GPS devices report various subsets of a coherent fix,
 * this class tracks which members of the fix are being reported:
 * each part has its own validity flag. Also, operator |= implements
 * merging multiple reports into one consolidated report.
 *
 * @section Limitations
 * Reports are not really fused with an algorithm; if present in
 * the source, they are simply replaced in the destination.
 *
 */
class GpsFix {
public:

  // Default Constructor
  GpsFix() {
    Init();
  }

  //---------------------------------------------------------------------------
  // 'WholeFrac' is a utility structure that holds the two parts of a floating-point number.
  //
  // This is used for Altitude, Heading and Speed, which require more significant digits than a 16-bit number.
  //
  // When parsing floating-point fields, the decimal point is used as a separator for these two parts.
  // This is much more efficient than calling 'long' or 'floating-point' math subroutines.
  //
  // This requires knowing the exponent on the fraction when a simple type (e.g., float or int) is needed.
  // For example, 'Altitude()' knows that the whole part was stored as integer meters, and the fractional part
  // was stored as integer centimeters.
  //
  // Unless you want the speed and precision of the two integer parts, you shouldn't have to use 'WholeFrac'.
  // Instead, use the accessor functions for each of the specific fields for Altitude, Heading and Speed.

  struct WholeFrac {
    int16_t Whole {};
    int16_t Fraction {};

    int32_t Int32_00(void) const {
      return static_cast<int32_t>(Whole) * 100L + Fraction;
    }
    int16_t Int16_00(void) const {
      return Whole * 100 + Fraction;
    }
    int32_t Int32_000(void) const {
      return Whole * 1000L + Fraction;
    }
    float Float_00(void) const {
      return static_cast<float>(Whole) + static_cast<float>(Fraction) * 0.01;
    }
    float Float_000(void) const {
      return static_cast<float>(Whole) + static_cast<float>(Fraction)*0.001;
    }
  };

  //--------------------------------------------------------------
  // Members of a GPS fix
  // Each member is separately enabled or disabled by the CFG file by #ifdef/#endif wrappers.
  // Each member has a storage declaration that depends on the precision and type of data available from GPS devices.
  // Some members have a separate accessor function that converts the internal storage type to a more common or
  // convenient type for use by an application.
  // For example, latitude data is internally stored as a 32-bit integer, where the reported degrees have been
  // multiplied by 10^7.
  // Many applications expect a floating-point value, so a floating-point accessor is provided: 'Latitude()'.
  // This function converts the internal 32-bit integer to a floating-point value, and then divides it by 10^7.
  // The returned value is now a floating-point degrees value.

#ifdef GPS_FIX_LOCATION
  Location location;

  int32_t LatitudeL(void) const {
    return location.Lat ();
  }
  float Latitude(void) const {
    return location.LatF();
  }

  int32_t LongitudeL(void) const {
    return location.Lon ();
  }
  float Longitude(void) const {
    return location.LonF();
  }
#endif

#ifdef GPS_FIX_LOCATION_DMS
  DMS_t latitudeDMS;
  DMS_t longitudeDMS;
#endif

#ifdef GPS_FIX_ALTITUDE
  WholeFrac    alt; // .01 meters

  int32_t Altitude_cm(void) const {
    return alt.Int32_00();
  }
  float Altitude(void) const {
    return alt.Float_00();
  }
  float AltitudeFt(void) const {
    return Altitude() * 3.28084;
  }
#endif

#ifdef GPS_FIX_VELNED
  int32_t velocityNorth;   // cm/s
  int32_t velocityEast;    // cm/s
  int32_t velocityDown;    // cm/s

  void CalculateNorthAndEastVelocityFromSpeedAndHeading(void) {
#if defined(GPS_FIX_HEADING) && defined(GPS_FIX_SPEED)
      if (valid.heading && valid.speed && valid.velned) {
        float course         = heading() * NeoGPS::Location_t::RAD_PER_DEG;
        float speedCmPerS = speed_metersph() * (100.0 / 3600.0);
        velocityNorth = round(speedCmPerS * cos(course));
        velocityEast  = round(speedCmPerS * sin(course));
      }
#endif
  }
#endif

#ifdef GPS_FIX_SPEED
  WholeFrac    spd; // .001 nautical miles per hour

  uint32_t SpeedMkn(void) const {
    return spd.Int32_000();
  }
  float Speed(void) const {
    return spd.Float_000();
  }

  // Utilities for speed in other units
  float SpeedKph(void) const {
    return Speed() * KM_PER_NMI;
  }

  uint32_t SpeedMetersph(void) const {
    return (spd.Whole * M_PER_NMI) + (spd.Fraction * M_PER_NMI) / 1000;
  }

  float SpeedMph(void) const {
    return Speed() * MI_PER_NMI;
  }
#endif

#ifdef GPS_FIX_HEADING
  WholeFrac    hdg; //  .01 degrees

  uint16_t HeadingCd(void) const {
    return hdg.Int16_00();
  }
  float Heading(void) const {
    return hdg.Float_00();
  }
#endif

  //--------------------------------------------------------
  // Dilution of Precision is a measure of the current satellite
  // constellation geometry WRT how 'good' it is for determining a
  // position.  This is _independent_ of signal strength and many
  // other factors that may be internal to the receiver.
  // It _cannot_ be used to determine position accuracy in meters.
  // Instead, use the LAT/LON/ALT error in cm members, which are
  //   populated by GST sentences.

#ifdef GPS_FIX_HDOP
  uint16_t           hdop; // Horizontal Dilution of Precision x 1000
#endif
#ifdef GPS_FIX_VDOP
  uint16_t           vdop; // Vertical Dilution of Precision x 1000
#endif
#ifdef GPS_FIX_PDOP
  uint16_t           pdop; // Position Dilution of Precision x 1000
#endif

  //  Error estimates for lat, lon, altitude, speed, heading and time
#ifdef GPS_FIX_LAT_ERR
  uint16_t latErrCm;
  float latError(void) const {
    return latErrCm / 100.0;  // m
  }
#endif

#ifdef GPS_FIX_LON_ERR
  uint16_t lonErrCm;
  float lon_err(void) const {
    return lonErrCm / 100.0;    // m
  }
#endif

#ifdef GPS_FIX_ALT_ERR
  uint16_t altErrCm;
  float AltError(void) const {
    return altErrCm / 100.0;    // m
  }
#endif

#ifdef GPS_FIX_SPD_ERR
  uint16_t spdErrMmps;
  float SpdError(void) const {
    return spdErrMmps / 1000.0; // m/s
  }
#endif

#ifdef GPS_FIX_HDG_ERR
  uint16_t hdgErrE5;    // 0.00001 deg
  float HdgError(void) const {
    return hdgErrE5 / 1.0e5;  // deg
  }
#endif

#ifdef GPS_FIX_TIME_ERR
  uint16_t timeErrNs;
  float TimeError(void) const {
    return timeErrNs / 1.0e9;   // s
  }
#endif

  //--------------------------------------------------------
  // Height of the geoid above the WGS84 ellipsoid
#ifdef GPS_FIX_GEOID_HEIGHT
  WholeFrac    geoidHt; // .01 meters

  int32_t GeoidHeightCm(void) const {
    return geoidHt.Int32_00();
  }
  float GeoidHeight(void) const {
    return geoidHt.Float_00();
  }
#endif

  //--------------------------------------------------------
  // Number of satellites used to calculate a fix.
#ifdef GPS_FIX_SATELLITES
  uint8_t   satellites {};
#endif

  //--------------------------------------------------------
  //  Date and Time for the fix
#if defined(GPS_FIX_DATE) | defined(GPS_FIX_TIME)
  TimeT  dateTime; // Date and Time in one structure
#endif
#if defined(GPS_FIX_TIME)
  uint8_t         dateTimeCs;         // The fix's UTC hundred of a second
  uint32_t dateTime_us(void) const {  // The fix's UTC microseconds
    return dateTimeCs * 10000UL;
  }
  uint16_t dateTime_ms(void) const {  // The fix's UTC millseconds
    return dateTimeCs * 10;
  }
#endif

  //--------------------------------------------------------
  // The current fix status or mode of the GPS device.
  //
  // Unfortunately, the NMEA sentences are a little inconsistent
  //   in their use of "status" and "mode". Both fields are mapped
  //   onto this enumerated type.  Be aware that different
  //   manufacturers interpret them differently.  This can cause
  //   problems in sentences which include both types (e.g., GPGLL).
  //
  // Note: Sorted by increasing accuracy.  See also /operator |=/.

  enum class Status : uint8_t {
    STATUS_NONE,
    STATUS_EST,
    STATUS_TIME_ONLY,
    STATUS_STD,
    STATUS_DGPS,
    STATUS_RTK_FLOAT,
    STATUS_RTK_FIXED,
    STATUS_PPS // Precise Position System, *NOT* Pulse-per-second
  };

  Status  status { 8 };

  //--------------------------------------------------------
  //  Flags to indicate which members of this fix are valid.
  //
  //  Because different sentences contain different pieces of a fix,
  //    each piece can be valid or NOT valid.  If the GPS device does not
  //    have good reception, some fields may not contain any value.
  //    Those empty fields will be marked as NOT valid.

  struct ValidT {
    uint8_t Status { 1 };

#if defined(GPS_FIX_DATE)
    uint8_t Date { 1 };
#endif

#if defined(GPS_FIX_TIME)
    uint8_t Time { 1 };
#endif

#if defined( GPS_FIX_LOCATION ) | defined( GPS_FIX_LOCATION_DMS )
    uint8_t Location { 1 };
#endif

#ifdef GPS_FIX_ALTITUDE
    uint8_t Altitude { 1 };
#endif

#ifdef GPS_FIX_SPEED
    uint8_t Speed { 1 };
#endif

#ifdef GPS_FIX_VELNED
    uint8_t Velned { 1 };
#endif

#ifdef GPS_FIX_HEADING
    uint8_t Heading { 1 };
#endif

#ifdef GPS_FIX_SATELLITES
    uint8_t Satellites { 1 };
#endif

#ifdef GPS_FIX_HDOP
    uint8_t Hdop { 1 };
#endif

#ifdef GPS_FIX_VDOP
    uint8_t Vdop { 1 };
#endif

#ifdef GPS_FIX_PDOP
    uint8_t Pdop { 1 };
#endif

#ifdef GPS_FIX_LAT_ERR
    uint16_t LatError { 1 };
#endif

#ifdef GPS_FIX_LON_ERR
    uint16_t LonError { 1 };
#endif

#ifdef GPS_FIX_ALT_ERR
    uint16_t AltError { 1 };
#endif

#ifdef GPS_FIX_SPD_ERR
    uint8_t SpdError { 1 };
#endif

#ifdef GPS_FIX_HDG_ERR
    uint8_t HdgError { 1 };
#endif

#ifdef GPS_FIX_TIME_ERR
    uint8_t TimeError { 1 };
#endif

#ifdef GPS_FIX_GEOID_HEIGHT
    uint8_t GeoidHeight { 1 };
#endif

    // Initialize all flags to false
    void Init(void) {
      uint8_t * all = reinterpret_cast<uint8_t*>(this);
      for (uint8_t i = 0; i < sizeof(*this); i++) {
        *all++ = 0;
      }
    }

    // Merge these valid flags with another set of valid flags
    void operator |= (const ValidT & r) {
      uint8_t *all = reinterpret_cast<uint8_t*>(this);
      const uint8_t * r_all = reinterpret_cast<const uint8_t*>(&r);

      for (uint8_t i = 0; i<sizeof(*this); i++) {
        *all++ |= *r_all++;
      }
    }

  } valid;  // This is the name of the collection of valid flags

  //--------------------------------------------------------
  //  Initialize a fix.  All configured members are set to zero.

  void Init(void) {

#ifdef GPS_FIX_LOCATION_DMS
    latitudeDMS.init();
    longitudeDMS.init();
#endif

  #ifdef GPS_FIX_VELNED
    velocityNorth =
    velocityEast  =
    velocityDown  = 0;
  #endif

  #ifdef GPS_FIX_HDOP
    hdop = 0;
  #endif
  #ifdef GPS_FIX_VDOP
    vdop = 0;
  #endif
  #ifdef GPS_FIX_PDOP
    pdop = 0;
  #endif

  #ifdef GPS_FIX_LAT_ERR
    latErrCm = 0;
  #endif
  #ifdef GPS_FIX_LON_ERR
    lonErrCm = 0;
  #endif
  #ifdef GPS_FIX_ALT_ERR
    altErrCm = 0;
  #endif
  #ifdef GPS_FIX_SPD_ERR
    spdErrMmps = 0;
  #endif
  #ifdef GPS_FIX_HDG_ERR
    hdgErrE5 = 0;
  #endif
#ifdef GPS_FIX_TIME_ERR
    timeErrNs = 0;
#endif

#ifdef GPS_FIX_SATELLITES
    satellites = 0;
#endif

#if defined(GPS_FIX_DATE) | defined(GPS_FIX_TIME)
    dateTime.Init();
#endif
#if defined(GPS_FIX_TIME)
    dateTimeCs = 0;
#endif

    status = Status::STATUS_NONE;
  }

  //-------------------------------------------------------------
  // Merge valid fields from the right fix into a "fused" fix
  //   on the left (i.e., /this/).
  //
  // Usage:  gps_fix left, right;
  //         left |= right;  // explicit merge

  GpsFix & operator |=(const GpsFix & r) {
    // Replace /status/  only if the right is more "accurate".
    if (r.valid.Status && (!valid.Status || (status < r.status))) {
      status = r.status;
    }

#ifdef GPS_FIX_DATE
    if (r.valid.Date) {
      dateTime.Date  = r.dateTime.Date;
      dateTime.Month = r.dateTime.Month;
      dateTime.Year  = r.dateTime.Year;
    }
#endif

#ifdef GPS_FIX_TIME
    if (r.valid.Time) {
      dateTime.Hours   = r.dateTime.Hours;
      dateTime.Minutes = r.dateTime.Minutes;
      dateTime.Seconds = r.dateTime.Seconds;
      dateTimeCs       = r.dateTimeCs;
    }
#endif

#ifdef GPS_FIX_LOCATION
    if (r.valid.Location) {
      location = r.location;
    }
#endif

#ifdef GPS_FIX_LOCATION_DMS
    if (r.valid.location) {
      latitudeDMS  = r.latitudeDMS;
      longitudeDMS = r.longitudeDMS;
    }
#endif

#ifdef GPS_FIX_ALTITUDE
    if (r.valid.Altitude) {
      alt = r.alt;
    }
#endif

#ifdef GPS_FIX_HEADING
    if (r.valid.Heading) {
      hdg = r.hdg;
    }
#endif

#ifdef GPS_FIX_SPEED
    if (r.valid.Speed) {
      spd = r.spd;
    }
#endif

#ifdef GPS_FIX_VELNED
    if (r.valid.Velned) {
      velocityNorth = r.velocityNorth;
      velocityEast  = r.velocityEast;
      velocityDown  = r.velocityDown;
    }
#endif

#ifdef GPS_FIX_SATELLITES
    if (r.valid.Satellites) {
      satellites = r.satellites;
    }
#endif

#ifdef GPS_FIX_HDOP
    if (r.valid.Hdop) {
      hdop = r.hdop;
    }
#endif

#ifdef GPS_FIX_VDOP
    if (r.valid.Vdop) {
      vdop = r.vdop;
    }
#endif

#ifdef GPS_FIX_PDOP
    if (r.valid.Pdop) {
      pdop = r.pdop;
    }
#endif

#ifdef GPS_FIX_LAT_ERR
    if (r.valid.LatError) {
      latErrCm = r.latErrCm;
    }
#endif

#ifdef GPS_FIX_LON_ERR
    if (r.valid.LonError) {
      lonErrCm = r.lonErrCm;
    }
#endif

#ifdef GPS_FIX_ALT_ERR
    if (r.valid.AltError) {
      altErrCm = r.altErrCm;
    }
#endif

#ifdef GPS_FIX_SPD_ERR
    if (r.valid.SpdError) {
      spdErrMmps = r.spdErrMmps;
    }
#endif

#ifdef GPS_FIX_HDG_ERR
    if (r.valid.HdgError) {
      hdgErrE5 = r.hdgErrE5;
    }
#endif

#ifdef GPS_FIX_TIME_ERR
    if (r.valid.TimeError) {
      timeErrNs = r.timeErrNs;
    }
#endif

#ifdef GPS_FIX_GEOID_HEIGHT
    if (r.valid.GeoidHeight) {
      geoidHt = r.geoidHt;
    }
#endif

    // Update all the valid flags
    valid |= r.valid;

    return *this;

  }
};

} // namespace Airsoft::Neo {

#endif
