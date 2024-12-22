/**
 *******************************************************************************
 * @file print.cpp
 *
 * @brief Description
 *
 * @author  Cristian
 *
 * @version 1.00
 *
 * @date Dec 9, 2024
 *
 *******************************************************************************
 * This file is part of the Airsoft project 
 * https://github.com/xxxx or http://xxx.github.io.
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

#include <cmath>
#include <classes/print.hpp>

namespace Airsoft::Classes {

Print::Print() {

}

Print::~Print() {

}

//------------------------------------------------------------------------------
size_t Print::print(const std::string &s) {
  return Write(s.c_str(), s.length());
}
//------------------------------------------------------------------------------
size_t Print::print(const char str[]) {
  return Write(str);
}
//------------------------------------------------------------------------------
size_t Print::print(char c) {
  return Write(c);
}
//------------------------------------------------------------------------------
size_t Print::print(uint8_t b, int base) {
  return print((uint64_t) b, base);
}
//------------------------------------------------------------------------------
size_t Print::print(int32_t n, int base) {
  return print((int64_t)n, base);
}
//------------------------------------------------------------------------------
size_t Print::print(unsigned int n, int base) {
  return print((uint64_t)n, base);
}
//------------------------------------------------------------------------------
size_t Print::print(int64_t n, int base)
{
  if (base == 0) {
    return Write(n);
  } else if (base == 10) {
    if (n < 0) {
      int t = print('-');
      n = -n;
      return PrintNumber(n, 10) + t;
    }
    return PrintNumber(n, 10);
  } else {
    return PrintNumber(n, base);
  }
}
//------------------------------------------------------------------------------
size_t Print::print(uint64_t n, int base)
{
  if (base == 0) {
    return Write(n);
  } else {
    return PrintNumber(n, base);
  }
}
//------------------------------------------------------------------------------
size_t Print::print(double n, int32_t digits) {
  return PrintFloat(n, digits);
}
//------------------------------------------------------------------------------
size_t Print::println(void) {
  return Write("\r\n");
}
//------------------------------------------------------------------------------
size_t Print::println(const std::string &s) {
  size_t n = print(s);

  n += println();

  return n;
}
//------------------------------------------------------------------------------
size_t Print::println(const char c[]) {
  size_t n = print(c);

  n += println();

  return n;
}
//------------------------------------------------------------------------------
size_t Print::println(char c) {
  size_t n = print(c);

  n += println();

  return n;
}
//------------------------------------------------------------------------------
size_t Print::println(unsigned char b, int base) {
  size_t n = print(b, base);

  n += println();

  return n;
}
//------------------------------------------------------------------------------
size_t Print::println(int32_t num, int base) {
  size_t n = print(num, base);

  n += println();

  return n;
}
//------------------------------------------------------------------------------
size_t Print::println(uint32_t num, int base) {
  size_t n = print(num, base);

  n += println();

  return n;
}
//------------------------------------------------------------------------------
size_t Print::println(int64_t num, int base) {
  size_t n = print(num, base);

  n += println();

  return n;
}
//------------------------------------------------------------------------------
size_t Print::println(uint64_t num, int base) {
  size_t n = print(num, base);

  n += println();

  return n;
}
//------------------------------------------------------------------------------
size_t Print::println(double num, int digits) {
  size_t n = print(num, digits);

  n += println();

  return n;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
size_t Print::PrintNumber(uint64_t n, uint8_t base) {
  char buf[8 * sizeof(long) + 1]; // Assumes 8-bit chars plus zero byte.
  char *str = &buf[sizeof(buf) - 1];

  *str = '\0';

  // prevent crash if called with base == 1
  if (base < 2) {
    base = 10;
  }

  do {
    uint64_t c { n % base };
    n /= base;

    *--str = c < 10 ? c + '0' : c + 'A' - 10;
  } while(n);

  return Write(str);
}
//------------------------------------------------------------------------------
size_t Print::PrintFloat(double number, uint8_t digits) {
  size_t n = 0;

  if (std::isnan(number)) {
    return print("nan");
  }
  if (std::isinf(number)) {
    return print("inf");
  }
  if (number > 4294967040.0) {
    return print ("ovf");  // constant determined empirically
  }
  if (number <-4294967040.0) {
    return print ("ovf");  // constant determined empirically
  }

  // Handle negative numbers
  if (number < 0.0) {
     n += print('-');
     number = -number;
  }

  // Round correctly so that print(1.999, 2) prints as "2.00"
  double rounding { 0.5 };
  for (uint8_t i {}; i < digits; ++i) {
    rounding /= 10.0;
  }

  number += rounding;

  // Extract the integer part of the number and print it
  uint64_t int_part { (uint64_t)number };
  double remainder { number - (double)int_part };

  n += print(int_part);

  // Print the decimal point, but only if there are digits beyond
  if (digits > 0) {
    n += print('.');
  }

  // Extract digits from the remainder one at a time
  while (digits-- > 0) {
    remainder *= 10.0;
    uint32_t toPrint = (uint32_t)(remainder);
    n += print(toPrint);
    remainder -= toPrint;
  }

  return n;
}
//------------------------------------------------------------------------------

} // namespace Airsoft::Classes
