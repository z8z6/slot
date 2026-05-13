//
// Created by zhou_zhengming on 2026/5/13.
//

#include "Core/Event.h"
#include <WindowsX.h>

using namespace z8;

z8::ButtonEventArgs::ButtonEventArgs(WPARAM wParam, LPARAM lParam)
  : State(wParam),
    X(GET_X_LPARAM(lParam)),
    Y(GET_Y_LPARAM(lParam))
{
}
