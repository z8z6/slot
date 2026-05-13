//
// Created by zhou_zhengming on 2026/5/13.
//
#pragma once
#include <windows.h>


namespace z8
{
struct ButtonEventArgs
{
  unsigned State;
  int X;
  int Y;

  ButtonEventArgs(WPARAM wParam, LPARAM lParam);
};
}
