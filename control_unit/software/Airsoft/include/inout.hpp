/**
 *******************************************************************************
 * @file inout.hpp
 *
 * @brief Input/Output/Leds and Keyboard Engine header file
 *
 * @author  Cristian Croci - CCDevelop.net
 *
 * @version 1.00
 *
 * @date Dec 3, 2024
 *
 *******************************************************************************
 * This file is part of the Airsoft Game Machine project
 * https://github.com/ccdevelop-net/AirsoftGameMachine.
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
#ifndef INOUT_HPP_
#define INOUT_HPP_

#include <string>
#include <cstring>
#include <thread>
#include <queue>
#include <mutex>

#include <devices/pcf8574.hpp>
#include <devices/i2ckeypad.hpp>

namespace Airsoft {

/**
 * @brief Address of GPIO Expander
 */
constexpr uint8_t IO0__7_ADDR     = 0x26;     /** << Address of PCF8574 for IO 0 to 7    */
constexpr uint8_t IO8_15_ADDR     = 0x20;     /** << Address of PCF8574 for IO 8 to 15   */
constexpr uint8_t KEYBOARD_ADDR   = 0x23;     /** << Address of PCF8574 for keyboard 4x4 */

/**
 * @brief Leds Mapping
 */
constexpr uint8_t LED1_PIN  = 0x07;           /** << Pin assigned to led 1                    */
constexpr uint8_t LED1_ADDR = IO0__7_ADDR;    /** << Address of associated expander for led 1 */
constexpr uint8_t LED2_PIN  = 0x06;           /** << Pin assigned to led 2                    */
constexpr uint8_t LED2_ADDR = IO0__7_ADDR;    /** << Address of associated expander for led 2 */
constexpr uint8_t LED3_PIN  = 0x05;           /** << Pin assigned to led 3                    */
constexpr uint8_t LED3_ADDR = IO0__7_ADDR;    /** << Address of associated expander for led 3 */
constexpr uint8_t LED4_PIN  = 0x04;           /** << Pin assigned to led 4                    */
constexpr uint8_t LED4_ADDR = IO0__7_ADDR;    /** << Address of associated expander for led 4 */
constexpr uint8_t LED5_PIN  = 0x03;           /** << Pin assigned to led 5                    */
constexpr uint8_t LED5_ADDR = IO0__7_ADDR;    /** << Address of associated expander for led 5 */

/**
 * @brief Reles Mapping
 */
constexpr uint8_t RELE1_PIN   = 0x02;
constexpr uint8_t RELE1_ADDR  = IO0__7_ADDR;
constexpr uint8_t RELE2_PIN   = 0x01;
constexpr uint8_t RELE2_ADDR  = IO0__7_ADDR;
constexpr uint8_t RELE3_PIN   = 0x00;
constexpr uint8_t RELE3_ADDR  = IO0__7_ADDR;
constexpr uint8_t RELE4_PIN   = 0x07;
constexpr uint8_t RELE4_ADDR  = IO8_15_ADDR;
constexpr uint8_t RELE5_PIN   = 0x06;
constexpr uint8_t RELE5_ADDR  = IO8_15_ADDR;
constexpr uint8_t RELE6_PIN   = 0x05;
constexpr uint8_t RELE6_ADDR  = IO8_15_ADDR;

// The IO board work with inverted logic
constexpr bool ON   = false;
constexpr bool OFF  = true;

// Index of the leds
constexpr uint8_t LED1  = 0;
constexpr uint8_t LED2  = 1;
constexpr uint8_t LED3  = 2;
constexpr uint8_t LED4  = 3;
constexpr uint8_t LED5  = 4;
constexpr uint8_t RELE1 = 5;
constexpr uint8_t RELE2 = 6;
constexpr uint8_t RELE3 = 7;
constexpr uint8_t RELE4 = 8;
constexpr uint8_t RELE5 = 9;
constexpr uint8_t RELE6 = 10;
constexpr uint8_t MAX_IO = 11;

struct IOMap {
  uint8_t   Address;
  uint8_t   Pin;
  bool      Value;
};

struct KeyData {
  char      Key;
  uint8_t   KeyCode;
};


class InOut final {
private:
  InOut(void) = default;

public:
  virtual ~InOut(void) = default;

public:
  static InOut & Instance(void) {
    static InOut _instance;
    return _instance;
  }

public:
  bool Init(std::string port);
  void Terminate(void);
  void Led(uint8_t ledId, bool value);
  void Rele(uint8_t releId, bool value);

  bool inline IsReady(void) {
    return _ready;
  }

  size_t KeysOnQueue(void);
  bool GetKeyFromQueue(char & key, uint8_t & keyCode);


private:
  std::thread       *     _process {};      // Pointer to thread
  bool                    _threadRunning {};         // Running flag for thread
  std::string             _port;
  std::mutex              _wireLock;

  std::queue<KeyData*>     _keysQueue;
  std::mutex               _queueLock;

  Airsoft::Devices::I2CKeyPad * _keyboard {};
  Airsoft::Devices::PCF8574 * _out0_7 {};
  Airsoft::Devices::PCF8574 * _out8_15 {};

  IOMap   _map[MAX_IO] {
    { LED1_ADDR,  LED1_PIN,  OFF },
    { LED2_ADDR,  LED2_PIN,  OFF },
    { LED3_ADDR,  LED3_PIN,  OFF },
    { LED4_ADDR,  LED4_PIN,  OFF },
    { LED5_ADDR,  LED5_PIN,  OFF },
    { RELE1_ADDR, RELE1_PIN, OFF },
    { RELE2_ADDR, RELE2_PIN, OFF },
    { RELE3_ADDR, RELE3_PIN, OFF },
    { RELE4_ADDR, RELE4_PIN, OFF },
    { RELE5_ADDR, RELE5_PIN, OFF },
    { RELE6_ADDR, RELE6_PIN, OFF }
  };

  bool _ready {};

  char _keymap[16] {
    '1', '2', '3', 'A',
    '4', '5', '6', 'B',
    '7', '8', '9', 'C',
    '*', '0', '#', 'D'
  };

private:
  void Engine(void);
  void ManageIo(IOMap * map, bool value);

};

} // namespace Airsoft

#endif // INOUT_HPP_
