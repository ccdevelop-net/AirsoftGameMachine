/*
 * timeout.hpp
 *
 *  Created on: Dec 4, 2024
 *      Author: ccroci
 */

#ifndef _TIMEOUT_HPP_
#define _TIMEOUT_HPP_

#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstdbool>
#include <limits>

namespace Airsoft::Drivers {

/**
 *  @brief Structure for setting the timeout of the serial port, times are in milliseconds.
 *         In order to disable the inter-byte timeout, set it to Timeout::Max().
 */
struct Timeout {
  static uint32_t Max() {
    return std::numeric_limits<uint32_t>::max();
  }

  /**
   * @brief Convenience function to generate Timeout struct using a single absolute timeout.
   * @param timeout A long that defines the time in milliseconds until a timeout occurs after a call
   * to read or write is made.
   * @return Timeout struct that represents this simple timeout provided.
   */
  static Timeout SimpleTimeout(uint32_t timeout) {
    return Timeout(Max(), timeout, 0, timeout, 0);
  }

  /**
   * @brief Number of milliseconds between bytes received to timeout on.
   */
  uint32_t InterByteTimeout;

  /**
   * @brief A constant number of milliseconds to wait after calling read.
   */
  uint32_t ReadTimeoutConstant;

  /**
   * @brief A multiplier against the number of requested bytes to wait after calling read.
   */
  uint32_t ReadTimeoutMultiplier;

  /**
   * @brief A constant number of milliseconds to wait after calling write.
   */
  uint32_t WriteTimeoutConstant;

  /**
   * @brief A multiplier against the number of requested bytes to wait after calling write.
   */
  uint32_t WriteTimeoutMultiplier;

  explicit Timeout (uint32_t interByteTimeout = 0, uint32_t readTimeoutConstant = 0,
                    uint32_t readTimeoutMultiplier = 0, uint32_t writeTimeoutConstant = 0,
                    uint32_t writeTimeoutMultiplier = 0) :
                        InterByteTimeout(interByteTimeout),
                        ReadTimeoutConstant(readTimeoutConstant),
                        ReadTimeoutMultiplier(readTimeoutMultiplier),
                        WriteTimeoutConstant(writeTimeoutConstant),
                        WriteTimeoutMultiplier(writeTimeoutMultiplier) {

  }
};


}

#endif // _SERIAL_DRIVER_HPP_
