/*
 * enumerators.hpp
 *
 *  Created on: Dec 4, 2024
 *      Author: ccroci
 */

#ifndef _ENUMERATORS_HPP_
#define _ENUMERATORS_HPP_

namespace Airsoft::Drivers {

/**
 * @brief Enumeration defines the possible byte sizes for the serial port.
 */
enum class ByteSize {
  Five = 5,
  Six = 6,
  Seven = 7,
  Eight = 8
};

/**
 * @brief Enumeration defines the possible parity types for the serial port.
 */
enum class Parity {
  None = 0,
  Odd = 1,
  Even = 2,
  Mark = 3,
  Space = 4
};

/**
 * @brief Enumeration defines the possible stop bit types for the serial port.
 */
enum class StopBits {
  One = 1,
  Two = 2,
  OnePointFive
};

/**
 * @brief Enumeration defines the possible flow control types for the serial port.
 */
enum class FlowControl {
  None = 0,
  Software,
  Hardware
};

}

#endif // _ENUMERATORS_HPP_
