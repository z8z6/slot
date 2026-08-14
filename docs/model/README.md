# 模型与网格

## 数据格式

`Mesh` 保存顶点和 16 位索引。固定顶点布局为：

| 语义 | 类型 | 偏移 |
| --- | --- | --- |
| POSITION | `XMFLOAT3` | 0 |
| NORMAL | `XMFLOAT3` | 12 |
| TEXCOORD | `XMFLOAT2` | 24 |

内建 Cube、Grid、Mountain、Rect、Skull、Sphere。索引采用 `uint16_t`，因此单个网格最多包含 65536 个渲染顶点；`Mesh::Validate` 会在资源注册前检查三角形完整性、索引范围和非有限浮点数。

`MeshNormalMode` 控制注册时的法线处理：

| 取值 | 含义 |
| --- | --- |
| `GenerateSmooth` | 默认值。`ResourceManager::AddMesh` 按三角形面积加权生成平滑法线；非法和零面积三角形不会参与累加。 |
| `PreserveAuthored` | 保留导入文件的分裂法线，或球体等曲面的解析法线，避免硬边被跨面平均。 |

Cube 和双面 Rect 按面拆分顶点，以保留硬边及正反面法线；Grid 覆盖完整 `[0, 1]` UV 且正面朝 `+Y`。Sphere 使用共享边中点细分二十面体，默认六级细分的基础拓扑为 40962 个顶点，不会越过 16 位索引上限；球面法线使用归一化位置直接计算，经度接缝和极点按三角形复制 UV 顶点，避免插值横穿整张纹理。

## GPU 合并

`DX12MeshManager::UnifyMesh` 把全部注册网格拼成一个大顶点/索引缓冲，并为每个网格记录索引数、索引起点和基础顶点偏移。绘制时使用 `DrawIndexedInstanced` 的偏移选择子网格。这减少了缓冲切换，但网格集合在初始化后固定。

## OBJ 导入

当前只读取 `v` 与 `f`，忽略法线、UV 和材质；四边形拆成两个三角形，不支持复杂多边形、负索引和 32 位索引。

## FBX 导入

`FbxMeshImporter` 提供文件和内存两种入口：

```cpp
auto imported = z8::FbxMeshImporter::Parse("asset/model/scene.fbx");
if (!imported) {
  // imported.Error 是可供编辑器展示的英文诊断。
  return;
}
auto handle = resources.AddMesh("model://scene", std::move(imported.Value));
```

内置解析器当前支持 ASCII FBX 7.x 的以下网格数据：

- 一个文件中的多个 `Geometry`，输出时合并为一个 Mesh；
- 三角形、四边形和凸 n-gon，使用扇形三角化；
- `ByControlPoint`、`ByPolygonVertex`、`ByPolygon`、`AllSame` 映射；
- `Direct`、`IndexToDirect` 引用方式的法线和 UV；
- 按位置/法线/UV 组合拆分渲染顶点，因而可保留 UV 接缝和硬边；
- 通过 `Connections` 关联的 Model 平移、XYZ 欧拉旋转、缩放、父级层次和 Geometric Transform；
- 默认把右手坐标转换为 Slot 左手坐标，也可通过 `FbxImportOptions` 关闭；UV 的 V 翻转为显式选项。

二进制 FBX 的数组通常带 zlib 压缩，需要 ufbx、Assimp 或 Autodesk FBX SDK 等完整解析库。仓库当前没有这类依赖，因此内置导入器会返回明确错误，不会把二进制内容误解析成损坏网格。FBX 的复杂旋转顺序、Pre/Post Rotation、枢轴、骨骼、动画、材质和凹多边形不在当前静态 Mesh 导入边界内；接入完整 FBX 库时可保留 `FbxMeshImportResult` 作为上层错误与所有权接口。

## 新增模型

1. 继承 Mesh，设置唯一 Name 并填充 V/I。
2. 在 `BuiltinResource.h` 添加规范 Asset ID，并由 `ResourceManager::AddMesh` 注册所有权。
3. 创建 SimpleGameObject 子类，通过 `Renderable.Mesh` 保存类型化资源引用。
4. 使用 `Scene::CreateGameObject` 把对象加入活动场景。

关键源码：`Mesh`、`ResourceManager.cpp`、`DX12MeshManager.cpp`、`DX12DefaultBuffer.cpp`。
