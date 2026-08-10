//
// Created by zhou_zhengming on 2026/7/29.
//

#pragma once
#include "Util/Owner.h"
#include "yoga/YGNode.h"

#include <memory>
#include <string>
#include <vector>

namespace z8 {
class UIObject;
}

namespace z8::ui {
class BaseNode {
protected:
  // GSL_OWNER 明确需要释放
  GSL_OWNER YGNodeRef YogaNode;
  std::unique_ptr<UIObject> UO;
  std::vector<std::unique_ptr<BaseNode>> ChildNodes;

public:
  // Key 是声明式 UI 在多次构建之间复用控件的稳定身份。
  std::string Key;
  // Parent 不拥有指针，所有权位于父节点的 ChildNodes
  BaseNode* Parent = nullptr;

  BaseNode();
  virtual ~BaseNode();

  BaseNode(const BaseNode&) = delete;
  BaseNode& operator=(const BaseNode&) = delete;

  size_t GetChildCount() const;
  BaseNode* GetChild(size_t index) const;
  YGNodeRef GetYogaNode() const { return YogaNode; }
  UIObject* GetUO() const { return UO.get(); }

  // 控件树和 Yoga 树必须同步修改。
  BaseNode* AddChild(std::unique_ptr<BaseNode> child);
  // 移除索引之后的所有子节点
  void RemoveChildrenFrom(size_t first);

  // 容器可把声明的子控件重定向到内部内容宿主，Panel 用它隔离标题栏。
  // 表示该容器如果要添加子元素，其合法的父容器
  virtual BaseNode* ContentHost();
  virtual const char* TypeName() const;
  virtual bool SetProperty(const std::string& name, const std::string& value);

  const std::vector<std::unique_ptr<BaseNode>>& GetChildren() const { return ChildNodes; }

protected:
  void SetObject(std::unique_ptr<UIObject> object);
};
}
