#include <iostream>
#include <ostream>
#include <yoga/Yoga.h>


void init() {
  YGNodeRef root = YGNodeNew();
  YGNodeStyleSetFlexDirection(root, YGFlexDirectionRow);
  YGNodeStyleSetWidth(root, 100.0f);
  YGNodeStyleSetHeight(root, 100.0f);

  YGNodeRef child0 = YGNodeNew();
  YGNodeStyleSetFlexGrow(child0, 1.0f);
  YGNodeStyleSetMargin(child0, YGEdgeRight, 10.0f);
  YGNodeInsertChild(root, child0, 0.0f);

  YGNodeRef child1 = YGNodeNew();
  YGNodeStyleSetFlexGrow(child1, 1.0f);
  YGNodeInsertChild(root, child1, 1.0f);

  float left = YGNodeLayoutGetLeft(child0);
  float height = YGNodeLayoutGetHeight(child0);

  std::cout << left << height << std::endl;
}
