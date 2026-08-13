#pragma once

#include "UI/Layout/PanelNode.h"

#include <cstddef>
#include <deque>
#include <string>

namespace z8::ui {

/**
 * 编辑器消息输出面板。
 *
 * TerminalNode 复用 Panel 的标题、滚动和 Dock 能力，但只维护一个稳定 TextNode
 * 作为输出表面。追加消息不会改变 UI 拓扑，指针捕获期间也无需重建布局索引。
 */
class TerminalNode final : public PanelNode {
public:
  TerminalNode();

  const char *TypeName() const override { return "Terminal"; }
  /** 追加一条英文运行时消息，并按 MaxLines 丢弃最早记录。 */
  void AppendMessage(std::string message);
  /** 清空全部消息但保留输出 TextNode，避免造成拓扑变化。 */
  void ClearMessages();
  /** 当前保留的消息条数。 */
  size_t MessageCount() const { return Messages.size(); }
  /** 返回渲染通道读取的合并文本。 */
  const std::string &OutputText() const { return OutputNode->Text; }
  /**
   * 在 ScrollNode 得到最新内容范围后应用挂起的滚底请求。
   * 返回值表示子树绝对坐标是否需要在本帧重新传播。
   */
  bool ApplyPendingScroll();

  TextNode *OutputNode = nullptr;
  size_t MaxLines = 200;

private:
  std::deque<std::string> Messages;
  bool ScrollToBottomPending = false;
  /** 重新合并限长消息，并同步文本布局高度供 ScrollNode 计算内容范围。 */
  void SynchronizeOutput();
};

} // namespace z8::ui
