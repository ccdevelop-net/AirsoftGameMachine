/**
 *******************************************************************************
 * @file print.hpp
 *
 * @brief Description
 *
 * @author  Cristian
 *
 * @version 1.00
 *
 * @date Dec 9, 2024
 *
 *******************************************************************************
 * This file is part of the Airsoft project
 * https://github.com/xxxx or http://xxx.github.io.
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
#ifndef CLASSES_TIMER_HPP_
#define CLASSES_TIMER_HPP_

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

namespace Airsoft::Classes {

class Timer {
public:
  Timer(void) = default;
  ~Timer(void) = default;

private:
  Timer(const Timer &);
  Timer & operator =(const Timer &);

private:
  std::atomic_bool active = { true };

public:
  void SetTimeout(std::function<void()> function, int32_t delay);
  void SetInterval(std::function<void()> function, int32_t interval);
  void Stop();
};

void Timer::SetTimeout(std::function<void()> function, int32_t delay) {
  active = true;

  std::thread t([=]() {
    if(!active.load()) {
      return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(delay));

    if(!active.load()) {
      return;
    }

    function();
  });

  t.detach();
}

void Timer::SetInterval(std::function<void()> function, int32_t interval) {
  active = true;
  std::thread t([=]() {
    while(active.load()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(interval));

      if(!active.load()) {
        return;
      }

      function();
    }
  });
  t.detach();
}

void Timer::Stop(void) {
  active = false;
}

} // namespace Airsoft::Classes

#endif  // CLASSES_TIMER_HPP_
