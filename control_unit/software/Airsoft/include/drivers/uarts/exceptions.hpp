/*
 * exceptions.hpp
 *
 *  Created on: Dec 4, 2024
 *      Author: ccroci
 */

#ifndef _SERIAL_EXCEPTIONS_HPP_
#define _SERIAL_EXCEPTIONS_HPP_

#include <drivers/uarts/enumerators.hpp>
#include <drivers/uarts/timeout.hpp>
#include <string>
#include <cstring>
#include <sstream>
#include <exception>
#include <stdexcept>

namespace Airsoft::Drivers {

class UartException : public std::exception {
private:
  // Disable copy constructors
  UartException& operator=(const UartException&);
  std::string _e_what;

public:
  UartException (const char *description) {
    std::stringstream ss;
    ss << "UartException " << description << " failed.";
    _e_what = ss.str();
  }

  UartException (const UartException & other) : _e_what(other._e_what) {}
  virtual ~UartException() throw() {}

  virtual const char* what(void) const throw () {
    return _e_what.c_str();
  }
};

class IOException : public std::exception {
private:
  // Disable copy constructors
  IOException& operator=(const IOException&);
  std::string   _file;
  int32_t       _line;
  std::string   _e_what;
  int32_t       _errno;

public:
  explicit IOException (std::string file, int32_t line, int32_t errnum) : _file(file), _line(line), _errno(errnum) {
    std::stringstream ss;
    char * error_str = strerror(errnum);
    ss << "IO Exception (" << _errno << "): " << error_str;
    ss << ", file " << _file << ", line " << _line << ".";
    _e_what = ss.str();
  }
  explicit IOException (std::string file, int32_t line, const char * description) : _file(file), _line(line), _errno(0) {
    std::stringstream ss;
    ss << "IO Exception: " << description;
    ss << ", file " << _file << ", line " << _line << ".";
    _e_what = ss.str();
  }
  virtual ~IOException() throw() {}
  IOException (const IOException& other) : _line(other._line), _e_what(other._e_what), _errno(other._errno) {}

  int32_t GetErrorNumber(void) const { return _errno; }

  virtual const char* what(void) const throw () {
    return _e_what.c_str();
  }
};

class PortNotOpenedException : public std::exception {
  // Disable copy constructors
  const PortNotOpenedException& operator=(PortNotOpenedException);
  std::string _e_what;
public:
  PortNotOpenedException (const char * description)  {
    std::stringstream ss;
    ss << "PortNotOpenedException " << description << " failed.";
    _e_what = ss.str();
  }
  PortNotOpenedException (const PortNotOpenedException& other) : _e_what(other._e_what) {};
  virtual ~PortNotOpenedException() throw() {}
  virtual const char* what(void) const throw () {
    return _e_what.c_str();
  }
};

} // namespace Airsoft::Drivers


#endif // _SERIAL_EXCEPTIONS_HPP_
