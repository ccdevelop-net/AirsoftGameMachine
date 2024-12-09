/*
 * states_naming.hpp
 *
 *  Created on: Dec 5, 2024
 *      Author: ccroci
 */

#ifndef _DEVICES_STATES_NAMING_HPP_
#define _DEVICES_STATES_NAMING_HPP_

#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstdbool>
#include <string>
#include <cstring>

#ifdef FREQUENCY_433
#define OPERATING_FREQUENCY 410
#elif defined(FREQUENCY_400)
#define OPERATING_FREQUENCY 410
#elif defined(FREQUENCY_230)
#define OPERATING_FREQUENCY 220
#elif defined(FREQUENCY_868)
#define OPERATING_FREQUENCY 850
#elif defined(FREQUENCY_900)
#define OPERATING_FREQUENCY 850
#elif defined(FREQUENCY_915)
#define OPERATING_FREQUENCY 900
#else
#define OPERATING_FREQUENCY 850
#endif

#define BROADCAST_ADDRESS 255

typedef enum RESPONSE_STATUS {
  E220_SUCCESS = 1,
  ERR_E220_UNKNOWN, /* something shouldn't happened */
  ERR_E220_NOT_SUPPORT,
  ERR_E220_NOT_IMPLEMENT,
  ERR_E220_NOT_INITIAL,
  ERR_E220_INVALID_PARAM,
  ERR_E220_DATA_SIZE_NOT_MATCH,
  ERR_E220_BUF_TOO_SMALL,
  ERR_E220_TIMEOUT,
  ERR_E220_HARDWARE,
  ERR_E220_HEAD_NOT_RECOGNIZED,
  ERR_E220_NO_RESPONSE_FROM_DEVICE,
  ERR_E220_WRONG_UART_CONFIG,
  ERR_E220_WRONG_FORMAT,
  ERR_E220_PACKET_TOO_BIG
} Status;

static std::string GetResponseDescriptionByParams(uint8_t status){
  switch (status) {
    case E220_SUCCESS:
      return "Success";
    case ERR_E220_UNKNOWN:
      return "Unknown";
    case ERR_E220_NOT_SUPPORT:
      return "Not support!";
    case ERR_E220_NOT_IMPLEMENT:
      return "Not implement";
    case ERR_E220_NOT_INITIAL:
      return "Not initial!";
    case ERR_E220_INVALID_PARAM:
      return "Invalid parameter!";
    case ERR_E220_DATA_SIZE_NOT_MATCH:
      return "Data size not match!";
    case ERR_E220_BUF_TOO_SMALL:
      return "Buff too small!";
    case ERR_E220_TIMEOUT:
      return "Timeout!!";
    case ERR_E220_HARDWARE:
      return "Hardware error!";
    case ERR_E220_HEAD_NOT_RECOGNIZED:
      return "Save mode returned not recognized!";
    case ERR_E220_NO_RESPONSE_FROM_DEVICE:
      return "No response from device! (Check wiring)";
    case ERR_E220_WRONG_UART_CONFIG:
      return "Wrong UART configuration! (BPS must be 9600 for configuration)";
    case ERR_E220_PACKET_TOO_BIG:
      return "The device support only 200byte of data transmission!";
    default:
      return "Invalid status!";
  }
}

enum class E220UartParity : uint8_t {
  MODE_00_8N1 = 0b00,
  MODE_01_8O1 = 0b01,
  MODE_10_8E1 = 0b10,
  MODE_11_8N1 = 0b11
};

static std::string GetUARTParityDescriptionByParams(E220UartParity uartParity){
  switch (uartParity) {
    case E220UartParity::MODE_00_8N1:
      return "8N1 (Default)";
    case E220UartParity::MODE_01_8O1:
      return "8O1";
    case E220UartParity::MODE_10_8E1:
      return "8E1";
    case E220UartParity::MODE_11_8N1:
      return "8N1 (equal to 00";
    default:
      return "Invalid UART Parity!";
  }
}

enum class UartBpsType : uint8_t {
  UART_BPS_1200   = 0b000,
  UART_BPS_2400   = 0b001,
  UART_BPS_4800   = 0b010,
  UART_BPS_9600   = 0b011,
  UART_BPS_19200  = 0b100,
  UART_BPS_38400  = 0b101,
  UART_BPS_57600  = 0b110,
  UART_BPS_115200 = 0b111
};

enum class UartBpsRate : uint32_t {
  UART_BPS_RATE_1200    = 1200,
  UART_BPS_RATE_2400    = 2400,
  UART_BPS_RATE_4800    = 4800,
  UART_BPS_RATE_9600    = 9600,
  UART_BPS_RATE_19200   = 19200,
  UART_BPS_RATE_38400   = 38400,
  UART_BPS_RATE_57600   = 57600,
  UART_BPS_RATE_115200  = 115200
};

static std::string GetUARTBaudRateDescriptionByParams(UartBpsType uartBaudRate) {
  switch (uartBaudRate) {
    case UartBpsType::UART_BPS_1200:
      return "1200bps";
    case UartBpsType::UART_BPS_2400:
      return "2400bps";
    case UartBpsType::UART_BPS_4800:
      return "4800bps";
    case UartBpsType::UART_BPS_9600:
      return "9600bps (default)";
    case UartBpsType::UART_BPS_19200:
      return "19200bps";
    case UartBpsType::UART_BPS_38400:
      return "38400bps";
    case UartBpsType::UART_BPS_57600:
      return "57600bps";
    case UartBpsType::UART_BPS_115200:
      return "115200bps";
    default:
      return "Invalid UART Baud Rate!";
  }
}

enum class AirDataRate : uint8_t {
  AIR_DATA_RATE_000_24  = 0b000,
  AIR_DATA_RATE_001_24  = 0b001,
  AIR_DATA_RATE_010_24  = 0b010,
  AIR_DATA_RATE_011_48  = 0b011,
  AIR_DATA_RATE_100_96  = 0b100,
  AIR_DATA_RATE_101_192 = 0b101,
  AIR_DATA_RATE_110_384 = 0b110,
  AIR_DATA_RATE_111_625 = 0b111
};

static std::string GetAirDataRateDescriptionByParams(AirDataRate airDataRate) {
  switch (airDataRate) {
    case AirDataRate::AIR_DATA_RATE_000_24:
      return "2.4kbps";
    case AirDataRate::AIR_DATA_RATE_001_24:
      return "2.4kbps";
    case AirDataRate::AIR_DATA_RATE_010_24:
      return "2.4kbps (default)";
    case AirDataRate::AIR_DATA_RATE_011_48:
      return "4.8kbps";
    case AirDataRate::AIR_DATA_RATE_100_96:
      return "9.6kbps";
    case AirDataRate::AIR_DATA_RATE_101_192:
      return "19.2kbps";
    case AirDataRate::AIR_DATA_RATE_110_384:
      return "38.4kbps";
    case AirDataRate::AIR_DATA_RATE_111_625:
      return "62.5kbps";
    default:
      return "Invalid Air Data Rate!";
  }
}

enum class SubPacketSetting : uint8_t {
  SPS_200_00 = 0b00,
  SPS_128_01 = 0b01,
  SPS_064_10 = 0b10,
  SPS_032_11 = 0b11

};

static std::string GetSubPacketSettingByParams(SubPacketSetting subPacketSetting){
  switch (subPacketSetting) {
    case SubPacketSetting::SPS_200_00:
      return "200bytes (default)";
    case SubPacketSetting::SPS_128_01:
      return "128bytes";
    case SubPacketSetting::SPS_064_10:
      return "64bytes";
    case SubPacketSetting::SPS_032_11:
      return "32bytes";
    default:
      return "Invalid Sub Packet Setting!";
  }
}

enum class RssiAmbientNoiseEnable : uint8_t {
  RSSI_AMBIENT_NOISE_ENABLED = 0b1,
  RSSI_AMBIENT_NOISE_DISABLED = 0b0
};

static std::string GetRSSIAmbientNoiseEnableByParams(RssiAmbientNoiseEnable rssiAmbientNoiseEnabled) {
  switch (rssiAmbientNoiseEnabled) {
    case RssiAmbientNoiseEnable::RSSI_AMBIENT_NOISE_ENABLED:
      return "Enabled";
    case RssiAmbientNoiseEnable::RSSI_AMBIENT_NOISE_DISABLED:
      return "Disabled (default)";
    default:
      return "Invalid RSSI Ambient Noise enabled!";
  }
}

enum class WorPeriod : uint8_t {
  WOR_500_000   = 0b000,
  WOR_1000_001  = 0b001,
  WOR_1500_010  = 0b010,
  WOR_2000_011  = 0b011,
  WOR_2500_100  = 0b100,
  WOR_3000_101  = 0b101,
  WOR_3500_110  = 0b110,
  WOR_4000_111  = 0b111
};

static std::string GetWORPeriodByParams(WorPeriod worPeriod) {
  switch (worPeriod) {
    case WorPeriod::WOR_500_000:
      return "500ms";
    case WorPeriod::WOR_1000_001:
      return "1000ms";
    case WorPeriod::WOR_1500_010:
      return "1500ms";
    case WorPeriod::WOR_2000_011:
      return "2000ms (default)";
    case WorPeriod::WOR_2500_100:
      return "2500ms";
    case WorPeriod::WOR_3000_101:
      return "3000ms";
    case WorPeriod::WOR_3500_110:
      return "3500ms";
    case WorPeriod::WOR_4000_111:
      return "4000ms";
    default:
      return "Invalid WOR period!";
  }
}

enum class LbtEnableByte : uint8_t {
  LBT_ENABLED = 0b1,
  LBT_DISABLED = 0b0
};

static std::string GetLBTEnableByteByParams(LbtEnableByte LBTEnableByte) {
  switch (LBTEnableByte) {
    case LbtEnableByte::LBT_ENABLED:
      return "Enabled";
    case LbtEnableByte::LBT_DISABLED:
      return "Disabled (default)";
    default:
      return "Invalid LBT enable byte!";
  }
}

enum class RssiEnableByte : uint8_t {
  RSSI_ENABLED = 0b1,
  RSSI_DISABLED = 0b0
};

static std::string GetRSSIEnableByteByParams(RssiEnableByte RSSIEnableByte) {
  switch (RSSIEnableByte) {
    case RssiEnableByte::RSSI_ENABLED:
      return "Enabled";
    case RssiEnableByte::RSSI_DISABLED:
      return "Disabled (default)";
    default:
      return "Invalid RSSI enable byte!";
  }
}

enum class FixedTransmission : uint8_t {
  FT_TRANSPARENT_TRANSMISSION = 0b0,
  FT_FIXED_TRANSMISSION = 0b1
};

static std::string GetFixedTransmissionDescriptionByParams(FixedTransmission fixedTransmission) {
  switch (fixedTransmission) {
    case FixedTransmission::FT_TRANSPARENT_TRANSMISSION:
      return "Transparent transmission (default)";
    case FixedTransmission::FT_FIXED_TRANSMISSION:
      return "Fixed transmission (first three bytes can be used as high/low address and channel)";
    default:
      return "Invalid fixed transmission parameter!";
  }
}

#ifdef E220_22
enum class TransmissionPower : uint8_t {
  POWER_22 = 0b00,
  POWER_17 = 0b01,
  POWER_13 = 0b10,
  POWER_10 = 0b11
};

static std::string GetTransmissionPowerDescriptionByParams(TransmissionPower transmissionPower) {
  switch (transmissionPower) {
    case TransmissionPower::POWER_22:
      return "22dBm (Default)";
    case TransmissionPower::POWER_17:
      return "17dBm";
    case TransmissionPower::POWER_13:
      return "13dBm";
    case TransmissionPower::POWER_10:
      return "10dBm";
    default:
      return "Invalid transmission power parameter";
  }
}
#elif defined(E220_30)
enum class TransmissionPower : uint8_t {
  POWER_30 = 0b00,
  POWER_27 = 0b01,
  POWER_24 = 0b10,
  POWER_21 = 0b11
};

static std::string GetTransmissionPowerDescriptionByParams(TransmissionPower transmissionPower) {
  switch (transmissionPower) {
    case TransmissionPower::POWER_30:
      return "30dBm (Default)";
    case TransmissionPower::POWER_27:
      return "27dBm";
    case TransmissionPower::POWER_24:
      return "24dBm";
    case TransmissionPower::POWER_21:
      return "21dBm";
    default:
      return "Invalid transmission power parameter";
  }
}
#else
enum class TransmissionPower : uint8_t {
  POWER_22 = 0b00,
  POWER_17 = 0b01,
  POWER_13 = 0b10,
  POWER_10 = 0b11

};

static std::string GetTransmissionPowerDescriptionByParams(TransmissionPower transmissionPower) {
  switch (transmissionPower) {
    case TransmissionPower::POWER_22:
      return "22dBm (Default)";
    case TransmissionPower::POWER_17:
      return "17dBm";
    case TransmissionPower::POWER_13:
      return "13dBm";
    case TransmissionPower::POWER_10:
      return "10dBm";
    default:
      return "Invalid transmission power parameter";
  }
}
#endif



#endif // _DEVICES_STATES_NAMING_HPP_
