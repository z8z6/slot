#include "Core/App.h"
#include "Core/Window.h"

using namespace z8;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, int) {
  Window::Instance = hInstance;
  Window w1;
  Window w2;
  w1.Open();
  w2.Open();
  return Window::Run();
}