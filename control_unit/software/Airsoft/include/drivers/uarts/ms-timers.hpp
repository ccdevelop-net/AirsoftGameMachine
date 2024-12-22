/*
 * ms-timers.hpp
 *
 *  Created on: Dec 4, 2024
 *      Author: ccroci
 */

#ifndef _SERIAL_MS_TIMERS_HPP_
#define _SERIAL_MS_TIMERS_HPP_

#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstdbool>
#include <ctime>

namespace Airsoft::Drivers {

class MillisecondTimer {
public:
  MillisecondTimer(const uint32_t millis);
  int64_t Remaining(void);

private:
  static timespec TimespecNow(void);

private:
  timespec expiry;
};

}

#endif // _SERIAL_MS_TIMERS_HPP_
