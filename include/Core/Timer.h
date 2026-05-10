//
// Created by zhou_zhengming on 2026/5/10.
//

#pragma once

namespace z8 {
class Timer {
private:
  double SecondsPerCount;
  __int64 TimePrev;
  __int64 TimeCur;
public:
  double TimeCost;
  double TimeTotal;

  Timer();

  void Reset();
  void Tick();
};

}



