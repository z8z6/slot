#include "Core/Event.h"

#include <WindowsX.h>

using namespace z8;

MouseMovArgs::MouseMovArgs(WPARAM wParam, LPARAM lParam, int deltaX,
                           int deltaY, MouseButton button)
    : State(static_cast<unsigned>(wParam)), X(GET_X_LPARAM(lParam)),
      Y(GET_Y_LPARAM(lParam)), DeltaX(deltaX), DeltaY(deltaY), Button(button) {}

KeyArgs::KeyArgs(WPARAM wParam, LPARAM lParam)
    : Key(static_cast<unsigned>(wParam)),
      RepeatCount(static_cast<unsigned>(lParam & 0xffff)),
      ScanCode(static_cast<unsigned>((lParam >> 16) & 0xff)),
      IsExtended((lParam & (1LL << 24)) != 0),
      WasDown((lParam & (1LL << 30)) != 0) {}
