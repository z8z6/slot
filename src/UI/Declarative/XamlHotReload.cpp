#include "UI/Declarative/XamlHotReload.h"

#include "UI/Declarative/XamlLoader.h"
#include "UI/Layout/Layout.h"

#include <system_error>

using namespace z8::ui;

XamlHotReload::XamlHotReload(std::filesystem::path fileName)
    : FileName(std::move(fileName)) {}

XamlReloadStatus XamlHotReload::Poll(Layout &layout) {
  std::error_code error;
  const auto writeTime = std::filesystem::last_write_time(FileName, error);
  if (error) {
    LastError = "Unable to inspect XAML file: " + FileName.string();
    const bool repeatedFailure = InspectionFailed;
    // 文件被删除后清除旧签名，确保同名文件恢复时即使签名碰巧相同也会重载。
    InspectionFailed = true;
    HasSignature = false;
    return repeatedFailure ? XamlReloadStatus::Unchanged
                           : XamlReloadStatus::Failed;
  }
  const auto fileSize = std::filesystem::file_size(FileName, error);
  if (error) {
    LastError = "Unable to inspect XAML file size: " + FileName.string();
    const bool repeatedFailure = InspectionFailed;
    InspectionFailed = true;
    HasSignature = false;
    return repeatedFailure ? XamlReloadStatus::Unchanged
                           : XamlReloadStatus::Failed;
  }
  InspectionFailed = false;
  if (HasSignature && writeTime == LastWriteTime && fileSize == LastFileSize)
    return XamlReloadStatus::Unchanged;

  // 先记住观察到的签名，使同一份无效保存只报告一次；编辑器下一次写入产生
  // 新签名后会自然重试。XamlLoader 在解析成功前不会触碰现有 Layout。
  HasSignature = true;
  LastWriteTime = writeTime;
  LastFileSize = fileSize;
  const auto result = XamlLoader().LoadFileInto(layout, FileName.string());
  if (!result) {
    LastError = result.Error + " at byte " +
                std::to_string(result.ErrorOffset) + ".";
    return XamlReloadStatus::Failed;
  }
  LastError.clear();
  return XamlReloadStatus::Reloaded;
}
