/**
 *******************************************************************************
 * @file uarts.hpp
 *
 * @brief Driver UARTS for Luckfox Pico boards
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
#ifndef _UARTS_HPP_
#define _UARTS_HPP_

#include <limits>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <drivers/uarts/enumerators.hpp>
#include <drivers/uarts/exceptions.hpp>
#include <drivers/uarts/timeout.hpp>


namespace Airsoft::Drivers {

/**
 * @brief Throw Exception
 */
#define THROW(exceptionClass, message) throw exceptionClass(__FILE__, __LINE__, (message))

/**
 * @brief Class that provides a portable uart port interface.
 */
class Uarts final {
public:   // Constructor/Destructor
  /**
   * @brief Creates a Serial object and opens the port if a port is specified, otherwise it remains closed until
   *        SerialDriver::Serial::Open is called.
   * @param port - A std::string containing the address of the serial port, which would be
   *             something like '/dev/ttyS0' on Linux.
   * @param baudrate - An unsigned 32-bit integer that represents the baudrate
   * @param timeout - A Serial::Timeout struct that defines the timeout conditions for the serial port.
   *        @see Serial::Timeout
   * @param bytesize -  Size of each byte in the serial transmission of data, default is ByteSize::Eight,
   *        possible values are: ByteSize::Five, ByteSize::Six, ByteSize::Seven, ByteSize::Eight.
   * @param parity - Method of parity, default is parity_none, possible values are: parity_none, parity_odd, parity_even
   * @param stopbits - Number of stop bits used, default is stopbits_one, possible values are:
   *                   stopbits_one, stopbits_one_point_five, stopbits_two
   * @param flowcontrol - Type of flowcontrol used, default is flowcontrol_none, possible values are: flowcontrol_none,
   *                      flowcontrol_software, flowcontrol_hardware
   *
   * @throw Serial::PortNotOpenedException
   * @throw Serial::IOException
   * @throw std::invalid_argument
   */
  Uarts (const std::string &port = "", uint32_t baudrate = 9600, Timeout timeout = Timeout(),
          ByteSize bytesize = ByteSize::Eight, Parity parity = Parity::None, StopBits stopbits = StopBits::One,
          FlowControl flowcontrol = FlowControl::None);
  virtual ~Uarts (void) = default;

public:   // Friend classes
  friend class ScopedReadLock;
  friend class ScopedWriteLock;


public:   // Public Functions
  /**
   * @brief Opens the serial port as long as the port is set and the port isn't already open.
   *        If the port is provided to the constructor then an explicit call to open is not needed.
   * @see Serial::Serial
   * @throw std::invalid_argument
   * @throw Serial::SerialException
   * @throw Serial::IOException
   */
  void Open(void);

  /**
   * @brief Gets the open status of the serial port.
   * @return Returns true if the port is open, false otherwise.
   */
  bool IsOpen(void) const;

  /**
   * @brief Closes the serial port.
   */
  void Close(void);

  /**
   * @brief Return the number of characters in the buffer.
   */
  size_t Available(void);

  /**
   * @brief Block until there is serial data to read or read_timeout_constant number of milliseconds have elapsed.
   *        The return value is true when the function exits with the port in a readable state, false otherwise
   *        (due to timeout or select interruption).
   */
  bool WaitReadable(void);

  /**
   * @brief Block for a period of time corresponding to the transmission time of count characters at present serial settings.
   * This may be used in con-junction with waitReadable to read larger blocks of data from the port.
   */
  void WaitByteTimes(size_t count);

  /**
   * @brief Read a given amount of bytes from the serial port into a given buffer.
   *        The read function will return in one of three cases:
   *        * The number of requested bytes was read.
   *          * In this case the number of bytes requested will match the size_t returned by read.
   *        * A timeout occurred, in this case the number of bytes read will not match the amount requested,
   *          but no exception will be thrown.  One of two possible timeouts occurred:
   *          * The inter byte timeout expired, this means that number of milliseconds elapsed between receiving bytes
   *            from the serial port exceeded the inter byte timeout.
   *          * The total timeout expired, which is calculated by multiplying the read timeout multiplier by the number
   *            of requested bytes and then added to the read timeout constant.  If that total number of milliseconds
   *            elapses after the initial call to read a timeout will occur.
   *        * An exception occurred, in this case an actual exception will be thrown.
   * @param buffer - An uint8_t array of at least the requested size.
   * @param size - A size_t defining how many bytes to be read.
   * @return A size_t representing the number of bytes read as a result of the call to read.
   *
   * @throw Serial::PortNotOpenedException
   * @throw Serial::SerialException
   */
  size_t Read(uint8_t * buffer, size_t size);

  /**
   * @brief Read a given amount of bytes from the serial port into a give buffer.
   * @param buffer - A reference to a std::vector of uint8_t.
   * @param size - A size_t defining how many bytes to be read.
   * @return A size_t representing the number of bytes read as a result of the call to read.
   * @throw Serial::PortNotOpenedException
   * @throw Serial::SerialException
   */
  size_t Read(std::vector<uint8_t> & buffer, size_t size = 1);

  /**
   * @brief Read a given amount of bytes from the serial port into a give buffer.
   * @param buffer - A reference to a std::string.
   * @param size - A size_t defining how many bytes to be read.
   * @return A size_t representing the number of bytes read as a result of the call to read.
   * @throw Serial::PortNotOpenedException
   * @throw Serial::SerialException
   */
  size_t Read(std::string & buffer, size_t size = 1);

  /**
   * @brief Read a given amount of bytes from the serial port and return a string containing the data.
   * @param size A size_t defining how many bytes to be read.
   * @return A std::string containing the data read from the port.
   * @throw Serial::PortNotOpenedException
   * @throw Serial::SerialException
   */
  std::string Read(size_t size = 1);

  /**
   * @brief Reads in a line or until a given delimiter has been processed.
   *        Reads from the serial port until a single line has been read.
   * @param buffer - A std::string reference used to store the data.
   * @param size - A maximum length of a line, defaults to 65536 (2^16)
   * @param eol - A string to match against for the EOL.
   * @return A size_t representing the number of bytes read.
   * @throw Serial::PortNotOpenedException
   * @throw Serial::SerialException
   */
  size_t ReadLine(std::string & buffer, size_t size = 65536, std::string eol = "\n");

  /**
   * @brief Reads in a line or until a given delimiter has been processed.
   *        Reads from the serial port until a single line has been read.
   * @param size - A maximum length of a line, defaults to 65536 (2^16)
   * @param eol - A string to match against for the EOL.
   * @return A std::string containing the line.
   * @throw Serial::PortNotOpenedException
   * @throw Serial::SerialException
   */
  std::string ReadLine(size_t size = 65536, std::string eol = "\n");

  /**
   * @brief Reads in multiple lines until the serial port times out.
   *        This requires a timeout > 0 before it can be run. It will read until a timeout occurs and
   *        return a list of strings.
   * @param size - A maximum length of combined lines, defaults to 65536 (2^16)
   * @param eol - A string to match against for the EOL.
   * @return A vector<string> containing the lines.
   * @throw Serial::PortNotOpenedException
   * @throw Serial::SerialException
   */
  std::vector<std::string> ReadLines(size_t size = 65536, std::string eol = "\n");

  /**
   * @brief Write a string to the serial port.
   * @param data - A const reference containing the data to be written to the serial port.
   * @param size - A size_t that indicates how many bytes should be written from the given data buffer.
   * @return A size_t representing the number of bytes actually written to the serial port.
   * @throw Serial::PortNotOpenedException
   * @throw Serial::SerialException
   * @throw Serial::IOException
   */
  size_t Write(const uint8_t * data, size_t size);

  /**
   * @brief Write a string to the serial port.
   * @param data - A const reference containing the data to be written* to the serial port.
   * @return A size_t representing the number of bytes actually written to the serial port.
   * @throw Serial::PortNotOpenedException
   * @throw Serial::SerialException
   * @throw Serial::IOException
   */
  size_t Write(const std::vector<uint8_t> & data);

  /**
   * @brief Write a string to the serial port.
   * @param data - A const reference containing the data to be written to the serial port.
   * @return A size_t representing the number of bytes actually written to the serial port.
   * @throw Serial::PortNotOpenedException
   * @throw Serial::SerialException
   * @throw Serial::IOException
   */
  size_t Write(const std::string & data);

  /**
   * @brief Sets the serial port identifier.
   * @param port - A const std::string reference containing the address of the serial port, which would be something
   *        '/dev/ttyS0' on Linux.
   * @throw std::invalid_argument
   */
  void SetPort(const std::string & port);

  /**
   * @brief Gets the serial port identifier.
   * @see Serial::SetPort
   * @throw std::invalid_argument
   */
  std::string GetPort(void) const;

  /**
   * @brief Sets the timeout for reads and writes using the Timeout struct.
   *        There are two timeout conditions described here:
   *          * The inter byte timeout:
   *            * The inter_byte_timeout component of serial::Timeout defines the maximum amount of time, in milliseconds,
   *              between receiving bytes on the serial port that can pass before a timeout occurs.
   *              Setting this to zero will prevent inter-byte timeouts from occurring.
   *          * Total time timeout:
   *            * The constant and multiplier component of this timeout condition, for both read and write,
   *              are defined in Serial::Timeout. This timeout occurs if the total time since the read or write call was
   *              made exceeds the specified time in milliseconds.
   *          * The limit is defined by multiplying the multiplier component by the number of requested bytes and adding
   *            that product to the constant component. In this way if you want a read call, for example, to timeout
   *            after exactly one second regardless of the number of bytes you asked for then set the
   *            ReadTimeoutConstant component of Serial::Timeout to 1000 and the ReadTimeoutMultiplier to zero.
   *            This timeout condition can be used in conjunction with the inter-byte timeout condition without any
   *            problems, timeout will simply occur when one of the two timeout conditions is met. This allows users to
   *            have maximum control over the trade-off between responsiveness and efficiency.
   *        Read and write functions will return in one of three cases.  When the reading or writing is complete,
   *        when a timeout occurs, or when an exception occurs.
   *        A timeout of 0 enables non-blocking mode.
   * @param timeout - A Serial::Timeout struct containing the inter-byte timeout, and the read and write timeout
   *                  constants and multipliers.
   * @see Serial::Timeout
   */
  void SetTimeout(Timeout & timeout);

  /**
   * @brief Sets the timeout for reads and writes.
   */
  void SetTimeout(uint32_t interByteTimeout, uint32_t readTimeoutConstant, uint32_t readTimeoutMultiplier,
                  uint32_t writeTimeoutConstant, uint32_t writeTimeoutMultiplier) {
    Timeout timeout(interByteTimeout, readTimeoutConstant, readTimeoutMultiplier, writeTimeoutConstant, writeTimeoutMultiplier);
    return SetTimeout(timeout);
  }

  /**
   * @brief Gets the timeout for reads in seconds.
   * @return A Timeout struct containing the inter_byte_timeout, and read and write timeout constants and multipliers.
   * @see Serial::SetTimeout
   */
  Timeout GetTimeout(void) const;

  /**
   * @brief Sets the baudrate for the serial port.
   *        Possible baudrates depends on the system but some safe baudrates include:
   *        110, 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 56000, 57600, 115200
   *        Some other baudrates that are supported by some comports:
   *        128000, 153600, 230400, 256000, 460800, 500000, 921600
   * @param baudrate - An integer that sets the baud rate for the serial port.
   * @throw std::invalid_argument
   */
  void SetBaudrate(uint32_t baudrate);

  /**
   * @brief Gets the baudrate for the serial port.
   * @return An integer that sets the baud rate for the serial port.
   * @see Serial::SetBaudrate
   * @throw std::invalid_argument
   */
  uint32_t GetBaudrate(void) const;

  /**
   * @brief Sets the byte size for the serial port.
   * @param bytesize - Size of each byte in the serial transmission of data, default is ByteSize::EightBits,
   *        possible values are: ByteSize::FiveBits, ByteSize::SixBits, ByteSize::SevenBits, ByteSize::EightBits
   * @throw std::invalid_argument
   */
  void SetBytesize(ByteSize bytesize);

  /**
   * @brief Gets the bytesize for the serial port.
   * @see Serial::SetBytesize
   * @throw std::invalid_argument
   */
  ByteSize GetBytesize(void) const;

  /**
   * @brief Sets the parity for the serial port.
   * @param parity - Method of parity, default is Parity::None, possible values
   *        are: Parity::None, Parity::Odd, Parity::Even
   * @throw std::invalid_argument
   */
  void SetParity(Parity parity);

  /**
   * @brief Gets the parity for the serial port.
   * @see Serial::SetParity
   * @throw std::invalid_argument
   */
  Parity GetParity(void) const;

  /**
   * @brief Sets the stop bits for the serial port.
   * @param stopbits - Number of stop bits used, default is StopBits::One,
   *        possible values are: StopBits::One, StopBits::OnePointFive, StopBits::Two
   * @throw std::invalid_argument
   */
  void SetStopbits(StopBits stopbits);

  /**
   * @brief Gets the stopbits for the serial port.
   * @see Serial::setStopbits
   * @throw std::invalid_argument
   */
  StopBits GetStopbits(void) const;

  /**
   * @brief Sets the flow control for the serial port.
   * @param flowcontrol - Type of FlowControl used, default is FlowControl::None,
   *        possible values are: FlowControl::None, FlowControl::Software, FlowControl::Hardware
   * @throw std::invalid_argument
   */
  void SetFlowcontrol(FlowControl flowcontrol);

  /**
   * @brief Gets the flow control for the serial port.
   * @see Serial::setFlowcontrol
   * @throw std::invalid_argument
   */
  FlowControl GetFlowcontrol(void) const;

  /**
   * @brief Flush the input and output buffers
   */
  void Flush(void);

  /**
   * @brief Flush only the input buffer
   */
  void FlushInput(void);

  /**
   * @brief Flush only the output buffer
   */
  void FlushOutput(void);

  /**
   * @brief Sends the RS-232 break signal.
   * See tcsendbreak(3).
   */
  void SendBreak(int32_t duration);

  /**
   * @brief Set the break condition to a given level.
   *        Defaults to true.
   */
  void SetBreak(bool level = true);

  /**
   * @brief Set the RTS handshaking line to the given level.
   *        Defaults to true.
   */
  void SetRTS(bool level = true);

  /**
   * @brief Set the DTR handshaking line to the given level.
   *        Defaults to true.
   */
  void SetDTR (bool level = true);

  /**
   * @brief Blocks until CTS, DSR, RI, CD changes or something interrupts it.
   *        Can throw an exception if an error occurs while waiting.
   *        You can check the status of CTS, DSR, RI, and CD once this returns.
   *        Uses TIOCMIWAIT via ioctl if available (mostly only on Linux) with a resolution of less than +-1ms
   *        and as good as +-0.2ms. Otherwise a polling method is used which can give +-2ms.
   * @return Returns true if one of the lines changed, false if something else occurred.
   * @throw SerialException
   */
  bool WaitForChange(void);

  /**
   * @brief Returns the current status of the CTS line.
   */
  bool GetCTS(void);

  /**
   * @brief Returns the current status of the DSR line.
   */
  bool GetDSR(void);

  /**
   * @brief Returns the current status of the RI line.
   */
  bool GetRI ();

  /**
   * @brief Returns the current status of the CD line.
   */
  bool GetCD ();

private:
  // Disable copy constructors
  Uarts(const Uarts&);
  Uarts& operator=(const Uarts&);

private:  // Private Variables
  std::string       _port;                                // Path to the file descriptor
  int32_t           _fd {};                               // The current file descriptor

  bool              _is_open {};
  bool              _xonxoff {};
  bool              _rtscts {};

  Timeout           _timeout;                             // Timeout for read operations
  uint64_t          _baudrate {};                         // Baudrate
  uint32_t          _byteTime_ns {};                      // Nanoseconds to transmit/receive a single byte

  Parity            _parity { Parity::None };             // Parity
  ByteSize          _bytesize { ByteSize::Eight };        // Size of the bytes
  StopBits          _stopbits { StopBits::One };          // Stop Bits
  FlowControl       _flowcontrol { FlowControl::None };   // Flow Control

  pthread_mutex_t   _readMutex;                           // Mutex used to lock the read functions
  pthread_mutex_t   _writeMutex;                          // Mutex used to lock the write functions

private:  // Private Functions
  void ReconfigurePort(void);

  size_t _Read (uint8_t * buffer, size_t size);            // Read common function
  void ReadLock(void);
  void ReadUnlock(void);

  size_t _Write (const uint8_t * data, size_t length);     // Write common function
  void WriteLock(void);
  void WriteUnlock(void);
};

}

#endif // _UARTS_HPP_
