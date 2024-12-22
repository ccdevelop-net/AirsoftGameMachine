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

#include <neo/gps-nema.hpp>

namespace Airsoft::Neo {

// Check configurations
#if defined( GPS_FIX_LOCATION_DMS ) & !defined( NMEAGPS_PARSING_SCRATCHPAD )
// The fractional part of the NMEA minutes can have 5 significant figures.
// This requires more temporary storage than is available in the DMS_t.
#error You must enable NMEAGPS_PARSING_SCRATCHPAD in NMEAGPS_cfg.h when GPS_FIX_LOCATION_DMS is enabled in GPSfix_cfg.h!
#endif

#ifndef CR
#define CR ((char)13)
#endif
#ifndef LF
#define LF ((char)10)
#endif

//----------------------------------------------------------------
inline static uint8_t ParseHEX (char a) {
  a |= 0x20; // make it lowercase

  if (('a' <= a) && (a <= 'f')) {
    return a - 'a' + 10;
  } else {
    return a - '0';
  }
}
//----------------------------------------------------------------
static char FormatHex (uint8_t val) {
  val &= 0x0F;

  return (val >= 10) ? ((val - 10) + 'A') : (val + '0');
}
//----------------------------------------------------------------
inline uint8_t ToBinary (uint8_t value) {
  uint8_t high { static_cast<uint8_t> (value >> 4) };
  uint8_t low { static_cast<uint8_t> (value & 0x0F) };
  return ((high << 3) + (high << 1) + low);
}

static uint32_t divu3 (uint32_t n) {
  return n / 3;
}


#ifdef GPS_FIX_LOCATION_DMS
static void FinalizeDMS(uint32_t min_frac, Dms & dms) {
  min_frac *= 6UL;
  uint32_t secs       = min_frac / 10000UL;
  uint16_t frac_x1000 = (min_frac - (secs * 10000UL) + 5) / 10;

  // Rounding up can yield a frac of 1000/1000ths. Carry into whole.
  if (frac_x1000 >= 1000) {
    frac_x1000 -= 1000;
    secs       += 1;
  }
  dms.seconds_whole  = secs;
  dms.seconds_frac   = frac_x1000;
}
#endif

//----------------------------------------------------------------

GpsNema::GpsNema () {
  DataInit ();
  Reset ();
}

//----------------------------------------------------------------
// Prepare internal members to receive data from sentence fields.

void GpsNema::SentenceBegin (void) {
  if (IntervalComplete ()) {
    // GPS quiet time is over, this is the start of a new interval.

#if defined(NMEAGPS_TIMESTAMP_FROM_INTERVAL) & ( defined(GPS_FIX_DATE) | defined(GPS_FIX_TIME) )
    _IntervalStart = micros();
#endif

    IntervalComplete (false);

#ifdef NMEAGPS_PARSE_SATELLITES
    SatCount = 0;
#endif
  }

  _crc = 0;
  nmeaMessage = NmeaMessages::NMEA_UNKNOWN;
  RxState = RxStateValues::NMEA_RECEIVING_HEADER;
  _chrCount = 0;
  CommaNeeded (false);

#ifdef NMEAGPS_PARSE_PROPRIETARY
  proprietary = false;

#ifdef NMEAGPS_SAVE_MFR_ID
    mfr_id[0] =
    mfr_id[1] =
    mfr_id[2] = 0;
#endif
#endif

#ifdef NMEAGPS_SAVE_TALKER_ID
    talker_id[0] =
    talker_id[1] = 0;
#endif

}

//----------------------------------------------------------------
void GpsNema::SentenceOk (void) {
  // Terminate the last field with a comma if the parser needs it.
  if (CommaNeeded()) {
    CommaNeeded (false);
    _chrCount++;
    ParseField (',');
  }

#ifdef NMEAGPS_STATS
  statistics.ok++;
#endif

  //  This implements coherency.
  IntervalComplete (IntervalCompleted());

  if(IntervalComplete()) {
    // GPS quiet time now
  }

  Reset();
}

//----------------------------------------------------------------
// There was something wrong with the sentence.

void GpsNema::SentenceInvalid () {
  // All the values are suspect.  Start over.
  _fix.valid.Init();
  nmeaMessage = NmeaMessages::NMEA_UNKNOWN;

  Reset();
}

//----------------------------------------------------------------
//  The sentence is well-formed, but is an unrecognized type

void GpsNema::SentenceUnrecognized () {
  nmeaMessage = NmeaMessages::NMEA_UNKNOWN;

  Reset();
}

//----------------------------------------------------------------

void GpsNema::HeaderReceived(void) {
  _fix.valid.Init();
  _fieldIndex = 1;
  _chrCount = 0;
  RxState = RxStateValues::NMEA_RECEIVING_DATA;
}

//----------------------------------------------------------------
// Process one character of an NMEA GPS sentence.

DecodeValues GpsNema::Decode (char c) {
#ifdef NMEAGPS_STATS
  statistics.chars++;
#endif

  DecodeValues res = DecodeValues::CHR_OK;

  if (c == '$') {  // Always restarts
    SentenceBegin ();
  } else if (RxState == RxStateValues::NMEA_RECEIVING_DATA) { //---------------------------
    // Receive complete sentence

    if (c == '*') {                // Line finished, CRC follows
      RxState = RxStateValues::NMEA_RECEIVING_CRC;
      _chrCount = 0;

    } else if ((' ' <= c) && (c <= '~')) { // Normal data character

    _crc ^= c;  // accumulate CRC as the chars come in...

    if (!ParseField (c)) {
      SentenceInvalid();
    } else if (c == ',') {
      // Start the next field
      CommaNeeded(false);
      _fieldIndex++;
      _chrCount = 0;
    } else
      _chrCount++;

    // This is an undocumented option.  It could be useful
    // for testing, but all real devices will output a CS.
#ifdef NMEAGPS_CS_OPTIONAL
    } else if ((c == CR) || (c == LF)) { // Line finished, no CRC
      sentenceOk();
      res = DecodeValues::COMPLETED;
#endif
    } else {                           // Invalid char
      SentenceInvalid ();
      res = DecodeValues::CHR_INVALID;
    }
  } else if (RxState == RxStateValues::NMEA_RECEIVING_HEADER) { //------------------------

    //  The first field is the sentence type.  It will be used
    //  later by the virtual /parseField/.

    _crc ^= c;  // accumulate CRC as the chars come in...

    DecodeValues cmd_res = ParseCommand (c);

    if (cmd_res == DecodeValues::CHR_OK) {
      _chrCount++;
    } else if (cmd_res == DecodeValues::COMPLETED) {
      HeaderReceived ();
    } else { // DECODE_CHR_INVALID
      SentenceUnrecognized ();
    }

  } else if (RxState == RxStateValues::NMEA_RECEIVING_CRC) { //---------------------------
    bool err;
    uint8_t nybble { ParseHEX (c) };

    if (_chrCount == 0) {
      _chrCount++;
      err = ((_crc >> 4) != nybble);
    } else { // chrCount == 1
      err = ((_crc & 0x0F) != nybble);
      if (!err) {
        SentenceOk();
        res = DecodeValues::COMPLETED;
      }
    }

    if (err) {
#ifdef NMEAGPS_STATS
      statistics.errors++;
#endif
      SentenceInvalid();
    }

  } else if (RxState == RxStateValues::NMEA_IDLE) {
    // Reject non-start characters
    res = DecodeValues::CHR_INVALID;
    nmeaMessage = NmeaMessages::NMEA_UNKNOWN;
  }

  return res;

}
//----------------------------------------------------------------
DecodeValues GpsNema::Handle (uint8_t c) {
  DecodeValues res = Decode(c);

  if (res == DecodeValues::COMPLETED) {
    StoreFix();

  } else if ((NMEAGPS_FIX_MAX == 0) && _available () && !IsSafe ()) {
    // No buffer, and m_fix is was modified by the last char
    Overrun(true);
  }

  return res;

}
//----------------------------------------------------------------
void GpsNema::StoreFix () {
  // Room for another fix?

  bool room = ((NMEAGPS_FIX_MAX == 0) && !_available ())
      || ((NMEAGPS_FIX_MAX > 0) && (_available () < NMEAGPS_FIX_MAX));

  if (!room) {
    Overrun (true);

    if (keepNewestFixes) {
#if NMEAGPS_FIX_MAX > 0
      // Write over the oldest fix (_firstFix), so "pop" it off the front.
      _firstFix++;
      if (_firstFix >= NMEAGPS_FIX_MAX)
        _firstFix = 0;

      // this new one is not available until the interval is complete
      _fixesAvailable--;

#else
        // Write over the one and only fix.  It may not be complete.
        _fixesAvailable = false;
      #endif

      // Now there's room!
      room = true;
    }
  }

  if (room) {
    // YES, save it.
    //   Note: If FIX_MAX == 0, this just marks _fixesAvailable = true.

#if NMEAGPS_FIX_MAX > 0
    if (merging == MergingValue::EXPLICIT_MERGING) {
      // Accumulate all sentences
      buffer[_currentFix] |= Fix ();
    }
#endif

    if ((merging == MergingValue::NO_MERGING) || IntervalComplete ()) {

#if defined(NMEAGPS_TIMESTAMP_FROM_INTERVAL) & defined(GPS_FIX_TIME)

        // If this new fix is the start of a second, save the
        //   interval start time as the start of this UTC second.

        #if NMEAGPS_FIX_MAX > 0
          gps_fix & currentFix = buffer[ _currentFix ];
        #else
          gps_fix & currentFix = m_fix;
        #endif

        if (currentFix.valid.time && (currentFix.dateTime_cs == 0))
          UTCsecondStart( _IntervalStart );

      #endif

#if NMEAGPS_FIX_MAX > 0

      if (merging != MergingValue::EXPLICIT_MERGING) {
        buffer[_currentFix] = Fix ();
      }

      _currentFix++;
      if (_currentFix >= NMEAGPS_FIX_MAX) {
        _currentFix = 0;
      }

      if (_fixesAvailable < NMEAGPS_FIX_MAX) {
        _fixesAvailable++;
      }

#else // FIX_MAX == 0
      _fixesAvailable = true;
#endif

    }
  }

} // storeFix

//----------------------------------------------------------------
// NMEA Sentence strings (alphabetical)

#if defined(NMEAGPS_PARSE_GGA) | defined(NMEAGPS_RECOGNIZE_ALL)
static const char gga[] = "GGA";
#endif
#if defined(NMEAGPS_PARSE_GLL) | defined(NMEAGPS_RECOGNIZE_ALL)
static const char gll[] = "GLL";
#endif
#if defined(NMEAGPS_PARSE_GSA) | defined(NMEAGPS_RECOGNIZE_ALL)
static const char gsa[] = "GSA";
#endif
#if defined(NMEAGPS_PARSE_GST) | defined(NMEAGPS_RECOGNIZE_ALL)
static const char gst[] = "GST";
#endif
#if defined(NMEAGPS_PARSE_GSV) | defined(NMEAGPS_RECOGNIZE_ALL)
static const char gsv[] = "GSV";
#endif
#if defined(NMEAGPS_PARSE_RMC) | defined(NMEAGPS_RECOGNIZE_ALL)
static const char rmc[] = "RMC";
#endif
#if defined(NMEAGPS_PARSE_VTG) | defined(NMEAGPS_RECOGNIZE_ALL)
static const char vtg[] = "VTG";
#endif
#if defined(NMEAGPS_PARSE_ZDA) | defined(NMEAGPS_RECOGNIZE_ALL)
static const char zda[] = "ZDA";
#endif

static const char *const standardNmea[] = {
#if defined(NMEAGPS_PARSE_GGA) | defined(NMEAGPS_RECOGNIZE_ALL)
    gga,
#endif
#if defined(NMEAGPS_PARSE_GLL) | defined(NMEAGPS_RECOGNIZE_ALL)
    gll,
#endif
#if defined(NMEAGPS_PARSE_GSA) | defined(NMEAGPS_RECOGNIZE_ALL)
    gsa,
#endif
#if defined(NMEAGPS_PARSE_GST) | defined(NMEAGPS_RECOGNIZE_ALL)
    gst,
#endif
#if defined(NMEAGPS_PARSE_GSV) | defined(NMEAGPS_RECOGNIZE_ALL)
    gsv,
#endif
#if defined(NMEAGPS_PARSE_RMC) | defined(NMEAGPS_RECOGNIZE_ALL)
    rmc,
#endif
#if defined(NMEAGPS_PARSE_VTG) | defined(NMEAGPS_RECOGNIZE_ALL)
    vtg,
#endif
#if defined(NMEAGPS_PARSE_ZDA) | defined(NMEAGPS_RECOGNIZE_ALL)
    zda
#endif
    };

//  If you change this message table structure, be sure to update the optimizations in
//    ::string_for and ::parseCommand for !defined( NMEAGPS_DERIVED_TYPES )
const GpsNema::NmeaMessageTable GpsNema::_nmeaMessageTable = {
    (uint8_t)NMEA_FIRST_MSG,
    (const GpsNema::NmeaMessageTable*)nullptr,
    (uint8_t)(sizeof(standardNmea) / sizeof(standardNmea[0])),
    standardNmea
};

//----------------------------------------------------------------
//  For NMEA, start with talker or manufacture ID

DecodeValues GpsNema::ParseCommand (char c) {
  if (c == ',') {
    // End of field, did we get a sentence type yet?
    return (nmeaMessage == NmeaMessages::NMEA_UNKNOWN) ? DecodeValues::CHR_INVALID : DecodeValues::COMPLETED;
  }

#ifdef NMEAGPS_PARSE_PROPRIETARY
    if ((_chrCount == 0) && (c == 'P')) {
      //  Starting a proprietary message...
      proprietary = true;
      return DecodeValues::CHR_OK;
    }
  #endif

  uint8_t cmdCount = _chrCount;

#ifdef NMEAGPS_PARSE_PROPRIETARY
    if (proprietary) {
      // Next three chars are the manufacturer ID
      if (_chrCount < 4) {
#ifdef NMEAGPS_SAVE_MFR_ID
        mfr_id[chrCount-1] = c;
#endif

#ifdef NMEAGPS_PARSE_MFR_ID
        if (!ParseMfrID( c )) {
          return DecodeValues::CHR_INVALID;
        }
#endif
        return DecodeValues::CHR_OK;
      }

      cmdCount -= 4;

    } else
  #endif
  { // standard

    // First two chars are talker ID
    if (_chrCount < 2) {
#ifdef NMEAGPS_SAVE_TALKER_ID
      talker_id[chrCount] = c;
#endif

#ifdef NMEAGPS_PARSE_TALKER_ID
      if (!ParseTalkerID( c )) {
        return DecodeValues::CHR_INVALID;
      }
#endif

      return DecodeValues::CHR_OK;
    }

    cmdCount -= 2;
  }

  //  The remaining characters are the message type.

  const NmeaMessageTable * msgs = MessageTable();

  return ParseCommand (msgs, cmdCount, c);
}
//----------------------------------------------------------------
DecodeValues GpsNema::ParseCommand(const NmeaMessageTable * msgs, uint8_t cmdCount, char c) {
  DecodeValues res { DecodeValues::CHR_INVALID };

  for (;;) {
#ifdef NMEAGPS_DERIVED_TYPES
      uint8_t  table_size       = pgm_read_byte( &msgs->size );
      uint8_t  msg_offset       = pgm_read_byte( &msgs->offset );
      bool     check_this_table = true;
#else
    const uint8_t table_size = sizeof(standardNmea) / sizeof(standardNmea[0]);
    const uint8_t msg_offset = (uint8_t)NMEA_FIRST_MSG;
    const bool check_this_table = true;
#endif
    uint8_t entry;

    if (nmeaMessage == NmeaMessages::NMEA_UNKNOWN) {
      // We're just starting
      entry = 0;
    } else if ((msg_offset <= (uint8_t)nmeaMessage) && ((uint8_t)nmeaMessage < msg_offset + table_size)) {
      // In range of this table, pick up where we left off
      entry = (uint8_t)nmeaMessage - msg_offset;
    }
#ifdef NMEAGPS_DERIVED_TYPES
    else {
      check_this_table = false;
    }
#endif

    if (check_this_table) {
      uint8_t i = entry;

#if !defined( NMEAGPS_DERIVED_TYPES )
      const char *const*table = standardNmea;
#else
        const char * const *table   = (const char * const *) pgm_read_ptr( &msgs->table );
      #endif
      const char *table_i = (const char*)table[i];

      for (;;) {
        char rc = table_i[cmdCount];
        if (c == rc) {
          // ok so far...
          entry = i;
          res = DecodeValues::CHR_OK;
          break;
        }

        if (c < rc) {
          // Alphabetical rejection, check next table
          break;
        }

        // Ok to check another entry in this table
        uint8_t next_msg = i + 1;
        if (next_msg >= table_size) {
          // No more entries in this table.
          break;
        }

        //  See if the next entry starts with the same characters.
        const char *table_next = (const char*)table[next_msg];

        for (uint8_t j = 0; j < cmdCount; j++)
          if (table_i[j] != table_next[j]) {
            // Nope, a different start to this entry
            break;
          }
        i = next_msg;
        table_i = table_next;
      }
    }

    if (res == DecodeValues::CHR_INVALID) {

#ifdef NMEAGPS_DERIVED_TYPES
      msgs = (const msg_table_t *)msgs->previous;
      if (msgs) {
        // Try the current character in the previous table
        continue;
      }
#endif

    } else {
      //  This entry is good so far.
      nmeaMessage = (NmeaMessages)(entry + msg_offset);
    }

    return res;
  }

  return res;

}
//----------------------------------------------------------------
const std::string GpsNema::StringFor(NmeaMessages msg) const {
  if (msg == NmeaMessages::NMEA_UNKNOWN) {
    return "UNK";
  }

#ifdef NMEAGPS_DERIVED_TYPES
  const NemaMessageTable * msgs = MessageTable();
#endif

  for (;;) {
#ifdef NMEAGPS_DERIVED_TYPES
    uint8_t  table_size       = pgm_read_byte( &msgs->size );
    uint8_t  msg_offset       = pgm_read_byte( &msgs->offset );
#else
    const uint8_t table_size = sizeof(standardNmea) / sizeof(standardNmea[0]);
    const uint8_t msg_offset = (uint8_t)NMEA_FIRST_MSG;
#endif

    if ((msg_offset <= (uint8_t)msg) && ((uint8_t)msg < msg_offset + table_size)) {
      // In range of this table
#if !defined( NMEAGPS_DERIVED_TYPES )
      const char * const * table = standardNmea;
#else
        const char * const *table   = (const char * const *) pgm_read_ptr( &msgs->table );
      #endif
      const uint8_t i = ((uint8_t) msg) - msg_offset;
      const char * table_i = (const char*)table[i];
      return table_i;
    }

#ifdef NMEAGPS_DERIVED_TYPES
    // Try the previous table
    msgs = (const NemaMessageTable*)msgs->previous;
    if (msgs) {
      continue;
    }
#endif

    return nullptr;
  }

  return nullptr;
}
//----------------------------------------------------------------
bool GpsNema::ParseField (char chr) {
  switch (nmeaMessage) {

#if defined(NMEAGPS_PARSE_GGA)
    case NmeaMessages::NMEA_GGA:
      return ParseGGA (chr);
#endif

#if defined(NMEAGPS_PARSE_GLL)
    case NmeaMessages::NMEA_GLL:
      return ParseGLL (chr);
#endif

#if defined(NMEAGPS_PARSE_GSA)
    case NmeaMessages::NMEA_GSA:
      return ParseGSA (chr);
#endif

#if defined(NMEAGPS_PARSE_GST)
    case NmeaMessages::NMEA_GST:
      return ParseGST (chr);
#endif

#if defined(NMEAGPS_PARSE_GSV)
    case NmeaMessages::NMEA_GSV:
      return ParseGSV (chr);
#endif

#if defined(NMEAGPS_PARSE_RMC)
    case NmeaMessages::NMEA_RMC:
      return ParseRMC (chr);
#endif

#if defined(NMEAGPS_PARSE_VTG)
    case NmeaMessages::NMEA_VTG:
      return ParseVTG (chr);
#endif

#if defined(NMEAGPS_PARSE_ZDA)
    case NmeaMessages::NMEA_ZDA:
      return ParseZDA (chr);
#endif

    default:
      break;
  }

  return true;
}
//----------------------------------------------------------------
bool GpsNema::ParseGGA (char chr) {
#ifdef NMEAGPS_PARSE_GGA
  switch (_fieldIndex) {
    case 1:
      return ParseTime (chr);
    PARSE_LOC(2)
      ;
    case 6:
      return ParseFix (chr);
    case 7:
      return ParseSatellites (chr);
    case 8:
      return ParseHDOP (chr);
    case 9:
      return ParseAlt (chr);
    case 11:
      return ParseGeoidHeight (chr);
  }
#endif

  return true;
}
//----------------------------------------------------------------
bool GpsNema::ParseGLL(char chr) {
#ifdef NMEAGPS_PARSE_GLL
  switch (_fieldIndex) {
    PARSE_LOC(1)
      ;
    case 5:
      return ParseTime (chr);
    case 7:
      return ParseFix (chr);
  }
#endif

  return true;
}
//----------------------------------------------------------------
bool GpsNema::ParseGSA(char chr) {
#ifdef NMEAGPS_PARSE_GSA

  switch (_fieldIndex) {
    case 2:
      if (_chrCount == 0) {
        if ((chr == '2') || (chr == '3')) {
          _fix.status = GpsFix::Status::STATUS_STD;
          _fix.valid.Status = true;
        } else if (chr == '1') {
          _fix.status = GpsFix::Status::STATUS_NONE;
          _fix.valid.Status = true;
        } else if (ValidateChars() | ValidateFields()) {
          SentenceInvalid ();
        }
      }
      break;

    case 15:
#if !defined( NMEAGPS_PARSE_GSV )
      // Finalize the satellite count (for the fix *and* the satellites array)
      if (_chrCount == 0) {
        if (SatCount >= NMEAGPS_MAX_SATELLITES) {
          SatCount = NMEAGPS_MAX_SATELLITES - 1;
        }
#if  defined( NMEAGPS_PARSE_SATELLITES ) && !defined( NMEAGPS_PARSE_GGA )
        _fix.valid.Satellites = (_fix.satellites > 0);
#endif
      }
#endif
      return ParsePDOP (chr);

    case 16:
      return ParseHDOP (chr);
    case 17:
      return ParseVDOP (chr);
#if defined(GPS_FIX_VDOP) & !defined(NMEAGPS_COMMA_NEEDED)
#error When GSA and VDOP are enabled, you must define NMEAGPS_COMMA_NEEDED in NMEAGPS_cfg.h!
#endif

#ifdef NMEAGPS_PARSE_SATELLITES

      // It's not clear how this sentence relates to GSV and GGA.
      // GSA only allows 12 satellites, while GSV allows any number.
      // GGA just says how many are used to calculate a fix.

      // GGA shall have priority over GSA with respect to populating the
      // satellites field.  Don't use the number of satellite ID fields
      // to set the satellites field if GGA is enabled.

      // GSV shall have priority over GSA with respect to populating the
      // satellites array.  Ignore the satellite ID fields if GSV is enabled.

    case 1:
      break; // allows "default:" case for SV fields

#ifndef NMEAGPS_PARSE_GSV
    case 3:
      if (_chrCount == 0) {
        SatCount = 0;
#ifndef NMEAGPS_PARSE_GGA
          NMEAGPS_INVALIDATE(satellites);
          _fix.satellites = 0;
#endif
        CommaNeeded(true);
      }
    default:
      if (chr == ',') {
        if (_chrCount > 0) {
          SatCount++;
#ifndef NMEAGPS_PARSE_GGA
          _fix.satellites++;
#endif
        }
      } else if (SatCount < NMEAGPS_MAX_SATELLITES) {
        ParseInt(Satellites[SatCount].id, chr );
      }
      break;
#endif
#endif
  }
#endif

  return true;
}
//----------------------------------------------------------------
bool GpsNema::ParseGST(char chr) {
#ifdef NMEAGPS_PARSE_GST
  switch (_fieldIndex) {
    case 1:
      return ParseTime (chr);
    case 6:
      return ParseLatError (chr);
    case 7:
      return ParseLonError(chr);
    case 8:
      return ParseAltError(chr);
  }
#endif

  return true;

}
//----------------------------------------------------------------
bool GpsNema::ParseGSV(char chr) {
#if defined(NMEAGPS_PARSE_GSV) & defined(NMEAGPS_PARSE_SATELLITES)
#if !defined(NMEAGPS_PARSE_GSA) & !defined(NMEAGPS_PARSE_GGA)
  if ((SatCount == 0) && (_fieldIndex == 1) && (_chrCount == 0)) {
    NMEAGPS_INVALIDATE(satellites);
    _fix.satellites = 0;
  }
#endif

  if (SatCount < NMEAGPS_MAX_SATELLITES) {
    if (_fieldIndex >= 4) {

      switch (_fieldIndex % 4) {
#ifdef NMEAGPS_PARSE_SATELLITE_INFO
        case 0:
          ParseInt(Satellites[SatCount].id, chr);
          break;
        case 1:
          ParseInt(Satellites[SatCount].elevation, chr);
          break;
        case 2:
          if (chr != ',') {
            ParseInt(Satellites[SatCount].azimuth, chr);
          } else {
            // field 3 can be omitted, do some things now
            Satellites[SatCount].tracked = false;
            SatCount++;
          }
          break;
        case 3:
          if (chr != ',') {
            uint8_t snr = Satellites[SatCount - 1].snr;
            ParseInt(snr, chr);
            Satellites[SatCount - 1].snr = snr;
            CommaNeeded (true);
          } else {
            Satellites[SatCount - 1].tracked = (_chrCount != 0);
#if !defined(NMEAGPS_PARSE_GSA) & !defined(NMEAGPS_PARSE_GGA)
            if (Satellites[SatCount - 1].tracked) {
              _fix.satellites++;
              _fix.valid.Satellites = true; // but there may be more
            }
#endif
          }
          break;
#else
        case 0:
          if (chr != ',') {
            ParseInt(Satellites[SatCount].id, chr );
          } else {
            SatCount++;
#if !defined(NMEAGPS_PARSE_GSA) & !defined(NMEAGPS_PARSE_GGA)
            _fix.satellites++;
            _fix.valid.Satellites = true; // but there may be more
#endif
          }
          break;
        case 3:
#if !defined(NMEAGPS_PARSE_GSA) & !defined(NMEAGPS_PARSE_GGA)
          if ((chr == ',') && (_chrCount != 0)) {
            _fix.satellites++; // tracked
            _fix.valid.Satellites = true; // but there may be more
          }
#endif
          break;
#endif
      }
    }
  }
#endif

  return true;
}
//----------------------------------------------------------------
bool GpsNema::ParseRMC (char chr) {
#ifdef NMEAGPS_PARSE_RMC
  switch (_fieldIndex) {
    case 1:
      return ParseTime (chr);
    case 2:
      return ParseFix (chr);
    PARSE_LOC(3)
      ;
    case 7:
      return ParseSpeed (chr);
    case 8:
      return ParseHeading (chr);
    case 9:
      return ParseDDMMYY (chr);
  }
#endif

  return true;
}
//----------------------------------------------------------------
bool
GpsNema::ParseVTG(char chr) {
#ifdef NMEAGPS_PARSE_VTG
  switch (_fieldIndex) {
    case 1:
      return ParseHeading(chr);
    case 5:
      return ParseSpeed(chr);
    case 9:
      return ParseFix(chr);
  }
#endif

  return true;
}
//----------------------------------------------------------------
bool GpsNema::ParseZDA(char chr) {
#ifdef NMEAGPS_PARSE_ZDA
  switch (_fieldIndex) {
    case 1:
      return ParseTime(chr);

#ifdef GPS_FIX_DATE
    case 2:
      if (_chrCount == 0) {
        NMEAGPS_INVALIDATE(date);
        if (ValidateFields()) {
          CommaNeeded (true);
        }
      }
      ParseInt(_fix.dateTime.Date, chr);

      if (ValidateFields() && (_chrCount > 0) && (chr == ',')) {
        uint8_t days = TimeT::DaysIn[_fix.dateTime.Date];
        if ((_fix.dateTime.Date < 1) || (days < _fix.dateTime.Date)) {
          SentenceInvalid ();
        }
      }
      break;

    case 3:
      if (ValidateFields () && (_chrCount == 0)) {
        CommaNeeded(true);
      }

      ParseInt(_fix.dateTime.Month, chr);

      if (ValidateFields() && (_chrCount > 0) && (chr == ',') && ((_fix.dateTime.Month < 1) || (12 < _fix.dateTime.Month))) {
        SentenceInvalid ();
      }
      break;

    case 4:
      if (ValidateFields() && (_chrCount == 0)) {
        CommaNeeded (true);
      }

      if (chr != ',') {
        // year is BCD until terminating comma.
        // This essentially keeps the last two digits
        if (ValidateChars () && !isdigit (chr)) {
          SentenceInvalid ();
        } else if (_chrCount == 0) {
          CommaNeeded (true);
          _fix.dateTime.Year = (chr - '0');
        } else {
          _fix.dateTime.Year = (_fix.dateTime.Year << 4) + (chr - '0');
        }
      } else {
        // Terminating comma received, convert from BCD to decimal
        _fix.dateTime.Year = ToBinary(_fix.dateTime.Year);
        if (ValidateFields() && (((_chrCount != 2) && (_chrCount != 4)) || (99 < _fix.dateTime.Year))) {
          SentenceInvalid();
        } else {
          _fix.valid.Date = true;
        }
      }
      break;
#endif
  }
#endif

  return true;
}
//----------------------------------------------------------------
bool GpsNema::ParseTime(char chr) {
#ifdef GPS_FIX_TIME
  switch (_chrCount) {
    case 0:
      NMEAGPS_INVALIDATE(time);
      _fix.dateTimeCs = 0;

      if (chr != ',') {
        CommaNeeded (true);
        if (ValidateChars () && !isdigit (chr)) {
          SentenceInvalid ();
        } else {
          _fix.dateTime.Hours = (chr - '0') * 10;
        }
      }
      break;

    case 1:
      if (ValidateChars() && !isdigit(chr)) {
        SentenceInvalid();
      } else {
        _fix.dateTime.Hours += (chr - '0');
      }

      if (ValidateFields() && (23 < _fix.dateTime.Hours)) {
        SentenceInvalid();
      }
      break;

    case 2:
      if (ValidateChars () && !isdigit(chr)) {
        SentenceInvalid ();
      } else {
        _fix.dateTime.Minutes = (chr - '0') * 10;
      }
      break;
    case 3:
      if (ValidateChars() && !isdigit(chr)) {
        SentenceInvalid();
      } else {
        _fix.dateTime.Minutes += (chr - '0');
      }
      if (ValidateFields() && (59 < _fix.dateTime.Minutes)) {
        SentenceInvalid();
      }
      break;

    case 4:
      if (ValidateChars() && !isdigit (chr)) {
        SentenceInvalid();
      } else {
        _fix.dateTime.Seconds = (chr - '0') * 10;
      }
      break;
    case 5:
      if (ValidateChars() && !isdigit (chr)) {
        SentenceInvalid();
      } else {
        _fix.dateTime.Seconds += (chr - '0');
      }

      if (ValidateFields() && (59 < _fix.dateTime.Seconds)) {
        SentenceInvalid();
      }
      break;

    case 6:
      if (chr == ',') {
        _fix.valid.Time = true;
      } else if (ValidateChars () && (chr != '.')) {
        SentenceInvalid();
      }
      break;

    case 7:
      if (chr == ',') {
        _fix.valid.Time = true;
      } else if (ValidateChars() && !isdigit (chr)) {
        SentenceInvalid();
      } else {
        _fix.dateTimeCs = (chr - '0') * 10;
      }
      break;
    case 8:
      if (chr == ',') {
        _fix.valid.Time = true;
      } else if (ValidateChars() && !isdigit (chr)) {
        SentenceInvalid ();
      } else {
        _fix.dateTimeCs += (chr - '0');
        if (ValidateFields() && (99 < _fix.dateTimeCs)) {
          SentenceInvalid();
        } else {
          _fix.valid.Time = true;
        }
      }
      break;

    default:
      if (ValidateChars() && !isdigit (chr) && (chr != ',')) {
        SentenceInvalid();
      }
      break;
  }
#endif

  return true;
}
//----------------------------------------------------------------
bool GpsNema::ParseDDMMYY(char chr) {
#ifdef GPS_FIX_DATE
  switch (_chrCount) {
    case 0:
      NMEAGPS_INVALIDATE(date);

      if (chr != ',') {
        if (ValidateChars()) {
          CommaNeeded (true);
        }

        if (ValidateChars() && !isdigit(chr)) {
          SentenceInvalid();
        } else {
          _fix.dateTime.Date = (chr - '0') * 10;
        }
      }
      break;

    case 1:
      if (ValidateChars() && !isdigit(chr)) {
        SentenceInvalid();
      } else {
        _fix.dateTime.Date += (chr - '0');

        if (ValidateFields()) {
          uint8_t days = TimeT::DaysIn[_fix.dateTime.Date];
          if ((_fix.dateTime.Date < 1) || (days < _fix.dateTime.Date)) {
            SentenceInvalid();
          }
        }
      }
      break;

    case 2:
      if (ValidateChars() && !isdigit(chr)) {
        SentenceInvalid();
      } else {
        _fix.dateTime.Month = (chr - '0') * 10;
      }
      break;
    case 3:
      if (ValidateChars() && !isdigit(chr)) {
        SentenceInvalid();
      } else {
        _fix.dateTime.Month += (chr - '0');

        if (ValidateFields() && ((_fix.dateTime.Month < 1) || (12 < _fix.dateTime.Month))) {
          SentenceInvalid();
        }
      }
      break;

    case 4:
      if (ValidateChars() && !isdigit (chr)) {
        SentenceInvalid();
      } else {
        _fix.dateTime.Year = (chr - '0') * 10;
      }
      break;
    case 5:
      if (ValidateChars() && !isdigit(chr)) {
        SentenceInvalid();
      } else {
        _fix.dateTime.Year += (chr - '0');
        _fix.valid.Date = true;
      }
      break;

    case 6:
      if (ValidateChars () && (chr != ',')) {
        SentenceInvalid ();
      }
      break;
  }
#endif

  return true;
}
//----------------------------------------------------------------
bool GpsNema::ParseFix(char chr) {
  if (_chrCount == 0) {
    NMEAGPS_INVALIDATE( status );

    bool ok { true };
    if ((chr == '1') || (chr == 'A')) {
      _fix.status = GpsFix::Status::STATUS_STD;
    } else if ((chr == '0') || (chr == 'N') || (chr == 'V')) {
      _fix.status = GpsFix::Status::STATUS_NONE;
    } else if ((chr == '2') || (chr == 'D')) {
      _fix.status = GpsFix::Status::STATUS_DGPS;
    } else if (chr == '3') {
      _fix.status = GpsFix::Status::STATUS_PPS;
    } else if (chr == '4') {
      _fix.status = GpsFix::Status::STATUS_RTK_FIXED;
    } else if (chr == '5') {
      _fix.status = GpsFix::Status::STATUS_RTK_FLOAT;
    } else if ((chr == '6') || (chr == 'E')) {
      _fix.status = GpsFix::Status::STATUS_EST;
    } else {
      if (ValidateChars() | ValidateFields()) {
        SentenceInvalid();
      }
      ok = false;
    }
    if (ok) {
      _fix.valid.Status = true;
    }
  }

  if ((ValidateChars () | ValidateFields ()) && ((_chrCount > 1) || (chr != ','))) {
    SentenceInvalid ();
  }

  return true;
}
//----------------------------------------------------------------
bool GpsNema::ParseFloat(GpsFix::WholeFrac &val, char chr, uint8_t maxDecimal) {
  bool done {};

  if (_chrCount == 0) {
    CommaNeeded (true);
    _decimal = 0;
    _negative = (chr == '-');
    if (_negative) {
      return done;
    }
  }

  if (chr == ',') {
    // End of field, make sure it's scaled up
    if (!_decimal) {
      _decimal = 1;
    }

    if (val.Fraction) {
      while (_decimal++ <= maxDecimal) {
        val.Fraction *= 10;
      }
    }

    if (_negative) {
      val.Fraction = -val.Fraction;
      val.Whole = -val.Whole;
    }
    done = true;
  } else if (chr == '.') {
    _decimal = 1;
  } else if (ValidateChars () && !isdigit(chr)) {
    SentenceInvalid ();
  } else if (!_decimal) {
    val.Whole = val.Whole * 10 + (chr - '0');
  } else if (_decimal++ <= maxDecimal) {
    val.Fraction = val.Fraction * 10 + (chr - '0');
  }

  return done;
}
//----------------------------------------------------------------
bool GpsNema::ParseFloat (uint16_t &val, char chr, uint8_t maxDecimal) {
  bool done {};

  if (_chrCount == 0) {
    val = 0;
    CommaNeeded(true);
    _decimal = 0;
    _negative = (chr == '-');
    if (_negative) {
      return done;
    }
  }

  if (chr == ',') {
    if (val) {
      if (!_decimal) {
        _decimal = 1;
      }
      while (_decimal++ <= maxDecimal) {
        if (ValidateFields () && (val > 6553)) {
          SentenceInvalid ();
        } else {
          val *= 10;
        }
      }

      if (_negative) {
        val = -val;
      }
    }
    done = true;
  } else if (chr == '.') {
    _decimal = 1;
  } else if (ValidateChars () && !isdigit (chr)) {
    SentenceInvalid ();
  } else if (!_decimal || (_decimal++ <= maxDecimal)) {
    if (ValidateFields () && ((val > 6553) || ((val == 6553) && (chr > '5')))) {
      SentenceInvalid ();
    } else {
      val = val * 10 + (chr - '0');
    }
  }

  return done;
}
//-----------------------------------------------------------------------------
bool GpsNema::ParseDDDMM (
#if defined( GPS_FIX_LOCATION )
                     int32_t &val,
#endif
#if defined( GPS_FIX_LOCATION_DMS )
      Dms & dms,
    #endif
                     char chr) {
  bool done {};

#if defined( GPS_FIX_LOCATION ) | defined( GPS_FIX_LOCATION_DMS )
  if (_chrCount == 0) {
#ifdef GPS_FIX_LOCATION
    val = 0;
#endif
#ifdef GPS_FIX_LOCATION_DMS
    dms.init();
#endif
    _decimal = 0;
    CommaNeeded (true);
  }

  if ((chr == '.') || ((chr == ',') && !_decimal)) {
    // Now we know how many digits are in degrees; all but the last two.
    // Switch from BCD (digits) to binary minutes.
    _decimal = 1;
#ifdef GPS_FIX_LOCATION
    uint8_t *valBCD = (uint8_t*) &val;
#else
        uint8_t *valBCD = (uint8_t *) &dms;
      #endif
    uint8_t deg = ToBinary (valBCD[1]);
    if (valBCD[2] != 0) {
      deg += 100; // only possible if abs(longitude) >= 100.0 degrees
    }

    // Convert val to minutes
    uint8_t min { ToBinary (valBCD[0]) };

    if (ValidateFields () && (min >= 60)) {
      SentenceInvalid ();
    } else {
#ifdef GPS_FIX_LOCATION
      val = (deg * 60) + min;
#endif
#ifdef GPS_FIX_LOCATION_DMS
      dms.degrees   = deg;
      dms.minutes   = min;
      scratchpad.U4 = 0;
#endif
    }

    if (chr == '.') {
      return done;
    }
  }

  if (chr == ',') {
    // If the last chars in ".mmmmmm" were not received, force the value into its final state.

#ifdef GPS_FIX_LOCATION_DMS
    if (_decimal <= 5) {
      if (_decimal == 5) {
        scratchpad.U4 *= 10;
      } else if (_decimal == 4) {
        scratchpad.U4 *= 100;
      } else if (_decimal == 3) {
        scratchpad.U4 *= 1000;
      } else if (_decimal == 2) {
        scratchpad.U4 *= 10000;
      }

      FinalizeDMS(scratchpad.U4, dms);
    }
#endif

#ifdef GPS_FIX_LOCATION
    if (_decimal == 4) {
      val *= 100;
    } else if (_decimal == 5) {
      val *= 10;
    } else if (_decimal == 6) {

    } else if (_decimal > 6) {
      return true; // already converted at decimal==7
    } else if (_decimal == 3) {
      val *= 1000;
    } else if (_decimal == 2) {
      val *= 10000;
    } else if (_decimal == 1) {
      val *= 100000;
    }

    // Convert minutes x 1000000 to degrees x 10000000.
    val += divu3 (val * 2 + 1); // same as 10 * ((val+30)/60) without trunc
#endif

    done = true;
  } else if (ValidateChars () && !isdigit (chr)) {
    SentenceInvalid ();
  } else if (!_decimal) {
    // BCD until *after* decimal point

#ifdef GPS_FIX_LOCATION
    val = (val << 4) | (chr - '0');
#else
    uint32_t *val { (uint32_t *) &dms };
    *val = (*val << 4) | (chr - '0');
#endif

  } else {
    _decimal++;

#ifdef GPS_FIX_LOCATION_DMS
    if (_decimal <= 6) {
      scratchpad.U4 = scratchpad.U4 * 10 + (chr - '0');
      if (_decimal == 6) {
        FinalizeDMS(scratchpad.U4, dms);
      }
    }
#endif

#ifdef GPS_FIX_LOCATION
    if (_decimal <= 6) {
      val = val * 10 + (chr - '0');
    } else if (_decimal == 7) {
      // Convert now, while we still have the 6th decimal digit
      val += divu3 (val * 2 + 1); // same as 10 * ((val+30)/60) without trunc
      if (chr >= '9') {
        val += 2;
      } else if (chr >= '4') {
        val += 1;
      }
    }
#endif
  }

#endif

  return done;

} // parseDDDMM

//----------------------------------------------------------------

bool GpsNema::ParseLat(char chr) {
#if defined(GPS_FIX_LOCATION) | defined( GPS_FIX_LOCATION_DMS)
  if (_chrCount == 0) {
    _groupValid = (chr != ',');
    if (_groupValid) {
      NMEAGPS_INVALIDATE(location);
    }
  }

  if (_groupValid) {
    if (ParseDDDMM(
#if defined( GPS_FIX_LOCATION )
                  _fix.location._lat,
#endif
#if defined( GPS_FIX_LOCATION_DMS )
                  _fix.latitudeDMS,
#endif
                    chr)) {

      if (ValidateFields()) {

#if defined( GPS_FIX_LOCATION )
        if (_fix.location._lat > 900000000L) {
          SentenceInvalid ();
        }
#endif
#if defined( GPS_FIX_LOCATION_DMS )
          if ((_fix.latitudeDMS.degrees > 90) || ((_fix.latitudeDMS.degrees == 90) &&
              ((_fix.latitudeDMS.minutes > 0) || (_fix.latitudeDMS.seconds_whole > 0) || (_fix.latitudeDMS.seconds_frac  > 0) ))) {
            SentenceInvalid();
          }
#endif
      }
    }
  }
#endif

  return true;
}
//----------------------------------------------------------------
bool GpsNema::ParseNS(char chr) {
#if defined( GPS_FIX_LOCATION ) | defined( GPS_FIX_LOCATION_DMS )
  if (_groupValid) {
    if (_chrCount == 0) {
      // First char can only be 'N' or 'S'
      if (chr == 'S') {
#ifdef GPS_FIX_LOCATION
        _fix.location._lat = -_fix.location._lat;
#endif
#ifdef GPS_FIX_LOCATION_DMS
        _fix.latitudeDMS.hemisphere = SOUTH_H;
#endif
      } else if ((ValidateChars () | ValidateFields ()) && (chr != 'N')) {
        SentenceInvalid ();
      }

      // Second char can only be ','
    } else if ((ValidateChars () | ValidateFields ()) && ((_chrCount > 1) || (chr != ','))) {
      SentenceInvalid ();
    }
  }
#endif

  return true;
}
//----------------------------------------------------------------
bool GpsNema::ParseLon(char chr) {
#if defined(GPS_FIX_LOCATION) | defined(GPS_FIX_LOCATION_DMS)
  if ((chr == ',') && (_chrCount == 0)) {
    _groupValid = false;
  }

  if (_groupValid) {
    if (ParseDDDMM(
#if defined(GPS_FIX_LOCATION)
                    _fix.location._lon,
#endif
#if defined(GPS_FIX_LOCATION_DMS)
                _fix.longitudeDMS,
              #endif
                    chr)) {

      if (ValidateFields ()) {
#if defined( GPS_FIX_LOCATION )
        if (_fix.location._lon > 1800000000L) {
          SentenceInvalid ();
        }
#endif
#if defined(GPS_FIX_LOCATION_DMS)
          if ((_fix.longitudeDMS.degrees > 180) || ((_fix.longitudeDMS.degrees == 180) &&
              ((_fix.longitudeDMS.minutes > 0) || (_fix.longitudeDMS.seconds_whole > 0) || (_fix.longitudeDMS.seconds_frac  > 0) ))) {
            SentenceInvalid();
          }
#endif
      }
    }
  }
#endif

  return true;
}
//----------------------------------------------------------------
bool GpsNema::ParseEW(char chr) {
#if defined( GPS_FIX_LOCATION ) | defined( GPS_FIX_LOCATION_DMS )
  if (_groupValid) {
    if (_chrCount == 0) {
      _fix.valid.Location = true;

      // First char can only be 'W' or 'E'
      if (chr == 'W') {
#ifdef GPS_FIX_LOCATION
        _fix.location._lon = -_fix.location._lon;
#endif
#ifdef GPS_FIX_LOCATION_DMS
        _fix.longitudeDMS.hemisphere = WEST_H;
#endif
      } else if ((ValidateChars () | ValidateFields ()) && (chr != 'E')) {
        SentenceInvalid ();
      }
    } else if ((ValidateChars () | ValidateFields ()) && ((_chrCount > 1) || (chr != ','))) {
      SentenceInvalid ();
    }
  }
#endif

  return true;
}
//----------------------------------------------------------------
bool GpsNema::ParseSpeed(char chr) {
#ifdef GPS_FIX_SPEED
  if (_chrCount == 0) {
    NMEAGPS_INVALIDATE(speed);
  }

  if (ParseFloat(_fix.spd, chr, 3)) {
    if (ValidateFields () && _fix.valid.Speed && _negative) {
      SentenceInvalid ();
    } else {
      _fix.valid.Speed = (_chrCount != 0);
    }
  }
#endif

  return true;
}
//----------------------------------------------------------------
bool GpsNema::ParseSpeedKph(char chr) {
#ifdef GPS_FIX_SPEED
  ParseSpeed(chr);

  if ((chr == ',') && _fix.valid.Speed) {
    uint32_t kph { static_cast<uint32_t>(_fix.spd.Int32_000()) };
    // Convert to Nautical Miles/Hour
    uint32_t nmiph { (kph * 1000) / M_PER_NMI };
    _fix.spd.Whole = nmiph / 1000;
    _fix.spd.Fraction = (nmiph - _fix.spd.Whole * 1000);
  }
#endif

  return true;
}
//----------------------------------------------------------------
bool GpsNema::ParseHeading(char chr) {
#ifdef GPS_FIX_HEADING
  if (_chrCount == 0) {
    NMEAGPS_INVALIDATE(heading);
  }

  if (ParseFloat (_fix.hdg, chr, 2)) {
    if (ValidateFields () && _fix.valid.Heading && (_negative || (_fix.hdg.Whole >= 360))) {
      SentenceInvalid ();
    } else {
      _fix.valid.Heading = (_chrCount != 0);
    }
  }
#endif

  return true;
}
//----------------------------------------------------------------
bool GpsNema::ParseAlt (char chr) {
#ifdef GPS_FIX_ALTITUDE
  if (_chrCount == 0) {
    NMEAGPS_INVALIDATE( altitude );
  }

  if (ParseFloat(_fix.alt, chr, 2)) {
    if (ValidateFields() && (_fix.alt.Whole < -1000)) {
      SentenceInvalid ();
    } else {
      _fix.valid.Altitude = (_chrCount != 0);
    }
  }
#endif

  return true;
}
//----------------------------------------------------------------
bool GpsNema::ParseGeoidHeight (char chr) {
#ifdef GPS_FIX_GEOID_HEIGHT
  if (_chrCount == 0) {
    NMEAGPS_INVALIDATE( geoidHeight );
  }

  if (ParseFloat (_fix.geoidHt, chr, 2)) {
    _fix.valid.GeoidHeight = (_chrCount != 0);
  }
#endif

  return true;

}

//----------------------------------------------------------------

bool GpsNema::ParseSatellites (char chr) {
#ifdef GPS_FIX_SATELLITES
  if (_chrCount == 0) {
    NMEAGPS_INVALIDATE( satellites );
  }

  if (ParseInt(_fix.satellites, chr)) {
    if (ValidateFields() && _negative) {
      SentenceInvalid();
    } else {
      _fix.valid.Satellites = true;
    }
  }
#endif

  return true;
}
//----------------------------------------------------------------
bool GpsNema::ParseHDOP(char chr) {
#ifdef GPS_FIX_HDOP
  if (_chrCount == 0) {
    NMEAGPS_INVALIDATE(hdop);
  }

  if (ParseFloat(_fix.hdop, chr, 3)) {
    if (ValidateFields() && _negative) {
      SentenceInvalid ();
    } else {
      _fix.valid.Hdop = (_chrCount != 0);
    }
  }
#endif

  return true;
}

//----------------------------------------------------------------

bool GpsNema::ParseVDOP(char chr) {
#ifdef GPS_FIX_VDOP
  if (_chrCount == 0) {
    NMEAGPS_INVALIDATE(vdop);
  }
  if (ParseFloat(_fix.vdop, chr, 3)) {
    if (ValidateFields() && _negative) {
      SentenceInvalid();
    } else {
      _fix.valid.Vdop = (_chrCount != 0);
    }
  }
#endif

  return true;
}
//----------------------------------------------------------------
bool GpsNema::ParsePDOP(char chr) {
#ifdef GPS_FIX_PDOP
  if (_chrCount == 0) {
    NMEAGPS_INVALIDATE(pdop);
  }

  if (ParseFloat(_fix.pdop, chr, 3)) {
    if (ValidateFields() && _negative) {
      SentenceInvalid ();
    } else {
      _fix.valid.Pdop = (_chrCount != 0);
    }
  }
#endif

  return true;

}
//----------------------------------------------------------------

static const uint16_t MAX_ERROR_CM = 20000; // 200m is a large STD

bool GpsNema::ParseLatError(char chr) {
#ifdef GPS_FIX_LAT_ERR
  if (_chrCount == 0) {
    NMEAGPS_INVALIDATE( lat_err );
  }

  if (ParseFloat (_fix.latErrCm, chr, 2)) {
    if (ValidateFields () && (_negative || (_fix.valid.LatError > MAX_ERROR_CM))) {
      SentenceInvalid ();
    } else {
      _fix.valid.LatError = (_chrCount != 0);
    }
  }
#endif

  return true;

} // parse_lat_err

//----------------------------------------------------------------

bool GpsNema::ParseLonError(char chr) {
#ifdef GPS_FIX_LON_ERR
  if (_chrCount == 0) {
    NMEAGPS_INVALIDATE( lon_err );
  }

  if (ParseFloat(_fix.lonErrCm, chr, 2)) {
    if (ValidateFields () && (_negative || (_fix.valid.LonError > MAX_ERROR_CM))) {
      SentenceInvalid ();
    } else {
      _fix.valid.LonError = (_chrCount != 0);
    }
  }
#endif

  return true;

}
//----------------------------------------------------------------
bool GpsNema::ParseAltError(char chr) {
#ifdef GPS_FIX_ALT_ERR
  if (_chrCount == 0) {
    NMEAGPS_INVALIDATE(alt_err);
  }

  if (ParseFloat(_fix.altErrCm, chr, 2)) {
    if (ValidateFields () && (_negative || (_fix.valid.AltError > MAX_ERROR_CM))) {
      SentenceInvalid ();
    } else {
      _fix.valid.AltError = (_chrCount != 0);
    }
  }
#endif

  return true;

}
//----------------------------------------------------------------
const GpsFix GpsNema::Read () {
  GpsFix fix;

  if (_fixesAvailable) {
#if (NMEAGPS_FIX_MAX > 0)
    _fixesAvailable--;
    fix = buffer[_firstFix];
    if (merging == MergingValue::EXPLICIT_MERGING)
      // Prepare to accumulate all fixes in an interval
      buffer[_firstFix].Init ();
    if (++_firstFix >= NMEAGPS_FIX_MAX)
      _firstFix = 0;
#else
    if (is_safe()) {
      _fixesAvailable = false;
      fix = m_fix;
    }
#endif
  }

  return fix;

}
//----------------------------------------------------------------
void GpsNema::Poll (Airsoft::Drivers::Uarts * device, NmeaMessages msg) {
  //  Only the ublox documentation references talker ID "EI".
  //  Other manufacturer's devices use "II" and "GP" talker IDs for the GPQ sentence.
  //  However, "GP" is reserved for the GPS device, so it seems inconsistent
  //  to use that talker ID when requesting something from the GPS device.

#if defined(NMEAGPS_PARSE_GGA) | defined(NMEAGPS_RECOGNIZE_ALL)
  static const char gga[] { "EIGPQ,GGA" };
#endif
#if defined(NMEAGPS_PARSE_GLL) | defined(NMEAGPS_RECOGNIZE_ALL)
  static const char gll[] { "EIGPQ,GLL" };
#endif
#if defined(NMEAGPS_PARSE_GSA) | defined(NMEAGPS_RECOGNIZE_ALL)
  static const char gsa[] { "EIGPQ,GSA" };
#endif
#if defined(NMEAGPS_PARSE_GST) | defined(NMEAGPS_RECOGNIZE_ALL)
  static const char gst[] { "EIGPQ,GST" };
#endif
#if defined(NMEAGPS_PARSE_GSV) | defined(NMEAGPS_RECOGNIZE_ALL)
  static const char gsv[] { "EIGPQ,GSV" };
#endif
#if defined(NMEAGPS_PARSE_RMC) | defined(NMEAGPS_RECOGNIZE_ALL)
  static const char rmc[] { "EIGPQ,RMC" };
#endif
#if defined(NMEAGPS_PARSE_VTG) | defined(NMEAGPS_RECOGNIZE_ALL)
  static const char vtg[] { "EIGPQ,VTG" };
#endif
#if defined(NMEAGPS_PARSE_ZDA) | defined(NMEAGPS_RECOGNIZE_ALL)
  static const char zda[] { "EIGPQ,ZDA" };
#endif

  static const char *const PollMessages[] {
#if defined(NMEAGPS_PARSE_GGA) | defined(NMEAGPS_RECOGNIZE_ALL)
      gga,
#endif
#if defined(NMEAGPS_PARSE_GLL) | defined(NMEAGPS_RECOGNIZE_ALL)
      gll,
#endif
#if defined(NMEAGPS_PARSE_GSA) | defined(NMEAGPS_RECOGNIZE_ALL)
      gsa,
#endif
#if defined(NMEAGPS_PARSE_GST) | defined(NMEAGPS_RECOGNIZE_ALL)
      gst,
#endif
#if defined(NMEAGPS_PARSE_GSV) | defined(NMEAGPS_RECOGNIZE_ALL)
      gsv,
#endif
#if defined(NMEAGPS_PARSE_RMC) | defined(NMEAGPS_RECOGNIZE_ALL)
      rmc,
#endif
#if defined(NMEAGPS_PARSE_VTG) | defined(NMEAGPS_RECOGNIZE_ALL)
      vtg,
#endif
#if defined(NMEAGPS_PARSE_ZDA) | defined(NMEAGPS_RECOGNIZE_ALL)
      zda
#endif
      };

  if ((NMEA_FIRST_MSG <= msg) && (msg <= NMEA_LAST_MSG)) {
    std::string pollCmd = (const char*)&PollMessages[(uint8_t)(msg) - (uint8_t)(NMEA_FIRST_MSG)];
    SendString(device, pollCmd);
  }

}
//----------------------------------------------------------------
static void SendTrailer(Airsoft::Drivers::Uarts * device, uint8_t crc) {
  char hexDigit = '*';
  device->Write((uint8_t*)&hexDigit, 1);

  hexDigit = FormatHex (crc >> 4);
  device->Write((uint8_t*)&hexDigit, 1);

  hexDigit = FormatHex (crc);
  device->Write((uint8_t*)&hexDigit, 1);

  hexDigit = CR;
  device->Write((uint8_t*)&hexDigit, 1);
  hexDigit = LF;
  device->Write((uint8_t*)&hexDigit, 1);

}
//----------------------------------------------------------------
void GpsNema::Send (Airsoft::Drivers::Uarts * device, const char *msg) {
  if (msg && *msg) {
    if (*msg == '$') {
      msg++;
    }
    char dollar = '$';

    device->Write((uint8_t*)&dollar, 1);

    uint8_t sentTrailer = 0;
    uint8_t crc = 0;
    while (*msg) {
      if ((*msg == '*') || (sentTrailer > 0))
        sentTrailer++;
      else
        crc ^= *msg;
      device->Write((uint8_t*)msg++, 1);
    }

    if (!sentTrailer) {
      SendTrailer (device, crc);
    }
  }

}
//----------------------------------------------------------------
void GpsNema::SendString(Airsoft::Drivers::Uarts * device, const std::string & msg) {
  const char * ptr = msg.c_str();
  char chr = (char)*ptr++;
  char dollar = '$';

  device->Write((uint8_t*)&dollar, 1);
  if (chr == '$') {
    chr = (char)*ptr++;
  }
  uint8_t sent_trailer = 0;
  uint8_t crc = 0;
  while (chr) {
    if ((chr == '*') || (sent_trailer > 0))
      sent_trailer++;
    else
      crc ^= chr;
    device->Write((uint8_t*)&chr, 1);

    chr = (char)*ptr++;
  }

  if (!sent_trailer) {
    SendTrailer (device, crc);
  }
}

} // namespace Airsoft::Neo
