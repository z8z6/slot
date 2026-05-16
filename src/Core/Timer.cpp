//
// Created by zhou_zhengming on 2026/5/10.
//

#include <windows.h>
#include "Core/Timer.h"

using namespace z8;

z8::Timer::Timer() : TimeCost(0), TimeTotal(0), TimePrev(0), TimeCur(0)
{
  __int64 countsPerSec;
  QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&countsPerSec));
  SecondsPerCount = 1.0 / static_cast<double>(countsPerSec);
}

void Timer::Reset()
{
  QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&TimeCur));
  TimePrev = TimeCur;
  TimeCost = 0;
  TimeTotal = 0;
}

void Timer::Tick()
{
  static bool isFirstTime = true;
  QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&TimeCur));
  if (isFirstTime)
  {
    isFirstTime = false;
    TimePrev = TimeCur;
  }

  TimeCost = static_cast<double>(TimeCur - TimePrev) * SecondsPerCount;
  TimeTotal += static_cast<double>(TimeCur - TimePrev) * SecondsPerCount;
  if (TimeCost < 0) TimeCost = 0;

  TimePrev = TimeCur;
}