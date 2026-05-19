//
// Created by zhou_zhengming on 2026/5/19.
//

#pragma once
#include "Application.h"

namespace z8 {
class GameApplication : public Application {
  void PrepareScene() override;
public:
  void Init() override;
};
}
