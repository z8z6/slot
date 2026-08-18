#pragma once

#include "BaseMesh.h"

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

namespace z8 {

/** 控制 FBX 坐标和纹理坐标到 Slot 顶点布局的转换。 */
struct FbxImportOptions {
  // FBX 常见资产使用右手系；反转 Z 并同步翻转三角形绕序可保持几何法线方向。
  bool ConvertToLeftHanded = true;
  bool FlipTextureV = false;
};

/** 返回网格或可直接展示给工具层的英文错误，避免文件错误触发断言。 */
struct FbxMeshImportResult {
  std::unique_ptr<BaseMesh> Value;
  std::string Error;

  explicit operator bool() const { return Value != nullptr; }
};

/**
 * @brief 将 ASCII FBX 7.x 的 Geometry 数据转换为渲染网格。
 *
 * 导入器处理多 Geometry、任意多边形、控制点/多边形顶点属性映射及
 * Direct/IndexToDirect 引用。二进制 FBX 需要完整 SDK/ufbx，当前会明确拒绝，
 * 而不是把二进制内容误当文本并生成损坏网格。
 */
class FbxImporter : MeshImporter{
public:
  /** 从磁盘读取并解析 FBX。 */
  static FbxMeshImportResult Parse(
      const std::filesystem::path& fileName,
      const FbxImportOptions& options = {});
  /** 从内存解析，供资源管线和无需临时文件的单元测试复用。 */
  static FbxMeshImportResult ParseText(
      std::string_view source, std::string_view meshName = "FBX",
      const FbxImportOptions& options = {});
};

} // namespace z8
