#pragma once

#include "UI/Layout/TreeViewNode.h"

#include <filesystem>
#include <functional>
#include <unordered_map>

namespace z8::ui {

/**
 * @brief 把文件目录投影为 TreeView 的编辑器文件浏览控件。
 *
 * 控件只保存展示所需的路径映射和 TreeView 展开/选择状态，不缓存文件内容；
 * 调用方通过 PathSelected 接收稳定路径，并自行决定打开或导入资源。
 */
class FileExplorerNode final : public TreeViewNode {
public:
  std::filesystem::path RootPath = "asset";
  bool ShowHidden = false;
  std::function<void(const std::filesystem::path&)> PathSelected;

  FileExplorerNode();

  const std::filesystem::path* GetPath(const TreeViewItemNode* item) const;
  /** 重新扫描 RootPath；目录优先、同类按名称排序，并且不跟随目录符号链接。 */
  void Refresh();
  bool SetProperty(const std::string& name,
                   const std::string& value) override;
  const char* TypeName() const override { return "FileExplorer"; }

private:
  std::unordered_map<const TreeViewItemNode*, std::filesystem::path> ItemPaths;

  std::unique_ptr<TreeViewItemNode>
  BuildItem(const std::filesystem::path& path, bool isRoot);
  static std::string ToUtf8(const std::filesystem::path& path);
};

} // namespace z8::ui
