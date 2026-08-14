#include "UI/Layout/TextInputNode.h"

#include "UI/Property/PropertyParser.h"
#include "UI/Style/Theme.h"

#include <algorithm>
#include <windows.h>

using namespace z8::ui;
using z8::EventReply;

namespace {

bool IsContinuation(unsigned char value) { return (value & 0xC0U) == 0x80U; }

size_t NextCodePoint(const std::string &text, size_t offset) {
  if (offset >= text.size())
    return text.size();
  ++offset;
  while (offset < text.size() &&
         IsContinuation(static_cast<unsigned char>(text[offset])))
    ++offset;
  return offset;
}

size_t PreviousCodePoint(const std::string &text, size_t offset) {
  if (offset == 0 || text.empty())
    return 0;
  --offset;
  while (offset > 0 && IsContinuation(static_cast<unsigned char>(text[offset])))
    --offset;
  return offset;
}

std::string ToUtf8(wchar_t character) {
  char buffer[4]{};
  const int length =
      WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &character, 1, buffer,
                          static_cast<int>(sizeof(buffer)), nullptr, nullptr);
  return length > 0 ? std::string(buffer, static_cast<size_t>(length))
                    : std::string{};
}

size_t Utf8CodePointCount(const std::string &text, size_t end) {
  size_t count = 0;
  for (size_t offset = 0; offset < (std::min)(end, text.size());
       offset = NextCodePoint(text, offset))
    ++count;
  return count;
}

} // namespace

TextInputNode::TextInputNode() {
  const auto &style = Theme::Default().TextInput;
  Focusable = true;
  Style.Direction = FlexDirection::Row;
  Style.Height = style.ControlHeight;
  Style.MinWidth = style.MinimumWidth;
  Style.MinHeight = style.ControlHeight;
  Style.Padding = style.ContentPadding;
  // 单行输入框的高度是主题契约；宽度仍由父容器交叉轴确定，避免它在
  // Column 表单中纵向填满所有剩余空间。
  Style.FlexGrow = 0.0f;
  Style.FlexShrink = 0.0f;
  ClipChildren = true;
  SetCornerRadius(style.CornerRadius);

  auto display = std::make_unique<TextNode>();
  DisplayNode = display.get();
  DisplayNode->Key = "__text";
  DisplayNode->Style.Margin = 0.0f;
  DisplayNode->Style.MinHeight = 0.0f;
  DisplayNode->Style.FlexGrow = 1.0f;
  BaseNode::AddChild(std::move(display));

  auto caret = std::make_unique<RectNode>();
  CaretNode = caret.get();
  CaretNode->Key = "__caret";
  CaretNode->HitTestVisible = false;
  CaretNode->Style.Position = PositionType::Absolute;
  CaretNode->Style.MinWidth = 0.0f;
  CaretNode->Style.MinHeight = 0.0f;
  CaretNode->Style.Margin = 0.0f;
  CaretNode->SetColor(style.CaretColor);
  BaseNode::AddChild(std::move(caret));
  OnVisualStateChanged();
}

bool TextInputNode::ConsumeChanged() {
  const bool result = ChangePending;
  ChangePending = false;
  return result;
}

void TextInputNode::NotifyChanged() {
  ChangePending = true;
  RefreshTextVisual();
  if (TextChanged)
    TextChanged(Text);
}

EventReply TextInputNode::OnKeyDown(KeyArgs args) {
  if (!Enabled)
    return EventReply::Ignored;
  if (args.Key == VK_LEFT) {
    CaretOffset = PreviousCodePoint(Text, CaretOffset);
    return EventReply::Handled;
  }
  if (args.Key == VK_RIGHT) {
    CaretOffset = NextCodePoint(Text, CaretOffset);
    return EventReply::Handled;
  }
  if (args.Key == VK_HOME) {
    CaretOffset = 0;
    return EventReply::Handled;
  }
  if (args.Key == VK_END) {
    CaretOffset = Text.size();
    return EventReply::Handled;
  }
  if (args.Key == VK_BACK) {
    const size_t previous = PreviousCodePoint(Text, CaretOffset);
    if (previous != CaretOffset) {
      Text.erase(previous, CaretOffset - previous);
      CaretOffset = previous;
      NotifyChanged();
    }
    return EventReply::Handled;
  }
  if (args.Key == VK_DELETE) {
    const size_t next = NextCodePoint(Text, CaretOffset);
    if (next != CaretOffset) {
      Text.erase(CaretOffset, next - CaretOffset);
      NotifyChanged();
    }
    return EventReply::Handled;
  }
  if (args.Key == VK_RETURN) {
    if (!args.WasDown && Submitted)
      Submitted(Text);
    return EventReply::Handled;
  }
  // 可打印键的实际字符稍后由 WM_CHAR 给出；先消费 KeyDown，避免编辑器快捷键
  // 在 TextInput 获得焦点时与文字输入同时触发。
  return args.Key >= VK_SPACE && args.Key <= 0xFE ? EventReply::Handled
                                                  : EventReply::Ignored;
}

EventReply TextInputNode::OnMouseDown(MouseMovArgs args) {
  if (!Enabled || args.Button != MouseButton::Left || !Contains(args))
    return EventReply::Ignored;
  // 当前文本通道没有逐字形命中结果；点击先落到末尾，键盘方向键仍可精确按
  // UTF-8 code point 移动，避免用字节偏移切断多字节字符。
  CaretOffset = Text.size();
  return EventReply::Capture;
}

EventReply TextInputNode::OnMouseUp(MouseMovArgs) {
  return EventReply::Handled;
}

EventReply TextInputNode::OnTextInput(wchar_t character) {
  if (!Enabled || character < L' ' || character == 0x7F)
    return EventReply::Ignored;
  const auto utf8 = ToUtf8(character);
  if (utf8.empty())
    return EventReply::Ignored;
  Text.insert(CaretOffset, utf8);
  CaretOffset += utf8.size();
  NotifyChanged();
  return EventReply::Handled;
}

void TextInputNode::OnVisualStateChanged() {
  const auto &style = Theme::Default().TextInput;
  const auto state = !Enabled  ? WidgetVisualState::Disabled
                     : Focused ? WidgetVisualState::Focused
                     : Hovered ? WidgetVisualState::Hovered
                               : WidgetVisualState::Normal;
  SetColor(style.BackgroundColor.Resolve(state));
  SetBorder(Focused && Enabled ? style.FocusedBorderColor : style.BorderColor,
            style.BorderWidth);
  RefreshTextVisual();
}

void TextInputNode::RefreshTextVisual() {
  if (!DisplayNode || !CaretNode)
    return;
  const auto &style = Theme::Default().TextInput;
  const bool showsPlaceholder = Text.empty();
  DisplayNode->Text = showsPlaceholder ? Placeholder : Text;
  DisplayNode->Color = showsPlaceholder
                           ? style.PlaceholderColor
                           : style.ForegroundColor.Resolve(
                                 Enabled ? WidgetVisualState::Normal
                                         : WidgetVisualState::Disabled);
  CaretNode->Visible = Focused && Enabled;
}

void TextInputNode::SetEnabled(bool enabled) {
  Enabled = enabled;
  OnVisualStateChanged();
}

void TextInputNode::SetPlaceholder(const std::string &placeholder) {
  Placeholder = placeholder;
  RefreshTextVisual();
}

bool TextInputNode::SetProperty(const std::string &name,
                                const std::string &value) {
  if (name == "Text" || name == "Value")
    return SetText(value, false) || Text == value;
  if (name == "Placeholder" || name == "Hint") {
    SetPlaceholder(value);
    return true;
  }
  if (name == "Enabled") {
    bool enabled = false;
    if (!ParseBoolean(value, enabled))
      return false;
    SetEnabled(enabled);
    return true;
  }
  return RectNode::SetProperty(name, value);
}

bool TextInputNode::SetText(const std::string &text, bool notify) {
  if (Text == text)
    return false;
  Text = text;
  CaretOffset = (std::min)(CaretOffset, Text.size());
  if (notify)
    NotifyChanged();
  else
    RefreshTextVisual();
  return true;
}

void TextInputNode::Synchronize() {
  RectNode::Synchronize();
  if (!CaretNode || !DisplayNode)
    return;
  const auto &style = Theme::Default().TextInput;
  const size_t glyphs = Utf8CodePointCount(Text, CaretOffset);
  const float estimatedAdvance = DisplayNode->FontSize * 0.56f;
  const float caretLeft =
      (std::min)(Width - style.ContentPadding - style.CaretWidth,
                 style.ContentPadding + glyphs * estimatedAdvance);
  const float caretHeight = (std::min)(DisplayNode->FontSize + 2.0f, Height);
  CaretNode->Computed = {caretLeft, (Height - caretHeight) * 0.5f,
                         style.CaretWidth, caretHeight};
}
