#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace z8::ui {
class BaseNode;

using DockNodeID = std::uint64_t;

/** DockTree 与输入统一使用窗口客户区 UI 像素坐标。 */
struct DockRect {
  float Left = 0.0f;
  float Top = 0.0f;
  float Width = 0.0f;
  float Height = 0.0f;

  bool Contains(float x, float y) const;
};

enum class DockNodeType { Leaf, Split };
enum class SplitAxis { Horizontal, Vertical };
enum class DockSide { Left, Right, Top, Bottom, Center };
enum class PanelPlacement { Docked, Floating };

/**
 * DockTree 节点；Rect 仅为递归布局缓存，结构、轴向和比例才是布局真值。
 * Split 严格拥有两个 Child，Leaf 严格只拥有一个 Dock 项。多 Panel
 * 可见切换由 PanelGroupNode 表达，避免 DockTree 再维护一套隐式页签状态。
 */
struct DockNode {
  DockNodeID ID = 0;
  DockNodeType Type = DockNodeType::Leaf;
  DockNode *Parent = nullptr;
  DockRect Rect;
  SplitAxis Axis = SplitAxis::Vertical;
  float SplitRatio = 0.5f;
  std::unique_ptr<DockNode> ChildA;
  std::unique_ptr<DockNode> ChildB;
  std::vector<BaseNode *> Panels;
};

/** 一次提交所需的稳定描述；拖动阶段只生成事务，不修改树。 */
struct DockTransaction {
  BaseNode *Panel = nullptr;
  DockNodeID SourceNode = 0;
  DockNodeID TargetNode = 0;
  DockSide TargetSide = DockSide::Center;
  bool TargetFloating = false;
  DockRect FloatingRect;
  /** 新建 Split 的 childA 比例；仅 Commit 阶段消费。 */
  float SplitRatio = 0.5f;
};

/**
 * Dock 布局的唯一结构真值，负责插入、移除、空节点折叠和递归矩形计算。
 * 所有结构修改都经 Commit，并在返回前验证父子关系与 Panel 唯一归属。
 */
class DockTree final {
public:
  std::unique_ptr<DockNode> Root;

  DockNode *AddPanel(BaseNode *panel);
  bool Commit(const DockTransaction &transaction);
  DockNode *Find(DockNodeID id) const;
  DockNode *FindLeafAt(float x, float y) const;
  DockNode *FindPanelLeaf(const BaseNode *panel) const;
  DockNode *FindSplitterAt(float x, float y, float tolerance = 4.0f) const;
  DockRect GetPreviewRect(const DockNode &target, DockSide side) const;
  void Layout(const DockRect &workspace);
  bool RemovePanel(BaseNode *panel);
  bool ResizeSplitter(DockNodeID split, float clientX, float clientY,
                      float minimumExtent = 80.0f);
  bool Validate(std::string *error = nullptr) const;
  std::string Dump() const;
  void Clear();

private:
  DockNodeID NextID = 1;

  std::unique_ptr<DockNode> CreateLeaf(BaseNode *panel = nullptr);
  void CollapseEmptyLeaf(DockNodeID leaf);
  static void LayoutNode(DockNode &node, const DockRect &rect);
  std::unique_ptr<DockNode> *OwnerSlot(DockNode *node);
};

} // namespace z8::ui
