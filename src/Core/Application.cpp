//
// Created by zhou_zhengming on 2026/5/7.
//

#include "Core/Application.h"

z8::Application::Application() {
  Windows.emplace_back();

  for (auto &W : Windows)
    W.Open();
}