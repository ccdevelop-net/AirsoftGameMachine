/*
 * PMain.cpp
 *
 *  Created on: Dec 12, 2024
 *      Author: ccroci
 */

#include <templates/display-page.hpp>
#include <templates/display-engine.hpp>

#include "p-main.hpp"

namespace Airsoft::Pages {

//======================================================================================================================
// Implement DisplayPage Interface *************************************************************************************
//======================================================================================================================

/**
 * @brief Time for periodic function
 */
constexpr uint32_t _periodicTime = 100;
constexpr char     _scrollText[] = "                    Nemesis Softair Club Cantello                    \0";

//-----------------------------------------------------------------------------
bool PMain::Load(Airsoft::Templates::DisplayEngine * engine) {
  // Set Engine
  _engine = engine;

  // Write welcome screen
  _engine->Clean();
  //                     "                    "
  _engine->PrintAt(0, 0, "Airsoft Game Machine");
  _engine->PrintAt(0, 1, "         by         ");
  _engine->PrintAt(0, 2, "   CCDevelop.NET    ");
  _engine->PrintAt(0, 3, "Press '*' to start  ");

  return true;
}
//-----------------------------------------------------------------------------
void PMain::Refresh(void) {

}
//-----------------------------------------------------------------------------
void PMain::KeyHandle(const char key, const uint8_t keyCode) {
  if (!_isStarted) {
    if (key == '*') {
      _start = true;
    }
  }
}
//-----------------------------------------------------------------------------
void PMain::Periodic(void) {
  if (!_isStarted && _start) {
    _isStarted = true;
    _start = false;

    // Write welcome screen
    //                     "                    "
    _engine->Clean();
    _engine->PrintAt(0, 2, "Selezionare Game    ");
    _engine->PrintAt(0, 3, "Usare i tasti * e # ");
  }

  if (_isStarted) {
    char  toScroll[21];
    static uint32_t step {};

    if (++step & 0x01) {
      if (_scrollPosition >= strlen(_scrollText) - 20) {
        _scrollPosition = 0;
        step = 0;
      }
      memcpy(toScroll, &_scrollText[++_scrollPosition], 20);
      toScroll[20] = 0;

      _engine->PrintAt(0, 0, toScroll);
    }
  }

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
