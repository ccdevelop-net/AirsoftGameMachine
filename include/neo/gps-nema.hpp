#ifndef GPS_NEMA_HPP_
#define GPS_NEMA_HPP_

#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstdbool>
#include <string>
#include <cstring>
#include <memory>

#include <drivers/uarts.hpp>

#include <gps-config.hpp>

#include <neo/gps-fix.hpp>

namespace Airsoft::Neo {

// NMEA standard message types (aka "sentences")
enum class NmeaMessages : uint8_t {
  NMEA_UNKNOWN,

#if defined(NMEAGPS_PARSE_GGA) | defined(NMEAGPS_RECOGNIZE_ALL)
  NMEA_GGA,
#endif

#if defined(NMEAGPS_PARSE_GLL) | defined(NMEAGPS_RECOGNIZE_ALL)
  NMEA_GLL,
#endif

#if defined(NMEAGPS_PARSE_GSA) | defined(NMEAGPS_RECOGNIZE_ALL)
  NMEA_GSA,
#endif

#if defined(NMEAGPS_PARSE_GST) | defined(NMEAGPS_RECOGNIZE_ALL)
  NMEA_GST,
#endif

#if defined(NMEAGPS_PARSE_GSV) | defined(NMEAGPS_RECOGNIZE_ALL)
  NMEA_GSV,
#endif

#if defined(NMEAGPS_PARSE_RMC) | defined(NMEAGPS_RECOGNIZE_ALL)
  NMEA_RMC,
#endif

#if defined(NMEAGPS_PARSE_VTG) | defined(NMEAGPS_RECOGNIZE_ALL)
  NMEA_VTG,
#endif

#if defined(NMEAGPS_PARSE_ZDA) | defined(NMEAGPS_RECOGNIZE_ALL)
  NMEA_ZDA,
#endif

  NMEAMSG_END // a bookend that tells how many enums there were
};

/**
 * @brief As characters are processed, they can be categorized as :
 *        INVALID (not part of this protocol)
 *        OK (accepted)
 *        COMPLETED (end-of-message)
 */

enum class DecodeValues : uint8_t {
  CHR_INVALID,
  CHR_OK,
  COMPLETED
};


constexpr NmeaMessages NMEA_FIRST_MSG { NmeaMessages::NMEA_GGA };
constexpr NmeaMessages NMEA_LAST_MSG  { NmeaMessages::NMEA_ZDA };

// Internal FSM states
enum class RxStateValues : uint8_t {
    NMEA_IDLE,             // Waiting for initial '$'
    NMEA_RECEIVING_HEADER, // Parsing sentence type field
    NMEA_RECEIVING_DATA,   // Parsing fields up to the terminating '*'
    NMEA_RECEIVING_CRC,    // Receiving two-byte transmitted CRC
    NMEA_UNDEFINED = 8
};
constexpr RxStateValues NMEA_FIRST_STATE = RxStateValues::NMEA_IDLE;
constexpr RxStateValues NMEA_LAST_STATE  = RxStateValues::NMEA_RECEIVING_CRC;

enum class MergingValue : uint8_t {
  NO_MERGING,
  EXPLICIT_MERGING,
  IMPLICIT_MERGING
};

//------------------------------------------------------
//
// NMEA 0183 Parser for generic GPS Modules.
//
// As bytes are received from the device, they affect the
// internal FSM and set various members of the current /fix/.
// As multiple sentences are received, they are (optionally)
// merged into a single fix.  When the last sentence in a time
// interval (usually 1 second) is received, the fix is stored
// in the (optional) buffer of fixes.
//
// Only these NMEA messages are parsed:
//      GGA, GLL, GSA, GST, GSV, RMC, VTG, and ZDA.

class GpsNema {
private:
  GpsNema & operator =( const GpsNema & );
  GpsNema(const NMEAGPS &);

public:
  GpsNema();

  //=======================================================================
  // FIX-ORIENTED methods: available, read, overrun and handle
  //=======================================================================
  // The typical sketch should have something like this in loop():
  //
  //    while (gps.available( Serial1 )) {
  //      gps_fix fix = gps.read();
  //      if (fix.valid.location) {
  //         ...
  //      }
  //    }

  //.......................................................................
  // The available(...) functions return the number of *fixes* that
  //   are available to be "read" from the fix buffer.  The GPS port
  //   object is passed in so a char can be read if port.available().

  uint8_t Available(Airsoft::Drivers::Uarts & port) {
    size_t dataSize = port.Available();
    std::unique_ptr<uint8_t[]> buf;

    if (dataSize > 0) {
      buf.reset(new uint8_t[dataSize]);

      for(size_t elem {}; elem < dataSize; elem++) {
        Handle(buf.get()[elem]);
      }
    }

    return _available();
  }
  uint8_t Available() const volatile {
    return _available();
  }

  /**
   * @brief Return the next available fix. When no more fixes are available, it returns an empty fix.
   */
  const GpsFix Read(void);


  //  The OVERRUN flag is set whenever a fix is not read by the time
  //  the next update interval starts.  You must clear it when you
  //  detect the condition.

  bool Overrun(void) const {
    return _overrun;
  }
  void Overrun(bool val) {
    _overrun = val;
  }

  /**
   * @brief Process one character, possibly saving a buffered fix.
   * @param c
   * @return
   */
  DecodeValues Handle(uint8_t c);

  //=======================================================================
  // CHARACTER-ORIENTED methods: decode, fix and is_safe
  //=======================================================================
  //
  //    *** MOST APPLICATIONS SHOULD USE THE FIX-ORIENTED METHODS ***
  //
  //    Using `decode` is only necessary if you want finer control
  //    on how fix information is filtered and merged.
  //
  // Process one character of an NMEA GPS sentence.  The internal state
  // machine tracks what part of the sentence has been received.  As the
  // sentence is received, members of the /fix/ structure are updated.
  // This character-oriented method *does not* buffer any fixes, and
  // /read()/ will always return an empty fix.
  //
  // @return DECODE_COMPLETED when a sentence has been completely received.

  DecodeValues Decode(char c);

  //.......................................................................
  //  Current fix accessor.
  //    *** MOST APPLICATIONS SHOULD USE read() TO GET THE CURRENT FIX  ***
  //    /fix/ will be constantly changing as characters are received.
  //
  //  For example, fix().longitude() may return nonsense data if
  //  characters for that field are currently being processed in /decode/.

  GpsFix & fix(void) {
    return _fix;
  };

  //  NOTE: /is_safe/ *must* be checked before accessing members of /fix/.
  //  If you need access to the current /fix/ at any time, you should
  //  use the FIX-ORIENTED methods.

  //.......................................................................
  //  Determine whether the members of /fix/ are "currently" safe.
  //  It will return true when a complete sentence and the CRC characters
  //  have been received (or after a CR if no CRC is present).
  //  It will return false after a new sentence starts.

  bool IsSafe(void) const volatile {
    return (RxState == RxStateValues::NMEA_IDLE);
  }

  //  NOTE:  When INTERRUPT_PROCESSING is enabled, is_safe()
  //  and fix() could change at any time (i.e., they should be
  //  considered /volatile/).

  //=======================================================================
  // DATA MEMBER accessors and mutators
  //=======================================================================

  //.......................................................................
  //  Convert a nmea_msg_t to a PROGMEM string.
  //    Useful for printing the sentence type instead of a number.
  //    This can return "UNK" if the message is not a valid number.

  const std::string StringFor(NmeaMessages msg) const;

  // Most recent NMEA sentence type received.
  NmeaMessages nmeaMessage { NmeaMessages::NMEAMSG_END };

  //  Storage for Talker and Manufacturer IDs

#ifdef NMEAGPS_SAVE_TALKER_ID
  char talker_id[2];
#endif

#ifdef NMEAGPS_SAVE_MFR_ID
  char mfr_id[3];
#endif

  //  Various parsing statistics

#ifdef NMEAGPS_STATS
  struct statistics_t {
    uint32_t ok {};     // count of successfully parsed sentences
    uint32_t errors {}; // NMEA checksum or other message errors
    uint32_t chars {};
  } statistics;
#endif

  // SATELLITE VIEW array

#ifdef NMEAGPS_PARSE_SATELLITES
  struct SatelliteView {
    uint8_t    id;
#ifdef NMEAGPS_PARSE_SATELLITE_INFO
    uint8_t  elevation; // 0..99 deg
    uint16_t azimuth;   // 0..359 deg
    uint8_t  snr     { 7 }; // 0..99 dBHz
    bool     tracked { 1 };
#endif
  } NEOGPS_PACKED;

  SatelliteView Satellites[NMEAGPS_MAX_SATELLITES];
  uint8_t       SatCount; // in the above array

  bool SatellitesValid() const { return (SatCount >= _fix.satellites); }
#endif

  //.......................................................................
  // Reset the parsing process.
  //   This is used internally after a CS error, or could be used
  //   externally to abort processing if it has been too long
  //   since any data was received.

  void Reset(void) {
    RxState = RxStateValues::NMEA_IDLE;
  }

  //=======================================================================
  // CORRELATING Arduino micros() WITH UTC.
  //=======================================================================

#if defined(NMEAGPS_TIMESTAMP_FROM_PPS) |  defined(NMEAGPS_TIMESTAMP_FROM_INTERVAL)
  protected:
    uint32_t _utcSecondStart;
#if defined(NMEAGPS_TIMESTAMP_FROM_INTERVAL) & (defined(GPS_FIX_DATE) | defined(GPS_FIX_TIME))
    uint32_t _intervalStart; // quiet time just ended
#endif
  public:

    // The micros() value when the current UTC second started
    uint32_t UTCsecondStart() const
      { lock();
          uint32_t ret = _UTCsecondStart;
        unlock();
        return ret;
      };
    void     UTCsecondStart( uint32_t us ) { _UTCsecondStart = us; };

    // The elapsed time since the start of the current UTC second
    uint32_t UTCus() const { return micros() - UTCsecondStart(); };
    uint32_t UTCms() const { return UTCus() / 1000UL; };
#endif

  //=======================================================================
  // COMMUNICATING WITH THE GPS DEVICE: poll, send and send_P
  //=======================================================================

  // Request the specified NMEA sentence.  Not all devices will respond.
  static void Poll(Airsoft::Drivers::Uarts * device, NmeaMessages msg );

  // Send a message to the GPS filedevice.
  // The '$' is optional, and the '*' and CS will be added automatically.

  static void Send(Airsoft::Drivers::Uarts * device, const char * msg );
  static void SendString(Airsoft::Drivers::Uarts * device, const std::string & msg );

  protected:
    //.......................................................................
    // Table entry for NMEA sentence type string and its offset
    // in enumerated nmea_msg_t.  Proprietary sentences can be implemented
    // in derived classes by adding a second table.  Additional tables
    // can be singly-linked through the /previous/ member.  The instantiated
    // class's table is the head, and should be returned by the derived
    // /msg_table/ function.  Tables should be sorted alphabetically.

    struct NmeaMessageTable {
      uint8_t                   offset;  // nmea_msg_t enum starting value
      const NmeaMessageTable  * previous;
      uint8_t                   size;    // number of entries in table array
      const char * const *      table;   // array of NMEA sentence strings
    };

    static const NmeaMessageTable  _nmeaMessageTable;
    const NmeaMessageTable * MessageTable(void) const {
      return &_nmeaMessageTable;
    }

    //.......................................................................
    //  These virtual methods can accept or reject
    //    the talker ID (for standard sentences) or
    //    the manufacturer ID (for proprietary sentences).
    //  The default is to accept *all* IDs.
    //  Override them if you want to reject certain IDs, or you want
    //    to handle COMPLETED sentences from certain IDs differently.

#ifdef NMEAGPS_PARSE_TALKER_ID
    bool ParseTalkerID(char) {
      return true;
    }
#endif

#ifdef NMEAGPS_PARSE_PROPRIETARY
#ifdef NMEAGPS_PARSE_MFR_ID
    bool ParseMfrID(char) {
      return true;
    }
#endif
#endif

  public:

    // Set all parsed data to initial values.
    void DataInit(void) {
      _fix.Init();

#ifdef NMEAGPS_PARSE_SATELLITES
      SatCount = 0;
#endif
    }

    //.......................................................................

    static const MergingValue merging { NMEAGPS_MERGING };

    static const bool keepNewestFixes { NMEAGPS_KEEP_NEWEST_FIXES };

    static const bool ValidateChars(void) {
      return NMEAGPS_VALIDATE_CHARS;
    }
    static const bool ValidateFields(void) {
      return NMEAGPS_VALIDATE_FIELDS;
    }

  protected:
    //=======================================================================
    //   PARSING FINITE-STATE MACHINE
    //=======================================================================

    //  Current fix
    GpsFix _fix;

    // Current parser state
    uint8_t      _crc;            // accumulated CRC in the sentence
    uint8_t      _fieldIndex;     // index of current field in the sentence
    uint8_t      _chrCount;       // index of current character in current field
    uint8_t      _decimal;        // digits received after the decimal point

    bool _negative          { true }; // field had a leading '-'
    bool _commaNeeded       { true }; // field needs a comma to finish parsing
    bool _groupValid        { true }; // multi-field group valid
    bool _overrun           { true }; // an entire fix was dropped
    bool _intervalComplete  { true }; // automatically set after LAST received

#if (NMEAGPS_FIX_MAX == 0)
    bool   _fixesAvailable { true };
#endif
#ifdef NMEAGPS_PARSE_PROPRIETARY
    bool   Proprietary     { true }; // receiving proprietary message
#endif

#ifdef NMEAGPS_PARSING_SCRATCHPAD
    union {
      uint32_t U4;
      uint16_t U2[2];
      uint8_t  U1[4];
    } scratchpad;
#endif

    bool CommaNeeded(void) {
#ifdef NMEAGPS_COMMA_NEEDED
      return _commaNeeded;
#else
      return false;
#endif
    }

    void CommaNeeded(bool value) {
#ifdef NMEAGPS_COMMA_NEEDED
      _commaNeeded = value;
#endif
    }

    RxStateValues RxState { RxStateValues::NMEA_UNDEFINED };

    uint8_t _available() const volatile {
      return _fixesAvailable;
    }

    //  Buffered fixes.
#if (NMEAGPS_FIX_MAX > 0)
    GpsFix buffer[NMEAGPS_FIX_MAX]; // could be empty, see NMEAGPS_cfg.h
    uint8_t _fixesAvailable;
    uint8_t _firstFix;
    uint8_t _currentFix;
#endif

    // Indicate that the next sentence should initialize the internal data.
    // This is useful for coherency or custom filtering.
    bool IntervalComplete(void) const {
      return _intervalComplete;
    }
    void IntervalComplete(bool val) {
      _intervalComplete = val;
    }

    // Identify when an update interval is completed, according to the most recently-received sentence.
    // In this base class, it just looks at the nmeaMessage member.
    // Derived classes may have more complex, specific conditions.
    bool IntervalCompleted() const {
      return (nmeaMessage == LAST_SENTENCE_IN_INTERVAL);
    }

    // When a fix has been fully assembled from a batch of sentences, as  determined by the configured merging technique
    // and ending with the LAST_SENTENCE_IN_INTERVAL, it is stored in the (optional) buffer of fixes.
    // They are removed with Read().
    void StoreFix();

    //=======================================================================
    //   PARSING METHODS
    //=======================================================================

    // Try to recognize an NMEA sentence type, after the IDs have been accepted.

    DecodeValues ParseCommand(char c);
    DecodeValues ParseCommand(const NmeaMessageTable * msgs, uint8_t cmdCount, char c);

    // Parse various NMEA sentences
    bool ParseGGA(char chr);
    bool ParseGLL(char chr);
    bool ParseGSA(char chr);
    bool ParseGST(char chr);
    bool ParseGSV(char chr);
    bool ParseRMC(char chr);
    bool ParseVTG(char chr);
    bool ParseZDA(char chr);

    // Depending on the NMEA sentence type, parse one field of an expected type.
    bool ParseField(char chr);

    // Parse the primary NMEA field types into /fix/ members.
    bool ParseFix(char chr); // aka STATUS or MODE
    bool ParseTime(char chr);
    bool ParseDDMMYY(char chr);
    bool ParseLat(char chr);
    bool ParseNS(char chr);
    bool ParseLon(char chr);
    bool ParseEW(char chr);
    bool ParseSpeed(char chr);
    bool ParseSpeedKph(char chr);
    bool ParseHeading(char chr);
    bool ParseAlt(char chr);
    bool ParseGeoidHeight(char chr);
    bool ParseHDOP(char chr);
    bool ParseVDOP(char chr);
    bool ParsePDOP(char chr);
    bool ParseLatError(char chr);
    bool ParseLonError(char chr);
    bool ParseAltError(char chr);
    bool ParseSatellites(char chr);

    // Helper macro for parsing the 4 consecutive fields of a location
    #define PARSE_LOC(i) case i: return ParseLat(chr);\
      case i + 1: return ParseNS (chr); \
      case i + 2: return ParseLon(chr); \
      case i + 3: return ParseEW (chr);

    // Parse floating-point numbers into a /whole_frac/
    // @return true when the value is fully populated.
    bool ParseFloat(GpsFix::WholeFrac & val, char chr, uint8_t maxDecimal);

    // Parse floating-point numbers into a uint16_t
    // @return true when the value is fully populated.
    bool ParseFloat(uint16_t & val, char chr, uint8_t maxDecimal);

    // Parse NMEA lat/lon dddmm.mmmm degrees
    bool ParseDDDMM(
#if defined(GPS_FIX_LOCATION)
        int32_t & val,
#endif
#if defined(GPS_FIX_LOCATION_DMS)
        Dms & dms,
#endif
      char chr
    );

    // Parse integer into 8-bit int
    // @return true when non-empty value

    bool ParseInt(uint8_t &val, uint8_t chr) {
      bool isComma { (chr == ',') };

      _negative = false;

      if (_chrCount == 0) {
        if (isComma)
          return false; // empty field!

        if (((ValidateChars() || ValidateFields()) && (chr == '-')) || (ValidateChars() && !isdigit( chr ))) {
          SentenceInvalid();
        } else {
          val = (chr - '0');
        }

      } else if (!isComma) {
        if (ValidateChars() && !isdigit( chr )) {
          SentenceInvalid();
        } else {
          val = (val * 10) + (chr - '0');
        }
      }
      return true;
    }

    // Parse integer into signed 8-bit int
    // @return true when non-empty value
    bool ParseInt(int8_t &val, uint8_t chr) {
      bool isComma { (chr == ',') };

      if (_chrCount == 0) {
        if (isComma) {
          return false; // empty field!
        }

        _negative = (chr == '-');
        if (_negative) {
          CommaNeeded(true); // to negate
          val = 0;
        } else if (ValidateChars() && !isdigit( chr )) {
          SentenceInvalid();
        } else {
          val = (chr - '0');
        }
      } else if (!isComma) {
        val = (val * 10) + (chr - '0');

      } else if (_negative) {
        val = -val;
      }

      return true;
    }

    // Parse integer into 16-bit int
    // @return true when non-empty value

    bool ParseInt(uint16_t &val, uint8_t chr) {
      bool isComma { (chr == ',') };

      _negative = false;

      if (_chrCount == 0) {
        if (isComma) {
          return false; // empty field!
        }

        if (((ValidateChars() || ValidateFields()) && (chr == '-')) || (ValidateChars() && !isdigit(chr))) {
          SentenceInvalid();
        } else {
          val = (chr - '0');
        }
      } else if (!isComma) {

        if (ValidateChars() && !isdigit(chr)) {
          SentenceInvalid();
        } else {
          val = (val * 10) + (chr - '0');
        }
      }
      return true;
    }

    //.......................................................................
    // Parse integer into 32-bit int
    // @return true when non-empty value
    bool parseInt(uint32_t &val, uint8_t chr) {
      bool isComma { (chr == ',') };

      _negative = false;

      if (_chrCount == 0) {
        if (isComma) {
          return false; // empty field!
        }

        if (((ValidateChars() || ValidateFields()) && (chr == '-')) || (ValidateChars() && !isdigit(chr))) {
          SentenceInvalid();
        } else {
          val = (chr - '0');
        }

      } else if (!isComma) {
        if (ValidateChars() && !isdigit(chr)) {
          SentenceInvalid();
        } else {
          val = (val*10) + (chr - '0');
        }
      }

      return true;
    }

  private:
    void SentenceBegin(void);
    void SentenceOk(void);
    void SentenceInvalid(void);
    void SentenceUnrecognized(void);
    void HeaderReceived(void);

};

} // namespace Airsoft::Neo

#endif  // GPS_NEMA_HPP_
