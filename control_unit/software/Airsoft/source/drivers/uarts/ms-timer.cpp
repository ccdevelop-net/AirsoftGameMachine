/*
 * ms-timer.cpp
 *
 *  Created on: Dec 5, 2024
 *      Author: ccroci
 */
#include <drivers/uarts/ms-timers.hpp>

namespace Airsoft::Drivers {

//-----------------------------------------------------------------------------
MillisecondTimer::MillisecondTimer (const uint32_t millis) : expiry(TimespecNow()) {
  int64_t tv_nsec = expiry.tv_nsec + (millis * 1e6);
  if (tv_nsec >= 1e9) {
    int64_t sec_diff = tv_nsec / static_cast<int32_t> (1e9);
    expiry.tv_nsec = tv_nsec % static_cast<int32_t>(1e9);
    expiry.tv_sec += sec_diff;
  } else {
    expiry.tv_nsec = tv_nsec;
  }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
int64_t MillisecondTimer::Remaining(void)
{
  timespec now(TimespecNow());
  int64_t millis = (expiry.tv_sec - now.tv_sec) * 1e3;
  millis += (expiry.tv_nsec - now.tv_nsec) / 1e6;
  return millis;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
timespec MillisecondTimer::TimespecNow(void) {
  timespec time;
  clock_gettime(CLOCK_MONOTONIC, &time);
  return time;
}
//-----------------------------------------------------------------------------

} // namespace Airsoft::Drivers
