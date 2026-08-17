//
// Created by zhou_zhengming on 2026/5/10.
//

#pragma once
#include <cstdint>

namespace z8 {
class Timer {
  double SecondsPerCount;
  int64_t TimePrev;
  int64_t TimeCur;
public:
  double TimeCost;
  double TimeTotal;

  Timer();

  void Reset();
  void Tick();
};

}



