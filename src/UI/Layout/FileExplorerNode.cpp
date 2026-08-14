#include "UI/Layout/FileExplorerNode.h"

#include <algorithm>
#include <system_error>
#include <vector>

using namespace z8::ui;

namespace {

bool ParseBool(const std::string& value, bool& result) {
  if (value == "true" || value == "True" || value == "1")
    result = true;
  else if (value == "false" || value == "False" || value == "0")
    result = false;
  else
    return false;
  return true;
}

} // namespace

FileExplorerNode::FileExplorerNode() {
  SelectionChanged = [this](TreeViewItemNode* item) {
    const auto* path = GetPath(item);
    if (path && PathSelected)
      PathSelected(*path);
  };
  Refresh();
}

std::unique_ptr<TreeViewItemNode>
FileExplorerNode::BuildItem(const std::filesystem::path& path, bool isRoot) {
  auto item = std::make_unique<TreeViewItemNode>();
  auto* observer = item.get();
  ItemPaths.emplace(observer, path);
  item->SetText(isRoot ? ToUtf8(path) : ToUtf8(path.filename()));
  item->Expanded = isRoot;

  std::error_code error;
  if (!std::filesystem::is_directory(path, error) || error)
    return item;

  std::vector<std::filesystem::directory_entry> entries;
  for (std::filesystem::directory_iterator iterator(path, error), end;
       !error && iterator != end; iterator.increment(error)) {
    const auto name = ToUtf8(iterator->path().filename());
    if (!ShowHidden && name.starts_with('.'))
      continue;
    entries.push_back(*iterator);
  }
  std::sort(entries.begin(), entries.end(), [](const auto& left,
                                                const auto& right) {
    std::error_code leftError;
    std::error_code rightError;
    const bool leftDirectory = left.is_directory(leftError);
    const bool rightDirectory = right.is_directory(rightError);
    if (leftDirectory != rightDirectory)
      return leftDirectory;
    return left.path().filename().native() < right.path().filename().native();
  });

  for (const auto& entry : entries) {
    std::error_code typeError;
    // 不跟随目录符号链接，防止资源目录中的 junction 形成递归环。
    if (entry.is_symlink(typeError) && entry.is_directory(typeError))
      continue;
    item->ContentHost()->AddChild(BuildItem(entry.path(), false));
  }
  return item;
}

const std::filesystem::path*
FileExplorerNode::GetPath(const TreeViewItemNode* item) const {
  const auto iterator = ItemPaths.find(item);
  return iterator == ItemPaths.end() ? nullptr : &iterator->second;
}

void FileExplorerNode::Refresh() {
  ItemPaths.clear();
  ContentNode->Children.clear();

  std::error_code error;
  const bool exists = std::filesystem::exists(RootPath, error) && !error;
  auto root = BuildItem(RootPath, true);
  if (!exists) {
    root->SetText(ToUtf8(RootPath) + " (not found)");
    root->SetEnabled(false);
  }
  ContentNode->AddChild(std::move(root));
  SelectedItem = nullptr;
}

bool FileExplorerNode::SetProperty(const std::string& name,
                                   const std::string& value) {
  if (name == "RootPath" || name == "Path") {
    RootPath = std::filesystem::path(value);
    Refresh();
    return true;
  }
  if (name == "ShowHidden") {
    bool showHidden = false;
    if (!ParseBool(value, showHidden))
      return false;
    ShowHidden = showHidden;
    Refresh();
    return true;
  }
  return TreeViewNode::SetProperty(name, value);
}

std::string FileExplorerNode::ToUtf8(const std::filesystem::path& path) {
  // filesystem 在 Windows 上保存 UTF-16；u8string 明确转为 TextNode 所需 UTF-8。
  const auto value = path.u8string();
  return {reinterpret_cast<const char*>(value.data()), value.size()};
}
