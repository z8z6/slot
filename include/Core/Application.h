//
// Created by zhou_zhengming on 2026/5/7.
//

#pragma once

#include "Window.h"

namespace z8
{
class IRender;
class Application {
private:
  Window Wnd;
  IRender* Render;
public:
  Application();
};
}






