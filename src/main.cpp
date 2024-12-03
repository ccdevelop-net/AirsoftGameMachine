//============================================================================
// Name        : main.cpp
// Author      : Cristian Croci
// Version     : 1.00
// Description : Airsoft multi-player system
//============================================================================
// Airsoft multi-player system
// Copyright (C) 2024 - CCDevelop.NET by Cristian Croci
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//============================================================================

#include <iostream>
#include "AirsoftManager.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  Airsoft::AirsoftManager manager = Airsoft::AirsoftManager();

  if (manager.Init()) {
    while(true) {
      std::string str;
      std::getline(std::cin, str);

      if (str == "quit") {
        manager.Terminate();
        break;
      }
    }
  }

  return 0;
}

