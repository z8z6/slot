#include "Core/Application.h"

using namespace z8;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, int) {
  // 开控制台窗口
  AllocConsole();
  freopen("CONOUT$", "w", stdout);
  freopen("CONOUT$", "w", stderr);

  Window::Instance = hInstance;
  Application App1;
  // Application App2;
  return Application::Run();
}