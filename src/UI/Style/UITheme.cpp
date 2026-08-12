#include "UI/Style/UITheme.h"

#include <algorithm>
#include <charconv>
#include <cstdlib>
#include <sstream>
#include <vector>

using namespace DirectX;
using namespace z8::ui;

const UITheme& UITheme::Modern() {
  // 低饱和蓝灰色减少长时间工具界面的视觉疲劳，同时保持标题与内容层级。
  static const UITheme theme{
      .Rect = {{0.20f, 0.22f, 0.26f, 1.0f}, 4.0f, 0.0f, 24.0f, 24.0f},
      .Panel = {{
                    {0.075f, 0.082f, 0.10f, 0.98f},
                    8.0f,
                    0.0f,
                    240.0f,
                    160.0f,
                },
                {0.13f, 0.15f, 0.19f, 1.0f},
                {0.92f, 0.94f, 0.98f, 1.0f},
                {0.09f, 0.10f, 0.13f, 0.92f},
                {0.32f, 0.38f, 0.48f, 1.0f},
                36.0f,
                10.0f,
                6.0f,
                12.0f,
                24.0f}};
  return theme;
}

bool z8::ui::ParseUIColor(const std::string& text, XMFLOAT4& color) {
  if ((text.size() == 7 || text.size() == 9) && text.front() == '#') {
    unsigned value = 0;
    const auto result = std::from_chars(text.data() + 1,
                                        text.data() + text.size(), value, 16);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size())
      return false;
    if (text.size() == 7) value = (value << 8) | 0xff;
    color = {static_cast<float>((value >> 24) & 0xff) / 255.0f,
             static_cast<float>((value >> 16) & 0xff) / 255.0f,
             static_cast<float>((value >> 8) & 0xff) / 255.0f,
             static_cast<float>(value & 0xff) / 255.0f};
    return true;
  }

  std::stringstream stream(text);
  std::vector<float> channels;
  std::string channel;
  while (std::getline(stream, channel, ',')) {
    char* end = nullptr;
    const float value = std::strtof(channel.c_str(), &end);
    if (end == channel.c_str() || *end != '\0') return false;
    channels.push_back(value);
  }
  if (channels.size() != 3 && channels.size() != 4) return false;
  for (float value : channels)
    if (value < 0.0f || value > 1.0f) return false;
  color = {channels[0], channels[1], channels[2],
           channels.size() == 4 ? channels[3] : 1.0f};
  return true;
}
