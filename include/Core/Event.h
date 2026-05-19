//
// Created by zhou_zhengming on 2026/5/13.
//
#pragma once
#include <windows.h>


namespace z8
{
struct MouseMovArgs
{
  unsigned State;
  int X;
  int Y;
  int DeltaX = 0;  // 鼠标X偏移
  int DeltaY = 0;  // 鼠标Y偏移

  inline static int LastX = 0;
  inline static int LastY = 0;
  inline static bool Init = false;

  MouseMovArgs(WPARAM wParam, LPARAM lParam);
};

struct KeyArgs {
  unsigned Key;

  KeyArgs(WPARAM wParam);
};
}
