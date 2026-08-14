#pragma once

#include "UI/Layout/ImageNode.h"
#include "UI/Layout/RectNode.h"
#include "UI/Layout/TextNode.h"

#include <functional>

namespace z8::ui {

class MenuNode;

/** Menu 的叶子命令项；只拥有激活手势，业务命令由回调或 ConsumeClicked 接收。 */
class MenuItemNode final : public RectNode {
private:
  bool Armed = false;
  bool ClickPending = false;

public:
  TextNode *LabelNode = nullptr;
  bool Enabled = true;
  std::function<void()> Clicked;

  MenuItemNode();

  bool ConsumeClicked();
  EventReply OnKeyDown(KeyArgs args) override;
  EventReply OnMouseDown(MouseMovArgs args) override;
  EventReply OnMouseUp(MouseMovArgs args) override;
  void OnPointerCaptureLost() override { Armed = false; }
  void SetEnabled(bool enabled);
  bool SetProperty(const std::string &name, const std::string &value) override;
  void SetText(const std::string &text);
  const char *TypeName() const override { return "MenuItem"; }

private:
  void Activate();
  void CloseOwningMenu();
  void OnVisualStateChanged() override;
};

/**
 * 可同时作为菜单栏入口和级联目录项的 Menu。
 * Popup 始终保留在节点树中，只切换可见性，打开菜单不会重建渲染资源。
 */
class MenuNode final : public RectNode {
private:
  bool TopLevel = false;

public:
  TextNode *LabelNode = nullptr;
  ImageNode *ChevronNode = nullptr;
  RectNode *PopupNode = nullptr;
  bool Enabled = true;
  bool Open = false;

  MenuNode();

  void CloseBranch();
  void CloseHierarchy();
  BaseNode *ContentHost() override { return PopupNode; }
  bool IsTopLevel() const { return TopLevel; }
  EventReply OnKeyDown(KeyArgs args) override;
  EventReply OnMouseDown(MouseMovArgs args) override;
  EventReply OnMouseMove(MouseMovArgs args) override;
  void OnAfterLayout() override;
  MenuNode *RootMenu();
  void SetEnabled(bool enabled);
  bool SetOpen(bool open);
  bool SetProperty(const std::string &name, const std::string &value) override;
  void SetText(const std::string &text);
  void SetTopLevel(bool topLevel);
  const char *TypeName() const override { return "Menu"; }

private:
  void CloseSiblingMenus();
  bool HasOpenSibling() const;
  void OnVisualStateChanged() override;
  void OpenExclusive();
  MenuNode *ParentMenu() const;
  void RefreshGeometry();
};

} // namespace z8::ui
