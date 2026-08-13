#pragma once

#include <filesystem>
#include <string>

namespace z8::ui {
class Layout;

/** 文件监视一次轮询的结果；Failed 不会替换当前仍可用的 UI 树。 */
enum class XamlReloadStatus { Unchanged, Reloaded, Failed };

/**
 * 基于文件签名的 XAML 热重载器。
 *
 * 它只在主线程帧边界检查时间戳和尺寸，不启动后台线程，也不与 Layout/DX12
 * 并发修改所有权。新 XML 会先完整解析，成功后才原子替换 Layout 根节点。
 */
class XamlHotReload final {
private:
  std::filesystem::path FileName;
  std::filesystem::file_time_type LastWriteTime{};
  std::uintmax_t LastFileSize = 0;
  std::string LastError;
  bool HasSignature = false;
  bool InspectionFailed = false;

public:
  explicit XamlHotReload(std::filesystem::path fileName);

  const std::string &GetLastError() const { return LastError; }
  const std::filesystem::path &GetPath() const { return FileName; }
  XamlReloadStatus Poll(Layout &layout);
};

} // namespace z8::ui
