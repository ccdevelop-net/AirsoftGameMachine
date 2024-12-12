/**
 *******************************************************************************
 * @file uarts.cpp
 *
 * @brief Driver UARTS for Luckfox Pico boards, implementation
 *
 * @author  Cristian Croci
 *
 * @version 1.00
 *
 * @date Nov 27, 2024
 *
 *******************************************************************************
 * This file is part of the Airsoft Game Machine project
 * https://github.com/ccdevelop-net/AirsoftGameMachine
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
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <errno.h>
#include <paths.h>
#include <sysexits.h>
#include <termios.h>
#include <sys/param.h>
#include <pthread.h>
#include <drivers/uarts/ms-timers.hpp>
#include <drivers/uarts.hpp>
#include <sys/ioctl.h>
#include <linux/serial.h>

#include <sys/select.h>
#include <sys/time.h>
#include <time.h>

#include <memory>

#ifndef TIOCINQ
#ifdef FIONREAD
#define TIOCINQ FIONREAD
#else
#define TIOCINQ 0x541B
#endif
#endif

namespace Airsoft::Drivers {

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Time conversion function

//-----------------------------------------------------------------------------
static timespec timespec_from_ms (const uint32_t millis) {
  timespec time;
  time.tv_sec = millis / 1e3;
  time.tv_nsec = (millis - (time.tv_sec * 1e3)) * 1e6;
  return time;
}
//-----------------------------------------------------------------------------

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Class ScopeReadLock for uart

//-----------------------------------------------------------------------------
class ScopedReadLock final {
public:
  ScopedReadLock(Uarts * serial) : _serial(serial) {
    _serial->ReadLock();
  }
  ~ScopedReadLock() {
    _serial->ReadUnlock();
  }
private:
  // Disable copy constructors
  ScopedReadLock(const ScopedReadLock&);
  const ScopedReadLock& operator=(ScopedReadLock);

  Uarts * _serial {};
};
//-----------------------------------------------------------------------------

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Class ScopeWriteLock for uart

//-----------------------------------------------------------------------------
class ScopedWriteLock {
public:
  ScopedWriteLock(Uarts * serial) : _serial(serial) {
    _serial->WriteLock();
  }
  ~ScopedWriteLock() {
    _serial->WriteUnlock();
  }
private:
  // Disable copy constructors
  ScopedWriteLock(const ScopedWriteLock&);
  const ScopedWriteLock& operator=(ScopedWriteLock);

private:
  Uarts * _serial {};
};
//-----------------------------------------------------------------------------

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Constructor / Destructor Uarts

//-----------------------------------------------------------------------------
Uarts::Uarts (const std::string &port, uint32_t baudrate, Timeout timeout, ByteSize bytesize, Parity parity,
                StopBits stopbits, FlowControl flowcontrol) {
  _port = port;
  SetTimeout(timeout);
}
//-----------------------------------------------------------------------------

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Public Functions

//-----------------------------------------------------------------------------
void Uarts::Open(void) {
  // Check if valid port
  if (_port.empty ()) {
    throw std::invalid_argument ("Empty port is invalid.");
  }

  // If the port is already open, throw
  if (_is_open) {
    throw UartException ("Serial port already open.");
  }

  // Open serial port
  _fd = ::open (_port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);

  // Check if port opened
  if (_fd == -1) {
    switch (errno) {
      case EINTR:
        // Recurse because this is a recoverable error.
        Open ();
        return;
      case ENFILE:
      case EMFILE:
        THROW (IOException, "Too many file handles open.");
      default:
        THROW (IOException, errno);
    }
  }

  // Reconfigure serial port
  ReconfigurePort();

  // Set flag
  _is_open = true;
}
//-----------------------------------------------------------------------------
void Uarts::Close(void) {
  if (_is_open) {
    if (_fd != -1) {
      if (::close(_fd) == 0) {
        _fd = -1;
      } else {
        THROW (IOException, errno);
      }
    }

    _is_open = false;
  }
}
//-----------------------------------------------------------------------------
bool Uarts::IsOpen(void) const {
  return _is_open;
}
//-----------------------------------------------------------------------------
size_t Uarts::Available(void) {
  // If the port is not open, throw
  if (!_is_open) {
    return 0;
  }

  // Function Variables
  int32_t count {};

  if (ioctl(_fd, TIOCINQ, &count) == -1) {
    THROW (IOException, errno);
  }

  return static_cast<size_t>(count);
}
//-----------------------------------------------------------------------------
bool Uarts::WaitReadable(void) {
  // Setup a select call to block for serial data or a timeout
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(_fd, &readfds);

  timespec timeout_ts (timespec_from_ms (_timeout.ReadTimeoutConstant));
  int32_t r { pselect (_fd + 1, &readfds, nullptr, nullptr, &timeout_ts, nullptr) };

  if (r < 0) {
    // Select was interrupted
    if (errno == EINTR) {
      return false;
    }
    // Otherwise there was some error
    THROW (IOException, errno);
  }

  // Timeout occurred
  if (r == 0) {
    return false;
  }

  // This shouldn't happen, if r > 0 our fd has to be in the list!
  if (!FD_ISSET (_fd, &readfds)) {
    THROW (IOException, "select reports ready to read, but our fd isn't in the list, this shouldn't happen!");
  }

  // Data available to read.
  return true;
}
//-----------------------------------------------------------------------------
void Uarts::WaitByteTimes (size_t count) {
  timespec wait_time;
  wait_time.tv_sec = 0;
  wait_time.tv_nsec = static_cast<int64_t>(_byteTime_ns * count);
  pselect (0, nullptr, nullptr, nullptr, &wait_time, nullptr);
}
//-----------------------------------------------------------------------------
size_t Uarts::Read (uint8_t * buffer, size_t size) {
  ScopedReadLock(this);
  return _Read (buffer, size);
}
//-----------------------------------------------------------------------------
size_t Uarts::Read(std::vector<uint8_t> & buffer, size_t size) {
  // Read lock
  ScopedReadLock(this);

  // Function Variables
  std::unique_ptr<uint8_t>  bufferRD(new uint8_t(size));
  size_t                    bytesRead {};

  try {
    bytesRead = _Read(bufferRD.get(), size);
  } catch (const std::exception &e) {
    throw;
  }

  buffer.insert(buffer.end (), bufferRD.get(), bufferRD.get() + bytesRead);

  return bytesRead;
}
//-----------------------------------------------------------------------------
size_t Uarts::Read(std::string & buffer, size_t size) {
  // Lock mutex
  ScopedReadLock(this);

  // Function Variables
  std::unique_ptr<uint8_t> bufferRD(new uint8_t(size));
  size_t bytesRead {};

  try {
    bytesRead = _Read(bufferRD.get(), size);
  } catch (const std::exception &e) {
    throw;
  }

  buffer.append(reinterpret_cast<const char*>(bufferRD.get()), bytesRead);

  return bytesRead;
}
//-----------------------------------------------------------------------------
std::string Uarts::Read(size_t size) {
  // Function Variables
  std::string buffer;

  Read (buffer, size);

  return buffer;
}
//-----------------------------------------------------------------------------
size_t Uarts::ReadLine(std::string & buffer, size_t size, std::string eol) {
  // Read lock
  ScopedReadLock(this);

  // Function Variables
  size_t    eolLen { eol.length () };
  uint8_t * bufferRead = static_cast<uint8_t*> (alloca(size * sizeof(uint8_t)));
  size_t    readSoFar {};

  while (true) {
    size_t bytes_read { _Read(bufferRead + readSoFar, 1) };

    readSoFar += bytes_read;

    if (bytes_read == 0) {
      break; // Timeout occurred on reading 1 byte
    }

    if (readSoFar < eolLen) {
      continue;
    }

    if (std::string(reinterpret_cast<const char*> (bufferRead + readSoFar - eolLen), eolLen) == eol) {
      break; // EOL found
    }

    if (readSoFar == size) {
      break; // Reached the maximum read length
    }
  }

  buffer.append(reinterpret_cast<const char*>(bufferRead), readSoFar);

  return readSoFar;
}
//-----------------------------------------------------------------------------
std::string Uarts::ReadLine(size_t size, std::string eol) {
  // Function Variables
  std::string buffer;

  ReadLine (buffer, size, eol);

  return buffer;
}
//-----------------------------------------------------------------------------
std::vector<std::string> Uarts::ReadLines(size_t size, std::string eol) {
  // Lock mutex
  ScopedReadLock(this);

  // Function Variables
  std::vector<std::string>  lines;
  size_t                    eolLen { eol.length () };
  uint8_t       *           buffer { static_cast<uint8_t*>(alloca(size * sizeof(uint8_t))) };
  size_t                    readSoFar {};
  size_t                    startOfLine {};

  while (readSoFar < size) {
    size_t bytes_read { _Read(buffer + readSoFar, 1) };
    readSoFar += bytes_read;
    if (bytes_read == 0) {
      if (startOfLine != readSoFar) {
        lines.push_back (std::string(reinterpret_cast<const char*>(buffer + startOfLine), readSoFar - startOfLine));
      }

      break; // Timeout occurred on reading 1 byte
    }

    if (readSoFar < eolLen) {
      continue;
    }

    if (std::string (reinterpret_cast<const char*>(buffer + readSoFar - eolLen), eolLen) == eol) {
      // EOL found
      lines.push_back (std::string (reinterpret_cast<const char*> (buffer + startOfLine), readSoFar - startOfLine));
      startOfLine = readSoFar;
    }

    if (readSoFar == size) {
      if (startOfLine != readSoFar) {
        lines.push_back (std::string (reinterpret_cast<const char*> (buffer + startOfLine), readSoFar - startOfLine));
      }
      break; // Reached the maximum read length
    }
  }

  return lines;
}
//-----------------------------------------------------------------------------
size_t Uarts::Write(const std::string &data) {
  // Lock mutex
  ScopedWriteLock(this);
  return _Write(reinterpret_cast<const uint8_t*> (data.c_str ()), data.length ());
}
//-----------------------------------------------------------------------------
size_t Uarts::Write (const std::vector<uint8_t> & data) {
  // Lock mutex
  ScopedWriteLock(this);
  return _Write(&data[0], data.size ());
}
//-----------------------------------------------------------------------------
size_t Uarts::Write (const uint8_t * data, size_t size) {
  // Lock mutex
  ScopedWriteLock(this);
  return _Write(data, size);
}
//-----------------------------------------------------------------------------
void Uarts::SetPort (const std::string & port) {
  // Lock mutex
  ScopedReadLock(this);
  ScopedWriteLock(this);

  // Function Variables
  bool was_open { _is_open };

  if (was_open) {
    Close();
  }
  _port = port;

  if (was_open) {
    Open();
  }
}
//-----------------------------------------------------------------------------
std::string Uarts::GetPort(void) const {
  return _port;
}
//-----------------------------------------------------------------------------
void Uarts::SetTimeout(Timeout & timeout) {
  _timeout = timeout;
}
//-----------------------------------------------------------------------------
Timeout Uarts::GetTimeout(void) const {
  return _timeout;
}
//-----------------------------------------------------------------------------
void Uarts::SetBaudrate(uint32_t baudrate) {
  _baudrate = baudrate;

  // Check if is open
  if (_is_open) {
    ReconfigurePort();
  }
}
//-----------------------------------------------------------------------------
uint32_t Uarts::GetBaudrate(void) const {
  return uint32_t(_baudrate);
}
//-----------------------------------------------------------------------------
void Uarts::SetBytesize (ByteSize bytesize) {
  _bytesize = bytesize;

  // Check if is open
  if (_is_open) {
    ReconfigurePort();
  }
}
//-----------------------------------------------------------------------------
ByteSize Uarts::GetBytesize(void) const {
  return _bytesize;
}
//-----------------------------------------------------------------------------
void Uarts::SetParity (Parity parity) {
  _parity = parity;

  // Check if is open
  if (_is_open) {
    ReconfigurePort();
  }
}
//-----------------------------------------------------------------------------
Parity Uarts::GetParity(void) const {
  return _parity;
}
//-----------------------------------------------------------------------------
void Uarts::SetStopbits (StopBits stopbits) {
  _stopbits = stopbits;

  // Check if is open
  if (_is_open) {
    ReconfigurePort();
  }
}
//-----------------------------------------------------------------------------
StopBits Uarts::GetStopbits(void) const {
  return _stopbits;
}
//-----------------------------------------------------------------------------
void Uarts::SetFlowcontrol(FlowControl flowcontrol) {
  _flowcontrol = flowcontrol;

  // Check if is open
  if (_is_open) {
    ReconfigurePort();
  }
}
//-----------------------------------------------------------------------------
FlowControl Uarts::GetFlowcontrol(void) const {
  return _flowcontrol;
}
//-----------------------------------------------------------------------------
void Uarts::Flush(void) {
  // If the port is not open, throw
  if (!_is_open) {
    throw PortNotOpenedException ("Uarts::Flush");
  }

  ReadLock();
  WriteLock();
  tcdrain(_fd);
  WriteUnlock();
  ReadUnlock();
}
//-----------------------------------------------------------------------------
void Uarts::FlushInput(void) {
  // If the port is not open, throw
  if (!_is_open) {
    throw PortNotOpenedException ("Uarts::FlushInput");
  }

  ReadLock();
  tcflush(_fd, TCIFLUSH);
  ReadUnlock();
}
//-----------------------------------------------------------------------------
void Uarts::FlushOutput(void) {
  // If the port is not open, throw
  if (!_is_open) {
    throw PortNotOpenedException ("Uarts::FlushOutput");
  }

  WriteLock();
  tcflush(_fd, TCOFLUSH);
  WriteUnlock();
}
//-----------------------------------------------------------------------------
void Uarts::SendBreak(int32_t duration) {
  // If the port is not open, throw
  if (!_is_open) {
    throw PortNotOpenedException ("Uarts::SendBreak");
  }

  tcsendbreak(_fd, static_cast<int>(duration / 4));
}
//-----------------------------------------------------------------------------
void Uarts::SetBreak(bool level) {
  // If the port is not open, throw
  if (!_is_open) {
    throw PortNotOpenedException ("Uarts::SetBreak");
  }

  // Check level
  if (level) {
    if (ioctl(_fd, TIOCSBRK) == -1) {
      std::stringstream ss;
      ss << "setBreak failed on a call to ioctl(TIOCSBRK): " << errno << " " << strerror(errno);
      throw(UartException(ss.str().c_str()));
    }
  } else {
    if (ioctl(_fd, TIOCCBRK) == -1) {
      std::stringstream ss;
      ss << "setBreak failed on a call to ioctl(TIOCCBRK): " << errno << " " << strerror(errno);
      throw(UartException(ss.str().c_str()));
    }
  }
}
//-----------------------------------------------------------------------------
void Uarts::SetRTS (bool level) {
  // If the port is not open, throw
  if (!_is_open) {
    throw PortNotOpenedException ("Uarts::SetRTS");
  }

  // Function Variables
  int32_t command { TIOCM_RTS };

  // Check level
  if (level) {
    if (ioctl(_fd, TIOCMBIS, &command) == -1) {
      std::stringstream ss;
      ss << "SetRTS failed on a call to ioctl(TIOCMBIS): " << errno << " " << strerror(errno);
      throw(UartException(ss.str().c_str()));
    }
  } else {
    if (ioctl(_fd, TIOCMBIC, &command) == -1) {
      std::stringstream ss;
      ss << "SetRTS failed on a call to ioctl(TIOCMBIC): " << errno << " " << strerror(errno);
      throw(UartException(ss.str().c_str()));
    }
  }
}
//-----------------------------------------------------------------------------
void Uarts::SetDTR(bool level) {
  // If the port is not open, throw
  if (!_is_open) {
    throw PortNotOpenedException ("Uarts::SetDTR");
  }

  // Function Variables
  int32_t command { TIOCM_DTR };

  // Check level
  if (level) {
    if (ioctl(_fd, TIOCMBIS, &command) == -1) {
      std::stringstream ss;
      ss << "SetDTR failed on a call to ioctl(TIOCMBIS): " << errno << " " << strerror(errno);
      throw(UartException(ss.str().c_str()));
    }
  } else {
    if (ioctl(_fd, TIOCMBIC, &command) == -1) {
      std::stringstream ss;
      ss << "SetDTR failed on a call to ioctl(TIOCMBIC): " << errno << " " << strerror(errno);
      throw(UartException(ss.str().c_str()));
    }
  }
}
//-----------------------------------------------------------------------------
bool Uarts::WaitForChange(void) {
#ifndef TIOCMIWAIT
  while (_is_open) {
    int32_t status {};

    if (ioctl (_fd, TIOCMGET, &status) == -1) {
      std::stringstream ss;
      ss << "waitForChange failed on a call to ioctl(TIOCMGET): " << errno << " " << strerror(errno);
      throw(SerialException(ss.str().c_str()));
    } else {
      if ((status & TIOCM_CTS) || (status & TIOCM_DSR) || (status & TIOCM_RI) || (status & TIOCM_CD)) {
        return true;
      }
    }

    usleep(1000);
  }

  return false;
#else
  int32_t command = (TIOCM_CD | TIOCM_DSR | TIOCM_RI | TIOCM_CTS);

  if (ioctl(_fd, TIOCMIWAIT, &command) == -1) {
    std::stringstream ss;
    ss << "WaitForDSR failed on a call to ioctl(TIOCMIWAIT): " << errno << " " << strerror(errno);
    throw(UartException(ss.str().c_str()));
  }

  return true;
#endif
}
//-----------------------------------------------------------------------------
bool Uarts::GetCTS(void) {
  // If the port is not open, throw
  if (!_is_open) {
    throw PortNotOpenedException ("Uarts::getCTS");
  }

  // Function Variables
  int32_t status {};

  if (ioctl(_fd, TIOCMGET, &status) == -1) {
    std::stringstream ss;
    ss << "GetCTS failed on a call to ioctl(TIOCMGET): " << errno << " " << strerror(errno);
    throw(UartException(ss.str().c_str()));
  }

  return (status & TIOCM_CTS);
}
//-----------------------------------------------------------------------------
bool Uarts::GetDSR(void) {
  // If the port is not open, throw
  if (!_is_open) {
    throw PortNotOpenedException ("Uarts::GetDSR");
  }

  // Function Variables
  int32_t status {};

  if (ioctl(_fd, TIOCMGET, &status) == -1) {
    std::stringstream ss;
    ss << "GetDSR failed on a call to ioctl(TIOCMGET): " << errno << " " << strerror(errno);
    throw(UartException(ss.str().c_str()));
  }

  return (status & TIOCM_DSR);
}
//-----------------------------------------------------------------------------
bool Uarts::GetRI(void) {
  // If the port is not open, throw
  if (!_is_open) {
    throw PortNotOpenedException ("Uarts::GetRI");
  }

  // Function Variables
  int32_t status {};

  if (ioctl(_fd, TIOCMGET, &status) == -1) {
    std::stringstream ss;
    ss << "GetRI failed on a call to ioctl(TIOCMGET): " << errno << " " << strerror(errno);
    throw(UartException(ss.str().c_str()));
  }

  return (status & TIOCM_RI);
}
//-----------------------------------------------------------------------------
bool Uarts::GetCD(void) {
  // If the port is not open, throw
  if (!_is_open) {
    throw PortNotOpenedException ("Uarts::GetCD");
  }

  // Function Variables
  int32_t status {};

  if (ioctl(_fd, TIOCMGET, &status) == -1) {
    std::stringstream ss;
    ss << "GetCD failed on a call to ioctl(TIOCMGET): " << errno << " " << strerror(errno);
    throw(UartException(ss.str().c_str()));
  }

  return (status & TIOCM_CD);
}
//-----------------------------------------------------------------------------

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Private Function

//-----------------------------------------------------------------------------
void Uarts::ReconfigurePort(void) {
  if (_fd == -1) {
    // Can only operate on a valid file descriptor
    THROW (IOException, "Invalid file descriptor, is the serial port open?");
  }

  // Function Variables
  struct termios options; // The options for the file descriptor

  if (tcgetattr(_fd, &options) == -1) {
    THROW (IOException, "::tcgetattr");
  }

  // set up raw mode / no echo / binary
  options.c_cflag |= (tcflag_t)  (CLOCAL | CREAD);
  options.c_lflag &= (tcflag_t) ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG | IEXTEN); //|ECHOPRT

  options.c_oflag &= (tcflag_t) ~(OPOST);
  options.c_iflag &= (tcflag_t) ~(INLCR | IGNCR | ICRNL | IGNBRK);
#ifdef IUCLC
  options.c_iflag &= (tcflag_t) ~IUCLC;
#endif  // IUCLC
#ifdef PARMRK
  options.c_iflag &= (tcflag_t) ~PARMRK;
#endif  // PARMRK

  // setup baud rate
  bool custom_baud { false };
  speed_t baud;
  switch (_baudrate) {
#ifdef B0
  case 0: baud = B0; break;
#endif
#ifdef B50
  case 50: baud = B50; break;
#endif
#ifdef B75
  case 75: baud = B75; break;
#endif
#ifdef B110
  case 110: baud = B110; break;
#endif
#ifdef B134
  case 134: baud = B134; break;
#endif
#ifdef B150
  case 150: baud = B150; break;
#endif
#ifdef B200
  case 200: baud = B200; break;
#endif
#ifdef B300
  case 300: baud = B300; break;
#endif
#ifdef B600
  case 600: baud = B600; break;
#endif
#ifdef B1200
  case 1200: baud = B1200; break;
#endif
#ifdef B1800
  case 1800: baud = B1800; break;
#endif
#ifdef B2400
  case 2400: baud = B2400; break;
#endif
#ifdef B4800
  case 4800: baud = B4800; break;
#endif
#ifdef B7200
  case 7200: baud = B7200; break;
#endif
#ifdef B9600
  case 9600: baud = B9600; break;
#endif
#ifdef B14400
  case 14400: baud = B14400; break;
#endif
#ifdef B19200
  case 19200: baud = B19200; break;
#endif
#ifdef B28800
  case 28800: baud = B28800; break;
#endif
#ifdef B57600
  case 57600: baud = B57600; break;
#endif
#ifdef B76800
  case 76800: baud = B76800; break;
#endif
#ifdef B38400
  case 38400: baud = B38400; break;
#endif
#ifdef B115200
  case 115200: baud = B115200; break;
#endif
#ifdef B128000
  case 128000: baud = B128000; break;
#endif
#ifdef B153600
  case 153600: baud = B153600; break;
#endif
#ifdef B230400
  case 230400: baud = B230400; break;
#endif
#ifdef B256000
  case 256000: baud = B256000; break;
#endif
#ifdef B460800
  case 460800: baud = B460800; break;
#endif
#ifdef B500000
  case 500000: baud = B500000; break;
#endif
#ifdef B576000
  case 576000: baud = B576000; break;
#endif
#ifdef B921600
  case 921600: baud = B921600; break;
#endif
#ifdef B1000000
  case 1000000: baud = B1000000; break;
#endif
#ifdef B1152000
  case 1152000: baud = B1152000; break;
#endif
#ifdef B1500000
  case 1500000: baud = B1500000; break;
#endif
#ifdef B2000000
  case 2000000: baud = B2000000; break;
#endif
#ifdef B2500000
  case 2500000: baud = B2500000; break;
#endif
#ifdef B3000000
  case 3000000: baud = B3000000; break;
#endif
#ifdef B3500000
  case 3500000: baud = B3500000; break;
#endif
#ifdef B4000000
  case 4000000: baud = B4000000; break;
#endif
  default:
    custom_baud = true;
  }

  if (!custom_baud) {
#ifdef _BSD_SOURCE
    ::cfsetspeed(&options, baud);
#else
    ::cfsetispeed(&options, baud);
    ::cfsetospeed(&options, baud);
#endif
  }

  // Setup char length
  options.c_cflag &= (tcflag_t) ~CSIZE;
  if (_bytesize == ByteSize::Eight) {
    options.c_cflag |= CS8;
  } else if (_bytesize == ByteSize::Seven) {
    options.c_cflag |= CS7;
  } else if (_bytesize == ByteSize::Six) {
    options.c_cflag |= CS6;
  } else if (_bytesize == ByteSize::Five) {
    options.c_cflag |= CS5;
  } else {
    throw std::invalid_argument("invalid char length");
  }

  // Setup stop bits
  if (_stopbits == StopBits::One) {
    options.c_cflag &= (tcflag_t) ~(CSTOPB);
  } else if (_stopbits == StopBits::OnePointFive) {
    // ONE POINT FIVE same as TWO.. there is no POSIX support for 1.5
    options.c_cflag |= (CSTOPB);
  } else if (_stopbits == StopBits::Two) {
    options.c_cflag |= (CSTOPB);
  } else {
    throw std::invalid_argument ("invalid stop bit");
  }

  // Setup parity
  options.c_iflag &= (tcflag_t) ~(INPCK | ISTRIP);
  if (_parity == Parity::None) {
    options.c_cflag &= (tcflag_t) ~(PARENB | PARODD);
  } else if (_parity == Parity::Even) {
    options.c_cflag &= (tcflag_t) ~(PARODD);
    options.c_cflag |=  (PARENB);
  } else if (_parity == Parity::Odd) {
    options.c_cflag |=  (PARENB | PARODD);
  }
#ifdef CMSPAR
  else if (_parity == Parity::Mark) {
    options.c_cflag |=  (PARENB | CMSPAR | PARODD);
  } else if (_parity == Parity::Space) {
    options.c_cflag |=  (PARENB | CMSPAR);
    options.c_cflag &= (tcflag_t) ~(PARODD);
  }
#else
  // CMSPAR is not defined on OSX. So do not support mark or space parity.
  else if (_parity == parity_mark || _parity == parity_space) {
    throw invalid_argument ("OS does not support mark or space parity");
  }
#endif  // ifdef CMSPAR
  else {
    throw std::invalid_argument ("invalid parity");
  }

  // Setup flow control
  if (_flowcontrol == FlowControl::None) {
    _xonxoff = false;
    _rtscts = false;
  } else if (_flowcontrol == FlowControl::Software) {
    _xonxoff = true;
    _rtscts = false;
  } else if (_flowcontrol == FlowControl::Hardware) {
    _xonxoff = false;
    _rtscts = true;
  }

  // xon-xoff
#ifdef IXANY
  if (_xonxoff) {
    options.c_iflag |=  (IXON | IXOFF);
  } else {
    options.c_iflag &= (tcflag_t) ~(IXON | IXOFF | IXANY);
  }
#else
  if (xonxoff_) {
    options.c_iflag |=  (IXON | IXOFF);
  } else {
    options.c_iflag &= (tcflag_t) ~(IXON | IXOFF);
  }
#endif

  // rts-cts
#ifdef CRTSCTS
  if (_rtscts) {
    options.c_cflag |=  (CRTSCTS);
  } else {
    options.c_cflag &= (uint64_t) ~(CRTSCTS);
  }
#elif defined CNEW_RTSCTS
  if (_rtscts) {
    options.c_cflag |=  (CNEW_RTSCTS);
  } else {
    options.c_cflag &= (uint64_t) ~(CNEW_RTSCTS);
  }
#else
#error "OS Support seems wrong."
#endif

  // http://www.unixwiz.net/techtips/termios-vmin-vtime.html
  // this basically sets the read call up to be a polling read,
  // but we are using select to ensure there is data available
  // to read before each call, so we should never needlessly poll
  options.c_cc[VMIN] = 0;
  options.c_cc[VTIME] = 0;

  // activate settings
  ::tcsetattr(_fd, TCSANOW, &options);

  // Apply custom baud rate, if any
  if (custom_baud) {
    // Linux Support
#if defined(__linux__) && defined (TIOCSSERIAL)
    struct serial_struct ser;

    if (ioctl(_fd, TIOCGSERIAL, &ser) == -1) {
      THROW (IOException, errno);
    }

    // set custom divisor
    ser.custom_divisor = ser.baud_base / static_cast<int>(_baudrate);
    // update flags
    ser.flags &= ~ASYNC_SPD_MASK;
    ser.flags |= ASYNC_SPD_CUST;

    if (ioctl(_fd, TIOCSSERIAL, &ser) == -1) {
      THROW (IOException, errno);
    }
#else
    throw invalid_argument ("OS does not currently support custom bauds");
#endif
  }

  // Update byte_time_ based on the new settings.
  uint32_t bit_time_ns = 1e9 / _baudrate;
  _byteTime_ns = bit_time_ns * (1 + (uint32_t)_bytesize + (uint32_t)_parity + (uint32_t)_stopbits);

  // Compensate for the stopbits_one_point_five enum being equal to int 3,
  // and not 1.5.
  if (_stopbits == StopBits::OnePointFive) {
    _byteTime_ns += ((1.5 - (uint32_t)StopBits::OnePointFive) * bit_time_ns);
  }
}
//-----------------------------------------------------------------------------
size_t Uarts::_Write (const uint8_t * data, size_t length) {
  // If the port is not open, throw
  if (!_is_open) {
    throw PortNotOpenedException ("Uarts::Write");
  }

  // Function Variables
  fd_set  writefds;
  size_t  bytesWritten {};
  bool    firstIteration { true };

  // Calculate total timeout in milliseconds t_c + (t_m * N)
  int64_t totalTimeoutMS { _timeout.WriteTimeoutConstant };
  totalTimeoutMS += _timeout.WriteTimeoutMultiplier * static_cast<int64_t>(length);
  MillisecondTimer totalTimeout(totalTimeoutMS);

  // Loop all bytes length
  while (bytesWritten < length) {
    int64_t timeoutRemainingMS { totalTimeout.Remaining() };

    // Only consider the timeout if it's not the first iteration of the loop
    // otherwise a timeout of 0 won't be allowed through
    if (!firstIteration && (timeoutRemainingMS <= 0)) {
      break; // Timed out
    }
    firstIteration = false;

    timespec timeout(timespec_from_ms(timeoutRemainingMS));

    FD_ZERO (&writefds);
    FD_SET (_fd, &writefds);

    // Do the select
    int32_t r = pselect (_fd + 1, nullptr, &writefds, NULL, &timeout, nullptr);

    // Figure out what happened by looking at select's response 'r'
    if (r < 0) {  // Error
      // Select was interrupted, try again
      if (errno == EINTR) {
        continue;
      }
      // Otherwise there was some error
      THROW (IOException, errno);
    }

    if (r == 0) { // Timeout
      break;
    }

    // Port ready to write
    if (r > 0) {
      // Make sure our file descriptor is in the ready to write list
      if (FD_ISSET (_fd, &writefds)) {
        // This will write some
        ssize_t bytesWrittenNow = ::write (_fd, data + bytesWritten, length - bytesWritten);

        // even though pselect returned readiness the call might still be
        // interrupted. In that case simply retry.
        if (bytesWrittenNow == -1 && errno == EINTR) {
          continue;
        }

        // write should always return some data as select reported it was
        // ready to write when we get to this point.
        if (bytesWrittenNow < 1) {
          // Disconnected devices, at least on Linux, show the
          // behavior that they are always ready to write immediately
          // but writing returns nothing.
          std::stringstream strs;
          strs << "device reports readiness to write but returned no data (device disconnected?)";
          strs << " errno =" << errno;
          strs << " bytesWrittenNow = " << bytesWrittenNow;
          strs << " bytesWritten =" << bytesWritten;
          strs << " length =" << length;
          throw UartException(strs.str().c_str());
        }

        // Update bytes_written
        bytesWritten += static_cast<size_t>(bytesWrittenNow);

        // If bytes_written == size then we have written everything we need to
        if (bytesWritten == length) {
          break;
        }

        // If bytes_written < size then we have more to write
        if (bytesWritten < length) {
          continue;
        }

        // If bytes_written > size then we have over written, which shouldn't happen
        if (bytesWritten > length) {
          throw UartException ("write over wrote, too many bytes where written, this shouldn't happen, might be a logical error!");
        }
      }

      // This shouldn't happen, if r > 0 our fd has to be in the list!
      THROW (IOException, "select reports ready to write, but our fd isn't in the list, this shouldn't happen!");
    }
  }
  return bytesWritten;
}
//-----------------------------------------------------------------------------
size_t Uarts::_Read (uint8_t * buffer, size_t size) {
  // If the port is not open, throw
  if (!_is_open) {
    throw PortNotOpenedException ("Uarts::read");
  }

  // Function Variables
  size_t bytesRead = 0;

  // Calculate total timeout in milliseconds t_c + (t_m * N)
  int64_t totalTimeoutMS = _timeout.ReadTimeoutConstant;
  totalTimeoutMS += _timeout.ReadTimeoutMultiplier * static_cast<int64_t>(size);
  MillisecondTimer totalTimeout(totalTimeoutMS);

  // Pre-fill buffer with available bytes
  ssize_t bytesReadNow = ::read(_fd, buffer, size);
  if (bytesReadNow > 0) {
    bytesRead = bytesReadNow;
  }

  // Loop all bytes size
  while (bytesRead < size) {
    int64_t timeoutRemainingMS = totalTimeout.Remaining();
    if (timeoutRemainingMS <= 0) {
      // Timed out
      break;
    }
    // Timeout for the next select is whichever is less of the remaining
    // total read timeout and the inter-byte timeout.
    //uint32_t timeout = std::min(static_cast<uint32_t>(timeoutRemainingMS), _timeout.InterByteTimeout);

    // Wait for the device to be readable, and then attempt to read.
    if (WaitReadable()) {
      // If it's a fixed-length multi-byte read, insert a wait here so that
      // we can attempt to grab the whole thing in a single IO call. Skip
      // this wait if a non-max inter_byte_timeout is specified.
      if (size > 1 && _timeout.InterByteTimeout == Timeout::Max()) {
        size_t bytesAvailable = Available();
        if (bytesAvailable + bytesRead < size) {
          WaitByteTimes(size - (bytesAvailable + bytesRead));
        }
      }
      // This should be non-blocking returning only what is available now
      //  Then returning so that select can block again.
      ssize_t bytesReadNow = ::read (_fd, buffer + bytesRead, size - bytesRead);

      // read should always return some data as select reported it was
      // ready to read when we get to this point.
      if (bytesReadNow < 1) {
        // Disconnected devices, at least on Linux, show the
        // behavior that they are always ready to read immediately
        // but reading returns nothing.
        throw UartException ("device reports readiness to read but returned no data (device disconnected?)");
      }

      // Update bytes_read
      bytesRead += static_cast<size_t>(bytesReadNow);

      // If bytes_read == size then we have read everything we need
      if (bytesRead == size) {
        break;
      }

      // If bytes_read < size then we have more to read
      if (bytesRead < size) {
        continue;
      }

      // If bytes_read > size then we have over read, which shouldn't happen
      if (bytesRead > size) {
        throw UartException ("read over read, too many bytes where read, this shouldn't happen, might be a logical error!");
      }
    }
  }

  return bytesRead;
}
//-----------------------------------------------------------------------------

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Private Function used by friend classes ScopedWriteLock & ScopedReadLock

//-----------------------------------------------------------------------------
void Uarts::ReadLock () {
  // Function Variables
  int result {};

  // Lock mutex
  if ((result = pthread_mutex_lock(&_readMutex)) != 0) {
    THROW (IOException, result);
  }
}
//-----------------------------------------------------------------------------
void Uarts::ReadUnlock(void) {
  // Function Variables
  int32_t result {};

  // Unlock mutex
  if ((result = pthread_mutex_unlock(&_readMutex)) |= 0) {
    THROW (IOException, result);
  }
}
//-----------------------------------------------------------------------------
void Uarts::WriteLock(void) {
  // Function Variables
  int32_t result {};

  // Lock mutex
  if ((result = pthread_mutex_lock(&_writeMutex)) != 0) {
    THROW (IOException, result);
  }
}
//-----------------------------------------------------------------------------
void Uarts::WriteUnlock(void) {
  // Function Variables
  int result {};

  // Unlock mutex
  if ((result = pthread_mutex_unlock(&_writeMutex)) != 0) {
    THROW (IOException, result);
  }
}
//-----------------------------------------------------------------------------

}

