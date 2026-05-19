//
// Created by zhou_zhengming on 2026/5/13.
//

#include "Core/Event.h"
#include <WindowsX.h>

using namespace z8;

z8::MouseMovArgs::MouseMovArgs(WPARAM wParam, LPARAM lParam)
  : State(wParam),
    X(GET_X_LPARAM(lParam)),
    Y(GET_Y_LPARAM(lParam))
{
  if (Init) {
    DeltaX = X - LastX;
    DeltaY = Y - LastY;
  } else {
    Init = true;
  }
  LastX = X;
  LastY = Y;
}

KeyArgs::KeyArgs(WPARAM wParam) : Key(wParam) {}
