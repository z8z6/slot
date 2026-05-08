#include "Core/Application.h"

using namespace z8;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, int) {
  Window::Instance = hInstance;
  Application App;
  return Window::Run();
}