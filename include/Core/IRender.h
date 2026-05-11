//
// Created by zhou_zhengming on 2026/5/9.
//

#pragma once

namespace z8 {
class IRender {
public:
  virtual ~IRender() = default;
  virtual void Init() = 0;
  virtual void Update() = 0;
  virtual void Draw() = 0;
  virtual void Resize() = 0;
};
}


