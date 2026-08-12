//
// Created by zhou_zhengming on 2026/8/12.
//
#include "Util/Color.h"

#include <charconv>
#include <sstream>
#include <string>
#include <vector>

bool z8::ParseUIColor(const std::string &text, DirectX::XMFLOAT4 &color) {
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
