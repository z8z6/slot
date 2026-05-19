#include "Core/GameApplication.h"
#include "Util/DXException.h"

using namespace z8;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, int) {
  // 开控制台窗口
  AllocConsole();
  freopen("CONOUT$", "w", stdout);
  freopen("CONOUT$", "w", stderr);

  Window::Instance = hInstance;
  GameApplication App1;
  // Application App2;
  App1.Init();

  try {
    return Application::Run();
  } catch (DXException &e) {
    MessageBoxW(nullptr, e.toString().c_str(), L"HR Failed", MB_OK);
    return 0;
  }
}