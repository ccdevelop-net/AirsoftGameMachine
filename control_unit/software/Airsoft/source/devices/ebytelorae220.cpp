/*
 * ebytelorae220.cpp
 *
 *  Created on: Dec 5, 2024
 *      Author: ccroci
 */

#include <chrono>
#include <functional>
#include <thread>
#include <iostream>
#include <bitset>

#include <devices/ebytelorae220.hpp>

namespace Airsoft::Devices {

typedef struct __attribute__((packed)) fixedStransmission {
  uint8_t AddrH {};
  uint8_t AddrL {};
  uint8_t Channel {};
  uint8_t message[];
} FixedStransmission;

//-----------------------------------------------------------------------------
FixedStransmission * init_stack(size_t msgSize){
  return (FixedStransmission*)new uint8_t[sizeof(FixedStransmission) + msgSize * sizeof(int32_t)];
}
//-----------------------------------------------------------------------------
ConfigurationMessage * init_stack_conf(size_t msgSize){
  return (ConfigurationMessage*)new uint8_t[sizeof(ConfigurationMessage) + msgSize * sizeof(int32_t)];
}
//-----------------------------------------------------------------------------

constexpr uint32_t KeeLoq_NLF = 0x3A5C742E;

//-----------------------------------------------------------------------------
EByteLoRaE220::EByteLoRaE220(Airsoft::Drivers::Uarts * serial, UartBpsRate bpsRate) {
  _serial = serial;
  _bpsRate = bpsRate;
}
//-----------------------------------------------------------------------------
EByteLoRaE220::EByteLoRaE220(Airsoft::Drivers::Uarts * serial, uint32_t auxPin, UartBpsRate bpsRate) {
  _auxPin = auxPin;

  _serial = serial;
  _bpsRate = bpsRate;
}
//-----------------------------------------------------------------------------
EByteLoRaE220::EByteLoRaE220(Airsoft::Drivers::Uarts * serial, uint32_t auxPin, uint32_t m0Pin, uint32_t m1Pin, UartBpsRate bpsRate) {
  _auxPin = auxPin;
  _m0Pin = m0Pin;
  _m1Pin = m1Pin;

  _serial = serial;
  _bpsRate = bpsRate;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
bool EByteLoRaE220::Begin(void){
  if (_auxPin != -1) {
    _auxGpio = new Airsoft::Drivers::Gpio(_auxPin);
    _auxGpio->Open(Airsoft::Drivers::Direction::Input);
  }

  if (_m0Pin != -1) {
    _m0Gpio = new Airsoft::Drivers::Gpio(_m0Pin);
    _m0Gpio->Open(Airsoft::Drivers::Direction::Output);
  }

  if (_m1Pin != -1) {
    _m1Gpio = new Airsoft::Drivers::Gpio(_m1Pin);
    _m1Gpio->Open(Airsoft::Drivers::Direction::Output);
  }

  // Some time to wait
  std::this_thread::sleep_for(std::chrono::milliseconds(250));

  Airsoft::Drivers::Timeout timeout { Airsoft::Drivers::Timeout::SimpleTimeout(100) };
  _serial->SetTimeout(timeout);
  return SetMode(ModeType::MODE_0_NORMAL) == E220_SUCCESS;
}
//-----------------------------------------------------------------------------
Status EByteLoRaE220::SetMode(ModeType mode) {

  // Datasheet claims module needs some extra time after mode setting (2ms)
  // most of my projects uses 10 ms, but 40ms is safer
  std::this_thread::sleep_for(std::chrono::milliseconds(40));

  if (_m0Pin == -1 && _m1Pin == -1) {
#ifdef LoRa_E220_DEBUG
    std::cout << "The M0 and M1 pins is not set, this mean that you are connect directly the pins as you need!" << std::endl;
#endif
  } else {
    switch (mode) {
      case ModeType::MODE_0_NORMAL:
        // Mode 0 | normal operation
        _m0Gpio->Reset();
        _m1Gpio->Reset();
        break;
      case ModeType::MODE_1_WOR_TRANSMITTER:
        _m0Gpio->Set();
        _m1Gpio->Reset();
        break;
      case ModeType::MODE_2_WOR_RECEIVER:
        _m0Gpio->Reset();
        _m1Gpio->Set();
        break;
      case ModeType::MODE_3_CONFIGURATION:
        // Mode 3 | Setting operation
        _m0Gpio->Set();
        _m1Gpio->Set();
        break;
      default:
        return ERR_E220_INVALID_PARAM;
    }
  }

  // Datasheet says 2ms later control is returned, let's give just a bit more time
  // these modules can take time to activate pins
  std::this_thread::sleep_for(std::chrono::milliseconds(40));

  // Wait until aux pin goes back low
  Status res = WaitCompleteResponse(1000);

  if (res == E220_SUCCESS){
    _mode = mode;
  }

  return res;
}
//-----------------------------------------------------------------------------
ModeType EByteLoRaE220::GetMode(){
  return _mode;
}
//-----------------------------------------------------------------------------
ResponseStructContainer EByteLoRaE220::GetConfiguration(){
  // Function Variables
  ResponseStructContainer rc;

  rc.status.code = CheckUARTConfiguration(ModeType::MODE_3_PROGRAM);
  if (rc.status.code != E220_SUCCESS) {
    return rc;
  }

  ModeType prevMode = _mode;

  rc.status.code = SetMode(ModeType::MODE_3_PROGRAM);
  if (rc.status.code != E220_SUCCESS) {
    return rc;
  }

  WriteProgramCommand(ProgramCommand::READ_CONFIGURATION, RegisterAddress::REG_ADDRESS_CFG, PacketLength::PL_CONFIGURATION);

  rc.data = new Configuration;
  rc.status.code = ReceiveStruct((uint8_t *)rc.data, sizeof(Configuration));

#ifdef LoRa_E220_DEBUG
   PrintParameters((Configuration*)rc.data);
#endif

  if (rc.status.code != E220_SUCCESS) {
    SetMode(prevMode);
    return rc;
  }

  rc.status.code = SetMode(prevMode);
  if (rc.status.code!=E220_SUCCESS) {
    return rc;
  }

  if (((Configuration *)rc.data)->Command == static_cast<uint8_t>(ProgramCommand::WRONG_FORMAT)){
    rc.status.code = ERR_E220_WRONG_FORMAT;
  }
  if (((Configuration *)rc.data)->Command != static_cast<uint8_t>(ProgramCommand::RETURNED_COMMAND) ||
      ((Configuration *)rc.data)->StartAddress != static_cast<uint8_t>(RegisterAddress::REG_ADDRESS_CFG) ||
      ((Configuration *)rc.data)->Lenght != static_cast<uint8_t>(PacketLength::PL_CONFIGURATION)){
    rc.status.code = ERR_E220_HEAD_NOT_RECOGNIZED;
  }

  return rc;
}
//-----------------------------------------------------------------------------
ResponseStatus EByteLoRaE220::SetConfiguration(Configuration configuration, ProgramCommand saveType){
  ResponseStatus rc;

  rc.code = CheckUARTConfiguration(ModeType::MODE_3_PROGRAM);
  if (rc.code != E220_SUCCESS) {
    return rc;
  }

  ModeType prevMode = _mode;

  rc.code = SetMode(ModeType::MODE_3_PROGRAM);
  if (rc.code != E220_SUCCESS) {
    return rc;
  }

  configuration.Command = static_cast<uint8_t>(saveType);
  configuration.StartAddress = static_cast<uint8_t>(RegisterAddress::REG_ADDRESS_CFG);
  configuration.Lenght = static_cast<uint8_t>(PacketLength::PL_CONFIGURATION);

  rc.code = SendStruct((uint8_t *)&configuration, sizeof(Configuration));
  if (rc.code != E220_SUCCESS) {
    SetMode(prevMode);
    return rc;
  }

  rc.code = ReceiveStruct((uint8_t *)&configuration, sizeof(Configuration));

#ifdef LoRa_E220_DEBUG
  PrintParameters((Configuration *)&configuration);
#endif

  rc.code = this->SetMode(prevMode);
  if (rc.code != E220_SUCCESS) {
    return rc;
  }

  if (((Configuration*)&configuration)->Command == static_cast<uint8_t>(ProgramCommand::WRONG_FORMAT)){
    rc.code = ERR_E220_WRONG_FORMAT;
  }

  if (((Configuration*)&configuration)->Command != static_cast<uint8_t>(ProgramCommand::RETURNED_COMMAND) ||
      ((Configuration*)&configuration)->StartAddress != static_cast<uint8_t>(RegisterAddress::REG_ADDRESS_CFG) ||
      ((Configuration*)&configuration)->Lenght != static_cast<uint8_t>(PacketLength::PL_CONFIGURATION)){
    rc.code = ERR_E220_HEAD_NOT_RECOGNIZED;
  }

  return rc;
}
//-----------------------------------------------------------------------------
ResponseStructContainer EByteLoRaE220::GetModuleInformation(void){
  ResponseStructContainer rc;

  rc.status.code = CheckUARTConfiguration(ModeType::MODE_3_PROGRAM);
  if (rc.status.code != E220_SUCCESS) {
    return rc;
  }

  ModeType prevMode = _mode;

  rc.status.code = SetMode(ModeType::MODE_3_PROGRAM);
  if (rc.status.code != E220_SUCCESS) {
    return rc;
  }

  WriteProgramCommand(ProgramCommand::READ_CONFIGURATION, RegisterAddress::REG_ADDRESS_PID, PacketLength::PL_PID);

  rc.data = new ModuleInformation;
  rc.status.code = ReceiveStruct((uint8_t *)rc.data, sizeof(ModuleInformation));
  if (rc.status.code!=E220_SUCCESS) {
    SetMode(prevMode);
    return rc;
  }

  rc.status.code = SetMode(prevMode);
  if (rc.status.code != E220_SUCCESS) {
    return rc;
  }

  if (((ModuleInformation *)rc.data)->Command == static_cast<uint8_t>(ProgramCommand::WRONG_FORMAT)){
    rc.status.code = ERR_E220_WRONG_FORMAT;
  }
  if (((ModuleInformation *)rc.data)->Command != static_cast<uint8_t>(ProgramCommand::RETURNED_COMMAND) ||
      ((ModuleInformation *)rc.data)->StartingAddress != static_cast<uint8_t>(RegisterAddress::REG_ADDRESS_PID) ||
      ((ModuleInformation *)rc.data)->Length != static_cast<uint8_t>(PacketLength::PL_PID)){
    rc.status.code = ERR_E220_HEAD_NOT_RECOGNIZED;
  }

#ifdef LoRa_E220_DEBUG
  std::cout << "----------------------------------------" << std::endl;
  std::cout << "HEAD: " << std::bitset<8>(((ModuleInformation *)rc.data)->Command) << " " <<
      ((ModuleInformation *)rc.data)->StartingAddress << " " << ((ModuleInformation *)rc.data)->Length  << std::endl;
  std::cout << "Model no.: " << ((ModuleInformation *)rc.data)->model << std::endl;
  std::cout << "Version  : " << ((ModuleInformation *)rc.data)->version << std::endl;
  std::cout << "Features : " << ((ModuleInformation *)rc.data)->features << std::endl;
  std::cout << "Status : " << rc.status.GetResponseDescription() << std::endl;
  std::cout << "----------------------------------------"  << std::endl;
#endif

  return rc;
}

//-----------------------------------------------------------------------------
ResponseStatus EByteLoRaE220::ResetModule(){
#ifdef LoRa_E220_DEBUG
  std::cout << "No information to reset module!" << std::endl;
#endif
  ResponseStatus status;
  status.code = ERR_E220_NOT_IMPLEMENT;
  return status;
}
//-----------------------------------------------------------------------------
ResponseContainer EByteLoRaE220::ReceiveMessage(void){
  return ReceiveMessageComplete(false);
}
//-----------------------------------------------------------------------------
ResponseContainer EByteLoRaE220::ReceiveMessageRSSI(){
  return ReceiveMessageComplete(true);
}
//-----------------------------------------------------------------------------
ResponseContainer EByteLoRaE220::ReceiveMessageComplete(bool rssiEnabled){
  ResponseContainer rc;
  rc.status.code = E220_SUCCESS;
  std::string tmpData = _serial->Read(255);

#ifdef LoRa_E220_DEBUG
  if (tmpData.length() > 0) {
    std::cout << tmpData << std::endl;
  }
#endif

  if (rssiEnabled) {
    rc.rssi = tmpData.at(tmpData.length() - 1);
    rc.data = tmpData.substr(0, tmpData.length() - 1);
  } else {
    rc.data = tmpData;
  }

  CleanUARTBuffer();

  if (rc.status.code != E220_SUCCESS) {
    return rc;
  }

  return rc;
}
//-----------------------------------------------------------------------------
ResponseContainer EByteLoRaE220::ReceiveMessageUntil(std::string delimiter){
  ResponseContainer rc;
  std::string message;

  rc.status.code = E220_SUCCESS;
  _serial->ReadLine(message, 0xFFFF, delimiter);

  //rc.data

  if (rc.status.code!=E220_SUCCESS) {
    return rc;
  }



  return rc;
}
//-----------------------------------------------------------------------------
ResponseContainer EByteLoRaE220::ReceiveInitialMessage(uint8_t size){
  ResponseContainer rc;
  std::unique_ptr<char> dataRead(new char[size]);

  rc.status.code = E220_SUCCESS;

  size_t len = _serial->Read((uint8_t*)dataRead.get(), size);

  if (len != size) {
    if (len == 0) {
      rc.status.code = ERR_E220_NO_RESPONSE_FROM_DEVICE;
    } else {
      rc.status.code = ERR_E220_DATA_SIZE_NOT_MATCH;
    }

    return rc;
  }

  rc.data = dataRead.get();

  return rc;
}
//-----------------------------------------------------------------------------
ResponseStructContainer EByteLoRaE220::ReceiveMessage(const uint8_t size){
  return ReceiveMessageComplete(size, false);
}
//-----------------------------------------------------------------------------
ResponseStructContainer EByteLoRaE220::ReceiveMessageRSSI(const uint8_t size){
  return ReceiveMessageComplete(size, true);
}
//-----------------------------------------------------------------------------
ResponseStructContainer EByteLoRaE220::ReceiveMessageComplete(const uint8_t size, bool rssiEnabled){
  ResponseStructContainer rc;

  rc.data = new uint8_t[size];
  rc.status.code = ReceiveStruct((uint8_t*)rc.data, size);

  if (rc.status.code != E220_SUCCESS) {
    return rc;
  }

  if (rssiEnabled) {
    uint8_t rssi[1];
    _serial->Read(rssi, 1);
    rc.rssi = rssi[0];
  }

  CleanUARTBuffer();

  return rc;
}
//-----------------------------------------------------------------------------
ResponseStatus EByteLoRaE220::SendMessage(const void *message, const uint8_t size){
  ResponseStatus status;

  status.code = SendStruct((uint8_t*)message, size);

  return status;
}
//-----------------------------------------------------------------------------
ResponseStatus EByteLoRaE220::SendMessage(const std::string message){
#ifdef LoRa_E220_DEBUG
  std::cout << "Send message: " << message << std::endl;
#endif
  size_t size = message.length();
#ifdef LoRa_E220_DEBUG
  std::cout << " size: " << size << std::endl;
#endif
  char messageFixed[size];

  memcpy(messageFixed, message.c_str(), size);
#ifdef LoRa_E220_DEBUG
  std::cout << " memcpy " << std::endl;
#endif

  ResponseStatus status;

  status.code = SendStruct((uint8_t *)&messageFixed, size);

  return status;
}
//-----------------------------------------------------------------------------
ResponseStatus EByteLoRaE220::SendFixedMessage(uint8_t addrH, uint8_t addrL, uint8_t channel, const std::string message){
  size_t size = message.length();

  char messageFixed[size];

  memcpy(messageFixed,message.c_str(),size);

  return SendFixedMessage(addrH, addrL, channel, (uint8_t*)messageFixed, size);
}
//-----------------------------------------------------------------------------
ResponseStatus EByteLoRaE220::SendBroadcastFixedMessage(uint8_t CHAN, const std::string message){
  return SendFixedMessage(BROADCAST_ADDRESS, BROADCAST_ADDRESS, CHAN, message);
}
//-----------------------------------------------------------------------------
ResponseStatus EByteLoRaE220::SendFixedMessage(uint8_t addrH, uint8_t addrL, uint8_t channel, const void * message, const uint8_t size){
#ifdef LoRa_E220_DEBUG
  std::cout << "Address H: " << addrH << std::endl;
#endif

  FixedStransmission *fixedStransmission = init_stack(size);

  fixedStransmission->AddrH = addrH;
  fixedStransmission->AddrL = addrL;
  fixedStransmission->Channel = channel;


  memcpy(fixedStransmission->message, (unsigned char*)message, size);

  ResponseStatus status;
  status.code = SendStruct((uint8_t *)fixedStransmission, size+3);

  delete fixedStransmission;

  return status;
}
//-----------------------------------------------------------------------------
ResponseStatus EByteLoRaE220::SendConfigurationMessage(uint8_t addrH, uint8_t addrL, uint8_t channel, Configuration * configuration, ProgramCommand programCommand){
  ResponseStatus rc;

  configuration->Command = static_cast<uint8_t>(programCommand);
  configuration->StartAddress = static_cast<uint8_t>(RegisterAddress::REG_ADDRESS_CFG);
  configuration->Lenght = static_cast<uint8_t>(PacketLength::PL_CONFIGURATION);

  ConfigurationMessage *fixedStransmission = init_stack_conf(sizeof(Configuration));

  memcpy(fixedStransmission->message, (unsigned char*)configuration, sizeof(Configuration));

  fixedStransmission->specialCommand1 = static_cast<uint8_t>(ProgramCommand::SPECIAL_WIFI_CONF_COMMAND);
  fixedStransmission->specialCommand2 = static_cast<uint8_t>(ProgramCommand::SPECIAL_WIFI_CONF_COMMAND);

#ifdef LoRa_E220_DEBUG
  std::cout << sizeof(Configuration) + 2 << std::endl;
#endif

  rc = SendFixedMessage(addrH, addrL, channel, fixedStransmission, sizeof(Configuration) + 2);

  return rc;
}
//-----------------------------------------------------------------------------
ResponseStatus EByteLoRaE220::SendBroadcastFixedMessage(uint8_t channel, const void *message, const uint8_t size){
  return SendFixedMessage(0xFF, 0xFF, channel, message, size);
}
//-----------------------------------------------------------------------------
int32_t EByteLoRaE220::Available() {
  return _serial->Available();
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
Status EByteLoRaE220::WaitCompleteResponse(uint64_t timeout, uint32_t waitNoAux) {
  // Function variables
  Status result { E220_SUCCESS };
  uint64_t t { timeout };

  // If AUX pin was supplied and look for HIGH state
  // note you can omit using AUX if no pins are available, but you will have to use delay() to let module finish
  if (_auxPin != -1) {
    while (!_auxGpio->Read()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      if (--t == 0) {
        result = ERR_E220_TIMEOUT;
#ifdef LoRa_E220_DEBUG
        std::cout << "Timeout error!" << std::endl;
#endif
        return result;
      }
    }
    std::cout << "AUX HIGH!" << std::endl;
  } else {
    // If you can't use aux pin, use 4K7 pullup with Arduino
    // you may need to adjust this value if transmissions fail
    std::this_thread::sleep_for(std::chrono::milliseconds(waitNoAux));
#ifdef LoRa_E220_DEBUG
    std::cout << "Wait no AUX pin!" << std::endl;
#endif
  }

  // per data sheet control after aux goes high is 2ms so delay for at least that long)
  std::this_thread::sleep_for(std::chrono::milliseconds(2));

  return result;
}
//-----------------------------------------------------------------------------
void EByteLoRaE220::Flush() {
  _serial->Flush();
}
//-----------------------------------------------------------------------------
void EByteLoRaE220::CleanUARTBuffer() {
  // Function Variables
  uint8_t dummy {};

  // Loop all bytes
  while (_serial->Available()) {
    _serial->Read(&dummy, 1);
  }
}
//-----------------------------------------------------------------------------
Status EByteLoRaE220::SendStruct(void *structureManaged, size_t size) {
  if (size > MaxSizeTxPacket + 2){
    return ERR_E220_PACKET_TOO_BIG;
  }

  Status result = E220_SUCCESS;

  size_t len = _serial->Write((uint8_t*)structureManaged, size);
  if (len != size){
#ifdef LoRa_E220_DEBUG
    std::cout << "Send... len: " << len << " size:" << size << std::endl;
#endif
    if (len == 0){
      result = ERR_E220_NO_RESPONSE_FROM_DEVICE;
    } else {
      result = ERR_E220_DATA_SIZE_NOT_MATCH;
    }
  }

  if (result != E220_SUCCESS) {
    return result;
  }

  result = WaitCompleteResponse(5000, 5000);
  if (result != E220_SUCCESS) {
    return result;
  }

#ifdef LoRa_E220_DEBUG
  std::cout << "Clear buffer...";
#endif
  CleanUARTBuffer();

#ifdef LoRa_E220_DEBUG
  std::cout << "ok!" << std::endl;
#endif

  return result;
}
//-----------------------------------------------------------------------------
Status EByteLoRaE220::ReceiveStruct(void * structureManaged, size_t size) {
  Status result = E220_SUCCESS;

  size_t len = _serial->Read((uint8_t*)structureManaged, size);

#ifdef LoRa_E220_DEBUG
  std::cout << "Available buffer: " << len << " structure size: " << size << std::endl;
#endif

  if (len != size){
    if (len == 0) {
      result = ERR_E220_NO_RESPONSE_FROM_DEVICE;
    } else {
      result = ERR_E220_DATA_SIZE_NOT_MATCH;
    }
  }

  if (result != E220_SUCCESS) {
    return result;
  }

  return WaitCompleteResponse(1000);
}
//-----------------------------------------------------------------------------
bool EByteLoRaE220::WriteProgramCommand(ProgramCommand cmd, RegisterAddress addr, PacketLength pl){
    uint8_t command[3] = { static_cast<uint8_t>(cmd), static_cast<uint8_t>(addr), static_cast<uint8_t>(pl)};
    size_t size = _serial->Write(command, 3);

#ifdef LoRa_E220_DEBUG
    std::cout << "Write command size: " << size << std::endl;
#endif

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    return size != 2;
}
//-----------------------------------------------------------------------------
RESPONSE_STATUS EByteLoRaE220::CheckUARTConfiguration(ModeType mode){
  if (mode == ModeType::MODE_3_PROGRAM && _bpsRate != UartBpsRate::UART_BPS_RATE_9600){
    return ERR_E220_WRONG_UART_CONFIG;
  }

  return E220_SUCCESS;
}
//-----------------------------------------------------------------------------
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
uint64_t EByteLoRaE220::Encrypt(uint64_t data) {
  uint64_t x = data;
  uint64_t r;
  int32_t keyBitNo, index;
  uint64_t keyBitVal,bitVal;

  for (r = 0; r < 528; r++) {
    keyBitNo = r & 63;
    if(keyBitNo < 32) {
      keyBitVal = bitRead(_halfKeyloqKey, keyBitNo); // key low
    } else {
      keyBitVal = bitRead(_halfKeyloqKey, keyBitNo - 32);// key hight
    }
    index = 1 * bitRead(x,1) + 2 * bitRead(x,9) + 4 * bitRead(x,20) + 8 * bitRead(x,26) + 16 * bitRead(x,31);
    bitVal = bitRead(x,0) ^ bitRead(x, 16) ^ bitRead(KeeLoq_NLF,index) ^ keyBitVal;
    x = (x>>1) ^ bitVal<<31;
  }
  return x;
}
//-----------------------------------------------------------------------------
uint64_t EByteLoRaE220::Decrypt(uint64_t data) {
  uint64_t x = data;
  uint64_t r;
  int32_t keyBitNo, index;
  uint64_t keyBitVal,bitVal;

  for (r = 0; r < 528; r++) {
    keyBitNo = (15-r) & 63;
    if(keyBitNo < 32) {
      keyBitVal = bitRead(_halfKeyloqKey, keyBitNo); // key low
    } else {
      keyBitVal = bitRead(_halfKeyloqKey, keyBitNo - 32); // key hight
    }
    index = 1 * bitRead(x,0) + 2 * bitRead(x,8) + 4 * bitRead(x,19) + 8 * bitRead(x,25) + 16 * bitRead(x,30);
    bitVal = bitRead(x,31) ^ bitRead(x, 15) ^ bitRead(KeeLoq_NLF,index) ^ keyBitVal;
    x = (x<<1) ^ bitVal;
  }
  return x;
 }
//-----------------------------------------------------------------------------
#ifdef LoRa_E220_DEBUG
void EByteLoRaE220::PrintParameters(struct Configuration *configuration) {
  std::cout << "----------------------------------------" << std::endl;

  std::cout << "HEAD : " << std::hex << (int)configuration->Command << std::dec << " " <<
      std::hex << configuration->StartAddress << std::dec << " " << std::hex << configuration->Lenght << std::dec << std::endl;
  std::cout << " " << std::endl;
  std::cout << "AddH : " << std::hex << (int)configuration->AddrH << std::dec << std::endl;
  std::cout << "AddL : " << std::hex << (int)configuration->AddrL << std::dec << std::endl;
  std::cout << " " << std::endl;
  std::cout << "Channel : " << (int)configuration->Channel << " -> " << configuration->GetChannelDescription();
  std::cout << " " << std::endl;
  std::cout << "SpeedParityBit     : " << std::bitset<8>(configuration->SpeeD.uartParity) << " -> " << configuration->SpeeD.GetUARTParityDescription() << std::endl;
  std::cout << "SpeedUARTDatte     : " << std::bitset<8>(configuration->SpeeD.uartBaudRate) << " -> " << configuration->SpeeD.GetUARTBaudRateDescription() << std::endl;
  std::cout << "SpeedAirDataRate   : " << std::bitset<8>(configuration->SpeeD.airDataRate) << " -> " << configuration->SpeeD.GetAirDataRateDescription() << std::endl;
  std::cout << " " << std::endl;
  std::cout << "OptionSubPacketSett: " << std::bitset<8>(configuration->OptionN.subPacketSetting) << " -> " <<  configuration->OptionN.GetSubPacketSetting() << std::endl;
  std::cout << "OptionTranPower    : " << std::bitset<8>(configuration->OptionN.transmissionPower) << " -> " << configuration->OptionN.GetTransmissionPowerDescription() << std::endl;
  std::cout << "OptionRSSIAmbientNo: " << std::bitset<8>(configuration->OptionN.RSSIAmbientNoise) << " -> " << configuration->OptionN.GetRSSIAmbientNoiseEnable() << std::endl;
  std::cout << " " << std::endl;
  std::cout << "TransModeWORPeriod : " << std::bitset<8>(configuration->TransMode.WORPeriod) << " -> " << configuration->TransMode.GetWORPeriodByParamsDescription() << std::endl;
  std::cout << "TransModeEnableLBT : " << std::bitset<8>(configuration->TransMode.enableLBT) << " -> " << configuration->TransMode.GetLBTEnableByteDescription() << std::endl;
  std::cout << "TransModeEnableRSSI: " << std::bitset<8>(configuration->TransMode.enableRSSI) << " -> " << configuration->TransMode.GetRSSIEnableByteDescription() << std::endl;
  std::cout << "TransModeFixedTrans: " << std::bitset<8>(configuration->TransMode.fixedTransmission) << " -> " << configuration->TransMode.GetFixedTransmissionDescription() << std::endl;

  std::cout << "----------------------------------------" << std::endl;
}
#endif
//-----------------------------------------------------------------------------

} // namespace Airsoft::Devices
