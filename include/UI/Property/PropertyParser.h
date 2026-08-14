#pragma once

#include <charconv>
#include <cmath>
#include <string_view>
#include <system_error>

namespace z8::ui {

/**
 * 解析声明层布尔值，并限制为项目公开支持的三组字面量。
 *
 * 这里刻意不接受任意大小写或非零整数，避免不同控件各自扩展语法后让同一份
 * XAML 在不同属性上产生不同含义。
 */
inline bool ParseBoolean(std::string_view text, bool &value) {
  if (text == "true" || text == "True" || text == "1") {
    value = true;
    return true;
  }
  if (text == "false" || text == "False" || text == "0") {
    value = false;
    return true;
  }
  return false;
}

/**
 * 严格解析声明属性中的有限浮点数。
 *
 * XAML 属性是外部输入，必须消费完整字符串并拒绝 NaN/Infinity；否则拼写错误
 * 会被 strtof 静默截断成看似有效的布局值，直到求解器中才表现为异常几何。
 */
inline bool ParseFiniteFloat(std::string_view text, float &value) {
  if (text.empty())
    return false;
  float parsed = 0.0f;
  const auto result =
      std::from_chars(text.data(), text.data() + text.size(), parsed);
  if (result.ec != std::errc{} || result.ptr != text.data() + text.size() ||
      !std::isfinite(parsed))
    return false;
  // 输出只在完整解析后提交，调用方即使直接传入成员也不会遭遇半更新。
  value = parsed;
  return true;
}

} // namespace z8::ui
