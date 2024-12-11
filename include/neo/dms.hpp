#ifndef DMS_H
#define DMS_H

#include <cstdint>
#include <cstdbool>
#include <cstdio>
#include <string>
#include <cstring>

#include <gps-config.hpp>

#include <classes/print.hpp>

namespace Airsoft::Neo {

enum class Hemisphere : uint8_t {
  NORTH_H = 0,
  SOUTH_H = 1,
  EAST_H = 0,
  WEST_H = 1
};

class Dms {
public:
  uint8_t    degrees {};
  uint8_t    minutes    {};
  Hemisphere hemisphere { Hemisphere::NORTH_H };
  uint8_t    secondsWhole {};
  uint16_t   secondsFrac  {}; // 1000ths

  float SecondsF(void) const {
    return secondsWhole + 0.001 * secondsFrac;
  }
  char NS(void) const {
    return (hemisphere == Hemisphere::SOUTH_H) ? 'S' : 'N';
  }
  char EW(void) const {
    return (hemisphere ==  Hemisphere::WEST_H) ? 'W' : 'E';
  }

  /**
   * @brief Function to convert from integer 'lat' or 'lon', scaled by 10^7
   * @param deg_1E7 - Coordinate in degree to convert
   */
  void From(int32_t deg_1E7);

  /**
   * @brief Print DMS as the funky NMEA DDDMM.mmmm format
   * @param outs - Stream out
   */
  void PrintDDDMMmmmm(Airsoft::Classes::Print & outs ) const;

};

extern Airsoft::Classes::Print & operator << (Airsoft::Classes::Print & outs, const Dms &);

}

#endif
