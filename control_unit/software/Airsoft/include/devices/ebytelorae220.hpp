/*
 * ebytelorae220.hpp
 *
 *  Created on: Dec 5, 2024
 *      Author: ccroci
 */

#ifndef _DEVICES_EBYTELORAE220_HPP_
#define _DEVICES_EBYTELORAE220_HPP_

#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstdbool>
#include <string>
#include <cstring>
#include <sstream>

#include <devices/states_naming.hpp>
#include <drivers/uarts.hpp>
#include <drivers/gpio.hpp>

#define LoRa_E220_DEBUG

namespace Airsoft::Devices {

enum class ModeType : uint8_t {
  MODE_0_NORMAL             = 0,
  MODE_0_TRANSMISSION       = 0,
  MODE_1_WOR_TRANSMITTER    = 1,
  MODE_1_WOR                = 1,
  MODE_2_WOR_RECEIVER       = 2,
  MODE_2_POWER_SAVING       = 2,
  MODE_3_CONFIGURATION      = 3,
  MODE_3_PROGRAM            = 3,
  MODE_3_SLEEP              = 3,
  MODE_INIT                 = 0xFF
};

enum class ProgramCommand : uint8_t {
  WRITE_CFG_PWR_DWN_SAVE    = 0xC0,
  READ_CONFIGURATION        = 0xC1,
  WRITE_CFG_PWR_DWN_LOSE    = 0xC2,
  WRONG_FORMAT              = 0xFF,
  RETURNED_COMMAND          = 0xC1,
  SPECIAL_WIFI_CONF_COMMAND = 0xCF
};

enum class RegisterAddress : uint8_t {
  REG_ADDRESS_CFG           = 0x00,
  REG_ADDRESS_SPED          = 0x02,
  REG_ADDRESS_TRANS_MODE    = 0x03,
  REG_ADDRESS_CHANNEL       = 0x04,
  REG_ADDRESS_OPTION        = 0x05,
  REG_ADDRESS_CRYPT         = 0x06,
  REG_ADDRESS_PID           = 0x08
};

enum class PacketLength : uint8_t {
  PL_CONFIGURATION          = 0x08,

  PL_SPED                   = 0x01,
  PL_OPTION                 = 0x01,
  PL_TRANSMISSION_MODE      = 0x01,
  PL_CHANNEL                = 0x01,
  PL_CRYPT                  = 0x02,
  PL_PID                    = 0x03
};


struct __attribute__((packed)) Speed {
  uint8_t airDataRate   :3; // bit 0-2
  uint8_t uartParity    :2; // bit 3-4
  uint8_t uartBaudRate  :3; // bit 5-7

  std::string GetAirDataRateDescription() {
    return GetAirDataRateDescriptionByParams((AirDataRate)this->airDataRate);
  }

  std::string GetUARTParityDescription() {
    return GetUARTParityDescriptionByParams((E220UartParity)this->uartParity);
  }

  std::string GetUARTBaudRateDescription() {
    return GetUARTBaudRateDescriptionByParams((UartBpsType)this->uartBaudRate);
  }
};

struct __attribute__((packed)) TransmissionMode {
  uint8_t WORPeriod         :3; // bit 2,1,0EByteLoRaE220::EByteLoRaE220 () {
  // TODO Auto-generated constructor stub
  uint8_t reserved2         :1; // bit 3
  uint8_t enableLBT         :1; // bit 4
  uint8_t reserved          :1; // bit 5
  uint8_t fixedTransmission :1; // bit 6
  uint8_t enableRSSI        :1; // bit 7

  std::string GetWORPeriodByParamsDescription() {
    return GetWORPeriodByParams((WorPeriod)this->WORPeriod);
  }

  std::string GetLBTEnableByteDescription() {
    return GetLBTEnableByteByParams((LbtEnableByte)this->enableLBT);
  }

  std::string GetFixedTransmissionDescription() {
    return GetFixedTransmissionDescriptionByParams((FixedTransmission)this->fixedTransmission);
  }

  std::string GetRSSIEnableByteDescription() {
    return GetRSSIEnableByteByParams((RssiEnableByte)this->enableRSSI);
  }
};

struct __attribute__((packed)) Option {
  uint8_t transmissionPower :2; //bit 0-1
  std::string GetTransmissionPowerDescription() {
    return GetTransmissionPowerDescriptionByParams((TransmissionPower)this->transmissionPower);
  }
  uint8_t reserved :3; //bit 2-4

  uint8_t RSSIAmbientNoise :1; //bit 5
  std::string GetRSSIAmbientNoiseEnable() {
    return GetRSSIAmbientNoiseEnableByParams((RssiAmbientNoiseEnable)this->RSSIAmbientNoise);
  }

  uint8_t subPacketSetting :2; //bit 6-7
  std::string GetSubPacketSetting() {
    return GetSubPacketSettingByParams((SubPacketSetting)this->subPacketSetting);
  }

};

struct __attribute__((packed)) Crypt {
  uint8_t CryptH = 0;
  uint8_t CryptL = 0;
};

struct __attribute__((packed)) Configuration {
  uint8_t Command {};
  uint8_t StartAddress {};
  uint8_t Lenght {};

  uint8_t AddrH {};
  uint8_t AddrL {};

  struct Speed SpeeD;
  struct Option OptionN;

  uint8_t Channel = 0;
  std::string GetChannelDescription() {
    std::stringstream ret;
    ret << (this->Channel + OPERATING_FREQUENCY) << " MHz";
    return ret.str();
  }

  struct TransmissionMode TransMode;

  struct Crypt CRYPT;
};

struct __attribute__((packed)) ModuleInformation {
  uint8_t Command {};
  uint8_t StartingAddress {};
  uint8_t Length {};

  uint8_t model {};
  uint8_t version {};
  uint8_t features {};
};

struct __attribute__((packed)) ResponseStatus {
  Status code;
  std::string GetResponseDescription() {
    return GetResponseDescriptionByParams(this->code);
  }
};

struct __attribute__((packed)) ResponseStructContainer {
  void    *       data;
  uint8_t         rssi;
  ResponseStatus  status;

  void Close() {
    delete (uint8_t*)this->data;
  }
};
struct /*__attribute__((packed))*/ ResponseContainer {
  std::string     data;
  uint8_t         rssi;
  ResponseStatus  status;
};

struct __attribute__((packed)) ConfigurationMessage {
  uint8_t specialCommand1 = 0xCF;
  uint8_t specialCommand2 = 0xCF;

  uint8_t message[];
};

constexpr uint32_t MaxSizeTxPacket = 200;

class EByteLoRaE220 final {
public:
  EByteLoRaE220(Airsoft::Drivers::Uarts * serial, UartBpsRate bpsRate = UartBpsRate::UART_BPS_RATE_9600);
  EByteLoRaE220(Airsoft::Drivers::Uarts * serial, uint32_t auxPin, UartBpsRate bpsRate = UartBpsRate::UART_BPS_RATE_9600);
  EByteLoRaE220(Airsoft::Drivers::Uarts * serial, uint32_t auxPin, uint32_t m0Pin, uint32_t m1Pin, UartBpsRate bpsRate = UartBpsRate::UART_BPS_RATE_9600);
  virtual ~EByteLoRaE220() = default;

public:
  bool Begin(void);
  Status SetMode(ModeType mode);
  ModeType GetMode();

  ResponseStructContainer GetConfiguration(void);
  ResponseStatus SetConfiguration(Configuration configuration, ProgramCommand saveType = ProgramCommand::WRITE_CFG_PWR_DWN_LOSE);

  ResponseStructContainer GetModuleInformation(void);
  ResponseStatus ResetModule(void);

  ResponseStatus SendMessage(const void *message, const uint8_t size);

  ResponseContainer ReceiveMessageUntil(std::string delimiter = std::string("\0"));
  ResponseStructContainer ReceiveMessage(const uint8_t size);
  ResponseStructContainer ReceiveMessageRSSI(const uint8_t size);

  ResponseStructContainer ReceiveMessageComplete(const uint8_t size, bool enableRSSI);
  ResponseContainer ReceiveMessageComplete(bool enableRSSI);

  ResponseStatus SendMessage(const std::string message);
  ResponseContainer ReceiveMessage(void);
  ResponseContainer ReceiveMessageRSSI(void);

  ResponseStatus SendFixedMessage(uint8_t addrH, uint8_t addrL, uint8_t Channel, const std::string message);

  ResponseStatus SendFixedMessage(uint8_t addrH,uint8_t addrL, uint8_t Channel, const void * message, const uint8_t size);
  ResponseStatus SendBroadcastFixedMessage(uint8_t Channel, const void *message, const uint8_t size);
  ResponseStatus SendBroadcastFixedMessage(uint8_t Channel, const std::string message);

  ResponseContainer ReceiveInitialMessage(const uint8_t size);

  ResponseStatus SendConfigurationMessage(uint8_t addrH, uint8_t addrL, uint8_t Channel, Configuration * configuration,
                                          ProgramCommand programCommand = ProgramCommand::WRITE_CFG_PWR_DWN_SAVE);

  /**
   * @brief Method to indicate availability
   * @return Amount of available data
   */
  int32_t Available(void);


private:
  Airsoft::Drivers::Uarts * _serial {};

  bool _isSoftwareSerial { false };

  int32_t                   _auxPin { -1 };
  Airsoft::Drivers::Gpio *  _auxGpio {};
  int32_t                   _m0Pin { -1 };
  Airsoft::Drivers::Gpio *  _m0Gpio {};
  int32_t                   _m1Pin { -1 };
  Airsoft::Drivers::Gpio *  _m1Gpio {};

  UartBpsRate _bpsRate = UartBpsRate::UART_BPS_RATE_9600;

  ModeType _mode = ModeType::MODE_0_NORMAL;

  uint64_t _halfKeyloqKey { 0x06660708 };

private:
  uint64_t Encrypt(uint64_t data);
  uint64_t Decrypt(uint64_t data);

  /**
   * @brief Utility method to wait until module is done transmitting a timeout is provided to avoid an infinite loop
   * @param timeout - Timeout in milliseconds
   * @param waitNoAux - Timeout in milliseconds if aux pin is not used
   * @return E220_SUCCESS if command complete successfully
   */
  Status WaitCompleteResponse(uint64_t timeout = 1000, uint32_t waitNoAux = 100);

  void Flush(void);
  void CleanUARTBuffer(void);

  Status SendStruct(void * structureManaged, size_t size_);
  Status ReceiveStruct(void * structureManaged, size_t size_);
  bool WriteProgramCommand(ProgramCommand cmd, RegisterAddress addr, PacketLength pl);

  RESPONSE_STATUS CheckUARTConfiguration(ModeType mode);

#ifdef LoRa_E220_DEBUG
  void PrintParameters(struct Configuration * configuration);
#endif

};

} // namespace Airsoft::Devices

#endif // _DEVICES_EBYTELORAE220_HPP_
