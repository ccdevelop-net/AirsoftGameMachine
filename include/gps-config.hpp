/**
 *******************************************************************************
 * @file gps-config.hpp
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
#ifndef GPS_CONFIG_HPP_
#define GPS_CONFIG_HPP_

//------------------------------------------------------------------------
//  Accommodate C++ compiler and IDE changes.
//
//  Declaring constants as class data instead of instance data helps avoid
//  collisions with #define names, and allows the compiler to perform more
//  checks on their usage.
//
//  Until C++ 10 and IDE 1.6.8, initialized class data constants
//  were declared like this:
//
//      static const <valued types> = <constant-value>;
//
//  Now, non-simple types (e.g., float) must be declared as
//
//      static constexpr <nonsimple-types> = <expression-treated-as-const>;
//
//  The good news is that this allows the compiler to optimize out an
//  expression that is "promised" to be "evaluatable" as a constant.
//  The bad news is that it introduces a new language keyword, and the old
//  code raises an error.
//
//  TODO: Evaluate the requirement for the "static" keyword.
//  TODO: Evaluate using a C++ version preprocessor symbol for the #if.
//          #if __cplusplus >= 201103L  (from XBee.h)
//
//  The CONST_CLASS_DATA define will expand to the appropriate keywords.
//



#define CONST_CLASS_DATA static const



/*-------------------------------------------------------------------
 * Enable/disable the storage for the members of a fix.
 *
 * Disabling a member prevents it from being parsed from a received message.
 * The disabled member cannot be accessed or stored, and its validity flag
 * would not be available.  It will not be declared, and code that uses that
 * member will not compile.
 *
 * DATE and TIME are somewhat coupled in that they share a single `time_t`,
 * but they have separate validity flags.
 *
 * See also note regarding the DOP members, below.
 */

#define GPS_FIX_DATE
#define GPS_FIX_TIME
#define GPS_FIX_LOCATION
//#define GPS_FIX_LOCATION_DMS
#define GPS_FIX_ALTITUDE
#define GPS_FIX_SPEED
//#define GPS_FIX_VELNED
#define GPS_FIX_HEADING
#define GPS_FIX_SATELLITES
#define GPS_FIX_HDOP
#define GPS_FIX_VDOP
#define GPS_FIX_PDOP
#define GPS_FIX_LAT_ERR
#define GPS_FIX_LON_ERR
#define GPS_FIX_ALT_ERR
#define GPS_FIX_SPD_ERR
#define GPS_FIX_HDG_ERR
#define GPS_FIX_TIME_ERR
#define GPS_FIX_GEOID_HEIGHT

/*
 * -------------------------------------------------------------------
 */

//------------------------------------------------------
// Enable/disable the parsing of specific sentences.
//
// Configuring out a sentence prevents it from being recognized; it
// will be completely ignored.  (See also NMEAGPS_RECOGNIZE_ALL, below)
//
// FYI: Only RMC and ZDA contain date information.  Other
// sentences contain time information.  Both date and time are
// required if you will be doing time_t-to-clock_t operations.

#define NMEAGPS_PARSE_GGA
#define NMEAGPS_PARSE_GLL
#define NMEAGPS_PARSE_GSA
#define NMEAGPS_PARSE_GSV
#define NMEAGPS_PARSE_GST
#define NMEAGPS_PARSE_RMC
#define NMEAGPS_PARSE_VTG
#define NMEAGPS_PARSE_ZDA

//------------------------------------------------------
// Select which sentence is sent *last* by your GPS device
// in each update interval.  This can be used by your sketch
// to determine when the GPS quiet time begins, and thus
// when you can perform "some" time-consuming operations.

#define LAST_SENTENCE_IN_INTERVAL Airsoft::Neo::NmeaMessages::NMEA_RMC

// NOTE: For PUBX-only, PGRM and UBX configs, use
//          (NMEAGPS::nmea_msg_t)(NMEAGPS::NMEA_LAST_MSG+1)
//       Otherwise, use one of the standard NMEA messages:
//          NMEAGPS::NMEA_RMC
//
//    ==>  CONFIRM THIS WITH NMEAorder.INO  <==
//
// If the NMEA_LAST_SENTENCE_IN_INTERVAL is not chosen
// correctly, GPS data may be lost because the sketch
// takes too long elsewhere when this sentence is received.
// Also, fix members may contain information from different
// time intervals (i.e., they are not coherent).
//
// If you don't know which sentence is the last one,
// use NMEAorder.ino to list them.  You do not have to select
// the last sentence the device sends if you have disabled
// it.  Just select the last sentence that you have *enabled*.

//------------------------------------------------------
// Choose how multiple sentences are merged into a fix:
//   1) No merging
//        Each sentence fills out its own fix; there could be
//        multiple sentences per interval.
//   2) EXPLICIT_MERGING
//        All sentences in an interval are *safely* merged into one fix.
//        NMEAGPS_FIX_MAX must be >= 1.
//        An interval is defined by NMEA_LAST_SENTENCE_IN_INTERVAL.
//   3) IMPLICIT_MERGING
//        All sentences in an interval are merged into one fix, with
//        possible data loss.  If a received sentence is rejected for
//        any reason (e.g., a checksum error), all the values are suspect.
//        The fix will be cleared; no members will be valid until new
//        sentences are received and accepted.  This uses less RAM.
//        An interval is defined by NMEA_LAST_SENTENCE_IN_INTERVAL.
// Uncomment zero or one:

#define NMEAGPS_EXPLICIT_MERGING
//#define NMEAGPS_IMPLICIT_MERGING

#ifdef NMEAGPS_IMPLICIT_MERGING
#define NMEAGPS_MERGING NMEAGPS::IMPLICIT_MERGING

// Nothing is done to the fix at the beginning of every sentence...
#define NMEAGPS_INIT_FIX(m)

// ...but we invalidate one part when it starts to get parsed.  It *may* get
// validated when the parsing is finished.
#define NMEAGPS_INVALIDATE(m) m_fix.valid.m = false
#else
#ifdef NMEAGPS_EXPLICIT_MERGING
#define NMEAGPS_MERGING Airsoft::Neo::MergingValue::EXPLICIT_MERGING
#else
#define NMEAGPS_MERGING Airsoft::Neo::MergingValue::NO_MERGING
#define NMEAGPS_NO_MERGING
#endif  // NMEAGPS_EXPLICIT_MERGING

// When NOT accumulating (not IMPLICIT), invalidate the entire fix
// at the beginning of every sentence...
#define NMEAGPS_INIT_FIX(m) m.valid.init()

// ...so the individual parts do not need to be invalidated as they are parsed
#define NMEAGPS_INVALIDATE(m)
#endif  // NMEAGPS_IMPLICIT_MERGING

#if (defined(NMEAGPS_NO_MERGING) + defined(NMEAGPS_IMPLICIT_MERGING) + defined(NMEAGPS_EXPLICIT_MERGING) )  > 1
#error Only one MERGING technique should be enabled in NMEAGPS_cfg.h!
#endif

//------------------------------------------------------
// Define the fix buffer size.  The NMEAGPS object will hold on to
// this many fixes before an overrun occurs.  This can be zero,
// but you have to be more careful about using gps.fix() structure,
// because it will be modified as characters are received.

#define NMEAGPS_FIX_MAX 1

#if defined(NMEAGPS_EXPLICIT_MERGING) && (NMEAGPS_FIX_MAX == 0)
#error You must define FIX_MAX >= 1 to allow EXPLICIT merging in NMEAGPS_cfg.h
#endif

//------------------------------------------------------
// Define how fixes are dropped when the FIFO is full.
//   true  = the oldest fix will be dropped, and the new fix will be saved.
//   false = the new fix will be dropped, and all old fixes will be saved.

#define NMEAGPS_KEEP_NEWEST_FIXES true

//------------------------------------------------------
// Enable/disable the talker ID, manufacturer ID and proprietary message processing.
//
// First, some background information.  There are two kinds of NMEA sentences:
//
// 1. Standard NMEA sentences begin with "$ttccc", where
//      "tt" is the talker ID, and
//      "ccc" is the variable-length sentence type (i.e., command).
//
//    For example, "$GPGLL,..." is a GLL sentence (Geographic Lat/Long)
//    transmitted by talker "GP".  This is the most common talker ID.  Some
//    devices may report "$GNGLL,..." when a mix of GPS and non-GPS
//    satellites have been used to determine the GLL data.
//
// 2. Proprietary NMEA sentences (i.e., those unique to a particular
//    manufacturer) begin with "$Pmmmccc", where
//      "P" is the NMEA-defined prefix indicator for proprietary messages,
//      "mmm" is the 3-character manufacturer ID, and
//      "ccc" is the variable-length sentence type (it can be empty).
//
// No validation of manufacturer ID and talker ID is performed in this
// base class.  For example, although "GP" is a common talker ID, it is not
// guaranteed to be transmitted by your particular device, and it IS NOT
// REQUIRED.  If you need validation of these IDs, or you need to use the
// extra information provided by some devices, you have two independent
// options:
//
// 1. Enable SAVING the ID: When /decode/ returns DECODE_COMPLETED, the
// /talker_id/ and/or /mfr_id/ members will contain ID bytes.  The entire
// sentence will be parsed, perhaps modifying members of /fix/.  You should
// enable one or both IDs if you want the information in all sentences *and*
// you also want to know the ID bytes.  This adds two bytes of RAM for the
// talker ID, and 3 bytes of RAM for the manufacturer ID.
//
// 2. Enable PARSING the ID:  The virtual /parse_talker_id/ and
// /parse_mfr_id/ will receive each ID character as it is parsed.  If it
// is not a valid ID, return /false/ to abort processing the rest of the
// sentence.  No CPU time will be wasted on the invalid sentence, and no
// /fix/ members will be modified.  You should enable this if you want to
// ignore some IDs.  You must override /parse_talker_id/ and/or
// /parse_mfr_id/ in a derived class.
//

//#define NMEAGPS_SAVE_TALKER_ID
//#define NMEAGPS_PARSE_TALKER_ID

//#define NMEAGPS_PARSE_PROPRIETARY
#ifdef NMEAGPS_PARSE_PROPRIETARY
//#define NMEAGPS_SAVE_MFR_ID
#define NMEAGPS_PARSE_MFR_ID
#endif  // NMEAGPS_PARSE_PROPRIETARY

//------------------------------------------------------
// Enable/disable tracking the current satellite array and,
// optionally, all the info for each satellite.
//

#define NMEAGPS_PARSE_SATELLITES
#define NMEAGPS_PARSE_SATELLITE_INFO

#ifdef NMEAGPS_PARSE_SATELLITES
#define NMEAGPS_MAX_SATELLITES (20)

#ifndef GPS_FIX_SATELLITES
#error GPS_FIX_SATELLITES must be defined in GPSfix.h!
#endif  // GPS_FIX_SATELLITES
#endif  // NMEAGPS_PARSE_SATELLITES

#if defined(NMEAGPS_PARSE_SATELLITE_INFO) & !defined(NMEAGPS_PARSE_SATELLITES)
#error NMEAGPS_PARSE_SATELLITES must be defined!
#endif

//------------------------------------------------------
// Enable/disable gathering interface statistics:
// CRC errors and number of sentences received

#define NMEAGPS_STATS

//------------------------------------------------------
// Configuration item for allowing derived types of NMEAGPS.
// If you derive classes from NMEAGPS, you *must* define NMEAGPS_DERIVED_TYPES.
// If not defined, virtuals are not used, with a slight size (2 bytes) and
// execution time savings.

//#define NMEAGPS_DERIVED_TYPES

#ifdef NMEAGPS_DERIVED_TYPES
#define NMEAGPS_VIRTUAL virtual
#else
#define NMEAGPS_VIRTUAL
#endif

//-----------------------------------
// See if DERIVED_TYPES is required
#if (defined(NMEAGPS_PARSE_TALKER_ID) | defined(NMEAGPS_PARSE_MFR_ID)) & !defined(NMEAGPS_DERIVED_TYPES)
#error You must define NMEAGPS_DERIVED_TYPES in NMEAGPS.h in order to parse Talker and/or Mfr IDs!
#endif

//------------------------------------------------------
//  Becase the NMEA checksum is not very good at error detection, you can
//    choose to enable additional validity checks.  This trades a little more
//    code and execution time for more reliability.
//
//  Validation at the character level is a syntactic check only.  For
//    example, integer fields must contain characters in the range 0..9,
//    latitude hemisphere letters can be 'N' or 'S'.  Characters that are not
//    valid for a particular field will cause the entire sentence to be
//    rejected as an error, *regardless* of whether the checksum would pass.
#define NMEAGPS_VALIDATE_CHARS false

//  Validation at the field level is a semantic check.  For
//    example, latitude degrees must be in the range -90..+90.
//    Values that are not valid for a particular field will cause the
//    entire sentence to be rejected as an error, *regardless* of whether the
//    checksum would pass.
#define NMEAGPS_VALIDATE_FIELDS false

//------------------------------------------------------
// Some devices may omit trailing commas at the end of some
// sentences.  This may prevent the last field from being
// parsed correctly, because the parser for some types keep
// the value in an intermediate state until the complete
// field is received (e.g., parseDDDMM, parseFloat and
// parseZDA).
//
// Enabling this will inject a simulated comma when the end
// of a sentence is received and the last field parser
// indicated that it still needs one.

#define NMEAGPS_COMMA_NEEDED

//------------------------------------------------------
//  Some applications may want to recognize a sentence type
//  without actually parsing any of the fields.  Uncommenting
//  this define will allow the nmeaMessage member to be set
//  when *any* standard message is seen, even though that
//  message is not enabled by a NMEAGPS_PARSE_xxx define above.
//  No valid flags will be true for those sentences.

#define NMEAGPS_RECOGNIZE_ALL

//------------------------------------------------------
// Sometimes, a little extra space is needed to parse an intermediate form.
// This config items enables extra space.

//#define NMEAGPS_PARSING_SCRATCHPAD

//------------------------------------------------------
// If you need to know the exact UTC time at *any* time,
//   not just after a fix arrives, you must calculate the
//   offset between the Arduino micros() clock and the UTC
//   time in a received fix.  There are two ways to do this:
//
// 1) When the GPS quiet time ends and the new update interval begins.
//    The timestamp will be set when the first character (the '$') of
//    the new batch of sentences arrives from the GPS device.  This is fairly
//    accurate, but it will be delayed from the PPS edge by the GPS device's
//    fix calculation time (usually ~100us).  There is very little variance
//    in this calculation time (usually < 30us), so all timestamps are
//    delayed by a nearly-constant amount.
//
//    NOTE:  At update rates higher than 1Hz, the updates may arrive with
//    some increasing variance.

//#define NMEAGPS_TIMESTAMP_FROM_INTERVAL

// 2) From the PPS pin of the GPS module.  It is up to the application
//    developer to decide how to capture that event.  For example, you could:
//
//    a) simply poll for it in loop and call UTCsecondStart(micros());
//    b) use attachInterrupt to call a Pin Change Interrupt ISR to save
//       the micros() at the time of the interrupt (see NMEAGPS.h), or
//    c) connect the PPS to an Input Capture pin.  Set the
//       associated TIMER frequency, calculate the elapsed time
//       since the PPS edge, and add that to the current micros().

//#define NMEAGPS_TIMESTAMP_FROM_PPS

#if defined( NMEAGPS_TIMESTAMP_FROM_INTERVAL ) & defined( NMEAGPS_TIMESTAMP_FROM_PPS )
#error You cannot enable both TIMESTAMP_FROM_INTERVAL and PPS in NMEAGPS_cfg.h!
#endif

/*-------------------------------------------------------------------
 * Enable/disable the parsing of specific Garmin NMEA sentences.
 *
 * Configuring out a sentence prevents its fields from being parsed.
 * However, the sentence type may still be recognized by /decode/ and
 * stored in member /nmeaMessage/.  No valid flags would be available.
 */

//#define GARMINGPS_PARSE_F

/*
 * -------------------------------------------------------------------
 */

/*-------------------------------------------------------------------
 * Enable/disable the parsing of specific UBLOX NMEA sentences.
 *
 * Configuring out a sentence prevents its fields from being parsed.
 * However, the sentence type may still be recognized by /decode/ and
 * stored in member /nmeaMessage/.  No valid flags would be available.
 */

#define NMEAGPS_PARSE_PUBX_00
//#define NMEAGPS_PARSE_PUBX_04

/*
 * -------------------------------------------------------------------
 */

/*-------------------------------------------------------------------
 * Enable/disable the parsing of specific UBX messages.
 *
 * Configuring out a message prevents its fields from being parsed.
 * However, the message type will still be recognized by /decode/ and
 * stored in member /rx_msg/.  No valid flags would be available.
 */

#define UBLOX_PARSE_STATUS
#define UBLOX_PARSE_TIMEGPS
#define UBLOX_PARSE_TIMEUTC
#define UBLOX_PARSE_POSLLH
//#define UBLOX_PARSE_DOP
//#define UBLOX_PARSE_PVT
#define UBLOX_PARSE_VELNED
//#define UBLOX_PARSE_SVINFO
//#define UBLOX_PARSE_CFGNAV5
//#define UBLOX_PARSE_MONVER
//#define UBLOX_PARSE_HNR_PVT

#if defined(UBLOX_PARSE_DOP) & ( !defined(GPS_FIX_HDOP) & !defined(GPS_FIX_VDOP) & !defined(GPS_FIX_PDOP) )
#warning UBX DOP message is enabled, but all gps_fix DOP members are disabled.
#endif

//--------------------------------------------------------------------
// Identify the last UBX message in an update interval.
//    (There are two parts to a UBX message, the class and the ID.)
// For coherency, you must determine which UBX message is last!
// This section *should* pick the correct last UBX message.

#if defined(UBLOX_PARSE_HNR_PVT)
#define UBX_LAST_MSG_CLASS_IN_INTERVAL ublox::UBX_HNR
#define UBX_LAST_MSG_ID_IN_INTERVAL    ublox::UBX_HNR_PVT
#else
#define UBX_LAST_MSG_

/*
 * -------------------------------------------------------------------
 */

// TODO: To fix
//CLASS_IN_INTERVAL ublox::UBX_NAV

#if defined(UBLOX_PARSE_VELNED)
#define UBX_LAST_MSG_ID_IN_INTERVAL    ublox::UBX_NAV_VELNED
#elif defined(UBLOX_PARSE_DOP)
#define UBX_LAST_MSG_ID_IN_INTERVAL    ublox::UBX_NAV_DOP
#elif defined(UBLOX_PARSE_POSLLH)
#define UBX_LAST_MSG_ID_IN_INTERVAL    ublox::UBX_NAV_POSLLH
#elif defined(UBLOX_PARSE_STATUS)
#define UBX_LAST_MSG_ID_IN_INTERVAL    ublox::UBX_NAV_STATUS
#elif defined(UBLOX_PARSE_PVT)
#define UBX_LAST_MSG_ID_IN_INTERVAL    ublox::UBX_NAV_PVT
#elif defined(UBLOX_PARSE_SVINFO)
#define UBX_LAST_MSG_ID_IN_INTERVAL    ublox::UBX_NAV_SVINFO
#endif
#endif

/*
 * -------------------------------------------------------------------
 */


#endif // GPS_CONFIG_HPP_
