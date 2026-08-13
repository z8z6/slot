#include "UI/Layout/TerminalNode.h"

#include "UI/Style/Theme.h"

#include <cmath>
#include <utility>

using namespace z8::ui;

TerminalNode::TerminalNode() {
  SetProperty("Icon", "asset://texture/icons/lucide/terminal.svg");
  SetProperty("Title", "Output Log");

  auto output = std::make_unique<TextNode>();
  OutputNode = output.get();
  OutputNode->Key = "__terminal_output";
  OutputNode->Wrap = true;
  OutputNode->Alignment = TextAlignment::Leading;
  OutputNode->Style.Margin = 2.0f;
  OutputNode->Style.FlexGrow = 0.0f;
  OutputNode->Style.FlexShrink = 0.0f;
  ContentHost()->AddChild(std::move(output));
}

void TerminalNode::AppendMessage(std::string message) {
  if (message.empty())
    return;
  Messages.push_back(std::move(message));
  while (Messages.size() > MaxLines)
    Messages.pop_front();
  SynchronizeOutput();
  // 此时新文本高度已写入布局样式，但滚动范围要到下一次布局后才准确。
  ScrollToBottomPending = true;
}

void TerminalNode::ClearMessages() {
  Messages.clear();
  SynchronizeOutput();
  ScrollToBottomPending = true;
}

bool TerminalNode::ApplyPendingScroll() {
  if (!ScrollToBottomPending)
    return false;
  ScrollToBottomPending = false;
  // 第一次坐标传播已经让 ScrollNode 用新文本高度更新范围。记录旧偏移是为了
  // 只在滚动位置确实变化时重走轻量坐标传播，避免无溢出日志产生额外遍历。
  if (auto *scroll = ScrollAreaNode->GetScrollBehavior()) {
    const float previousOffset = scroll->GetOffsetY();
    scroll->SetOffsetY(scroll->GetMaximumOffsetY());
    return std::abs(previousOffset - scroll->GetOffsetY()) > 0.01f;
  }
  return false;
}

void TerminalNode::SynchronizeOutput() {
  std::string text;
  for (const auto &message : Messages) {
    if (!text.empty())
      text.push_back('\n');
    text += message;
  }
  OutputNode->Text = std::move(text);
  // DirectWrite 文本不参与原生布局测量，因此显式提供逐行高度，让内容超过
  // viewport 后 ScrollBehavior 能得到正确的垂直滚动范围。
  const float lineHeight = Theme::Default().Text.LineHeight;
  OutputNode->Style.Height = lineHeight * static_cast<float>(Messages.size());
}
