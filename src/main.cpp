#include "Core/App.h"
#include "Core/Window.h"

using namespace z8;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine,
                   int showCmd) {
  Window w(hInstance);
  return w.Run();
}