/*
 * PMain.cpp
 *
 *  Created on: Dec 12, 2024
 *      Author: ccroci
 */

#include "p-main.hpp"

namespace Airsoft::Pages {

//======================================================================================================================
// Implement DisplayPage Interface *************************************************************************************
//======================================================================================================================

/**
 * @brief Time for periodic function
 */
constexpr uint32_t _periodicTime = 100;

//-----------------------------------------------------------------------------
bool PMain::Load(Airsoft::Templates::DisplayEngine * engine) {
  // Set Engine
  _engine = engine;

  return true;
}
//-----------------------------------------------------------------------------
void PMain::Refresh(void) {

}
//-----------------------------------------------------------------------------
void PMain::KeyHandle(const char key, const uint8_t keyCode) {

}
//-----------------------------------------------------------------------------
void PMain::Periodic(void) {

}
//-----------------------------------------------------------------------------
uint32_t PMain::PeriodicTime(void) const {
  return _periodicTime;
}
//-----------------------------------------------------------------------------
std::string PMain::Name(void) {
  return "Main Page";
}
//-----------------------------------------------------------------------------

} // namespace Airsoft::Pages
