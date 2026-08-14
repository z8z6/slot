#pragma once

#include "UI/Dock/DockTree.h"

#include <memory>
#include <vector>

namespace z8 {
class DX12Render;

namespace ui {
class BaseNode;
}

/**
 * 把 Layout 中的 Floating Dock item 投影为独立 Win32 顶层窗口。
 *
 * 控件所有权、Dock 状态和输入状态仍属于主 Layout；每个宿主只拥有 HWND、
 * 交换链及设备资源。SceneNode 额外复用 GOBatch 和独立深度目标。
 */
class DX12FloatingWindowManager final {
private:
  class Host;

  DX12Render *Render = nullptr;
  std::vector<std::unique_ptr<Host>> Hosts;

public:
  explicit DX12FloatingWindowManager(DX12Render &render);
  ~DX12FloatingWindowManager();

  void Draw();
  /** 同步 Floating 集合；拓扑变化时重建宿主的非拥有渲染批次。 */
  /** 返回本帧最终是否发生拓扑变化，主 UI 批次必须据此同步重建。 */
  bool Reconcile(bool topologyChanged);
  void Update();
};

} // namespace z8
