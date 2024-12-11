#ifndef NEO_TIME_HPP_
#define NEO_TIME_HPP_

#include <cstdint>
#include <cstdbool>
#include <cstdio>
#include <string>
#include <cstring>

#include <gps-config.hpp>

namespace Airsoft::Neo {

//------------------------------------------------------
// Enable/disable run-time modification of the epoch.
// If this is defined, epoch mutators are available.
// If this is not defined, the epoch is a hard-coded constant.
// Only epoch accessors are available.
//#define TIME_EPOCH_MODIFIABLE

/**
 * Number of seconds elapsed since January 1 of the Epoch Year, 00:00:00 +0000 (UTC).
 */
using clock_t = uint32_t;

constexpr uint8_t  SECONDS_PER_MINUTE { 60 };
constexpr uint8_t  MINUTES_PER_HOUR   { 60 };
constexpr uint16_t SECONDS_PER_HOUR   { (uint16_t) SECONDS_PER_MINUTE * MINUTES_PER_HOUR };
constexpr uint8_t  HOURS_PER_DAY      { 24 };
constexpr uint32_t SECONDS_PER_DAY    { (uint32_t) SECONDS_PER_HOUR * HOURS_PER_DAY };
constexpr uint8_t  DAYS_PER_WEEK      { 7 };

/**
 * Common date/time structure
 */
struct TimeT {

  enum class Weekday : uint8_t {
    SUNDAY    = 1,
    MONDAY    = 2,
    TUESDAY   = 3,
    WEDNESDAY = 4,
    THURSDAY  = 5,
    FRIDAY    = 6,
    SATURDAY  = 7
  };

  // NTP epoch year and weekday (Monday)
  static const uint16_t NTP_EPOCH_YEAR      { 1900 };
  static const uint8_t  NTP_EPOCH_WEEKDAY   { (uint8_t)Weekday::MONDAY };

  // POSIX epoch year and weekday (Thursday)
  static const uint16_t POSIX_EPOCH_YEAR    { 1970 };
  static const uint8_t  POSIX_EPOCH_WEEKDAY { (uint8_t)Weekday::THURSDAY };

  // Y2K epoch year and weekday (Saturday)
  static const uint16_t Y2K_EPOCH_YEAR      { 2000 };
  static const uint8_t  Y2K_EPOCH_WEEKDAY   { (uint8_t)Weekday::SATURDAY };

  uint8_t Seconds {};    //!< 00-59
  uint8_t Minutes {};    //!< 00-59
  uint8_t Hours {};      //!< 00-23
  uint8_t Day {};        //!< 01-07 Day of Week
  uint8_t Date {};       //!< 01-31 Day of Month
  uint8_t Month {};      //!< 01-12
  uint8_t Year {};       //!< 00-99

  /**
   * Constructor.
   */
  TimeT(void) {}

  /**
   * @brief Construct from seconds since the Epoch.
   * @param c - Clock.
   */
  TimeT(clock_t c);

  /**
   * @brief Initialize to January 1 of the Epoch Year, 00:00:00
   */
  void Init(void);

  /**
   * @brief Convert to seconds.
   * @return seconds from epoch.
   */
  operator clock_t() const;

  /**
   * @brief Offset by a number of seconds.
   * @param offset - Offset in seconds.
   */
  void operator +=(clock_t offset) {
    *this = offset + operator clock_t();
  }

  /**
   * Set day member from current value.  This is a relatively expensive
   * operation, so the weekday is only calculated when requested.
   */
  void SetDay(void) {
    Day = WeekdayFor(Days());
  }

  /**
   * @brief Convert to days.
   * @return Days from January 1 of the epoch year.
   */
  uint16_t Days(void) const;

  /**
   * @brief Calculate day of the current year.
   * @return days from January 1, which is day zero.
   */
  uint16_t DayOfYear(void) const;

  /**
   * @brief Calculate 4-digit year from internal 2-digit year member.
   * @return 4-digit year.
   */
  uint16_t FullYear(void) const {
    return FullYear(Year);
  }

  /**
   * @brief Calculate 4-digit year from a 2-digit year
   * @param year - year (4-digit).
   * @return True if year is a leap year.
   */
  static uint16_t FullYear(uint8_t year) {
    uint16_t y { year };

    if (y < PivotYear()) {
      y += 100 * (EpochYear() / 100 + 1);
    } else {
      y += 100 * (EpochYear() / 100);
    }

    return y;
  }

  /**
   * @brief Determine whether the current year is a leap year.
   * @returns True if the two-digit /year/ member is a leap year.
   */
  bool IsLeap(void) const {
    return IsLeap(FullYear());
  }

  /**
   * @brief Determine whether the 4-digit /year/ is a leap year.
   * @param year - Year (4-digit).
   * @return true if year is a leap year.
   */
  static bool IsLeap(uint16_t year) {
    if (year % 4) {
      return false;
    }

    uint16_t y { static_cast<uint16_t>(year % 400) };

    return (y == 0) || ((y != 100) && (y != 200) && (y != 300));
  }

  /**
   * @brief Calculate how many days are in the specified year.
   * @param year - Year (4-digit).
   * @return Number of days.
   */
  static uint16_t DaysPer(uint16_t year) {
    return (365 + IsLeap(year));
  }

  /**
   * @brief Determine the day of the week for the specified day number
   * @param dayno - Number as counted from January 1 of the epoch year.
   * @return Weekday number 1..7, as for the /day/ member.
   */
  static uint8_t WeekdayFor(uint16_t dayno) {
    return ((dayno + EpochWeekday() - 1) % DAYS_PER_WEEK) + 1;
  }

  /**
   * @brief Check that all members, EXCEPT FOR day, are set to a coherent date/time.
   * @return True if valid date/time.
   */
  bool IsValid(void) const {
    return ((Year <= 99) && (1 <= Month) && (Month <= 12) && ((1 <= Date) && ((Date <= DaysIn[Month]) ||
           ((Month == 2) && IsLeap() && (Date == 29)))) && (Hours   <= 23) && (Minutes <= 59) && (Seconds <= 59)         );
  }

  /**
   * @brief Set the epoch year for all time_t operations. Note that the pivot year defaults to the epoch_year % 100.
   *        Valid years will be in the range epoch_year..epoch_year+99. Selecting a different pivot year will slide this
   *        range to the right.
   * @param y - Epoch year to set.
   * See also FullYear.
   */
#ifdef TIME_EPOCH_MODIFIABLE
  static void EpochYear(uint16_t y) {
    _epochYear = y;
    EpochOffset(_epochYear % 100);
    PivotYear(EpochOffset());
  }
#endif

  /**
   * @brief Get the epoch year.
   * @return year.
   */
  static uint16_t EpochYear(void) {
    return _epochYear;
  }

  static uint8_t EpochWeekday(void) {
    return _epochWeekday;
  }
#ifdef TIME_EPOCH_MODIFIABLE
  static void EpochWeekday(uint8_t ew) {
    _epochWeekday = ew;
  }
#endif

  /**
   * The pivot year determine the range of years WRT the epoch_year
   * For example, an epoch year of 2000 and a pivot year of 80 will
   * allow years in the range 1980 to 2079. Default 0 for Y2K_EPOCH.
   */
  static uint8_t PivotYear()             {
    return _pivotYear;
  }
#ifdef TIME_EPOCH_MODIFIABLE
  static void PivotYear(uint8_t py) {
    _pivotYear = py;
  }
#endif

#ifdef TIME_EPOCH_MODIFIABLE
  /**
   * @brief Use the current year for the epoch year. This will result in the best performance of conversions,
   *        but dates/times before January 1 of the epoch year cannot be represented.
   */
  static void UseFastestEpoch(void);
#endif

  /**
   * @brief Parse a character string and fill out members.
   * @param s - String with format "YYYY-MM-DD HH:MM:SS".
   * @return success.
   */
  bool Parse(std::string s);

  static const uint8_t DaysIn[];

protected:
  static uint8_t EpochOffset(void) {
    return _epochOffset;
  };

#ifdef TIME_EPOCH_MODIFIABLE
  static void     EpochOffset(uint8_t eo) {
    _epochOffset = eo;
  }

  static uint16_t       _epochYear;
  static uint8_t        _pivotYear;
  static uint8_t        _epochOffset;
  static uint8_t        _epochWeekday;
#else
  static const uint16_t _epochYear    = Y2K_EPOCH_YEAR;
  static const uint8_t  _pivotYear    = _epochYear % 100;
  static const uint8_t  _epochOffset  = _pivotYear;
  static const uint8_t  _epochWeekday = Y2K_EPOCH_WEEKDAY;
#endif

};

} // namespace Airsoft::Neo

class Print;

/**
 * Print the date/time to the given stream with the format "YYYY-MM-DD HH:MM:SS".
 * @param[in] outs output stream.
 * @param[in] t time structure.
 * @return iostream.
 */
Print & operator <<( Print & outs, const Airsoft::Neo::TimeT &t );

#endif  // NEO_TIME_HPP_
