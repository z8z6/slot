# 模型与网格

## 数据格式

`Mesh` 保存顶点和 16 位索引。固定顶点布局为：

| 语义 | 类型 | 偏移 |
| --- | --- | --- |
| POSITION | `XMFLOAT3` | 0 |
| NORMAL | `XMFLOAT3` | 12 |
| TEXCOORD | `XMFLOAT2` | 24 |

内建 Cube、Grid、Mountain、Rect、Skull、Sphere。`ResourceManager` 显式拥有这些网格；调用 `AddMesh` 时，`ComputeNormals` 将三角形面法线按面积累加后归一化。

## GPU 合并

`DX12MeshManager::UnifyMesh` 把全部注册网格拼成一个大顶点/索引缓冲，并为每个网格记录索引数、索引起点和基础顶点偏移。绘制时使用 `DrawIndexedInstanced` 的偏移选择子网格。这减少了缓冲切换，但网格集合在初始化后固定。

## OBJ 导入

当前只读取 `v` 与 `f`，忽略法线、UV 和材质；四边形拆成两个三角形，不支持复杂多边形、负索引和 32 位索引。

## 新增模型

1. 继承 Mesh，设置唯一 Name 并填充 V/I。
2. 在 `BuiltinResource.h` 添加规范 Asset ID，并由 `ResourceManager::AddMesh` 注册所有权。
3. 创建 SimpleGameObject 子类，通过 `Renderable.Mesh` 保存类型化资源引用。
4. 使用 `Scene::CreateGameObject` 把对象加入活动场景。

关键源码：`Mesh`、`ResourceManager.cpp`、`DX12MeshManager.cpp`、`DX12DefaultBuffer.cpp`。
