# 场景对象

`Object` 是带 `Transform`、更新函数和输入回调的基类。`GameObject` 组合 Mesh、Material、Collider 与 VS/PS，并要求派生类提供对象常量。`GameObjectImpl<T>` 用模板保存常量结构；当前 3D 和 UI 对象都使用一个世界矩阵。

```mermaid
classDiagram
    Object <|-- GameObject
    Object <|-- Camera
    Object <|-- Light
    GameObject <|-- GameObjectImpl
    GameObjectImpl <|-- SimpleGameObject
    GameObjectImpl <|-- UIObject
    Object *-- Transform
    GameObject o-- Mesh
    GameObject o-- Material
    GameObject o-- Shader
```

## Transform

项目采用左手坐标系：+X 向右、+Y 向上、+Z 向屏幕内。世界矩阵计算为 `Scale * Rotation * Translation`。DirectXMath 矩阵写入 HLSL 常量前会转置；修改矩阵代码时必须同时核对 HLSL 的 `mul` 顺序。

Transform 还支持直角坐标与球坐标转换。

## 相机、灯光和碰撞

Camera 使用 `XMMatrixLookAtLH` 和 `XMMatrixPerspectiveFovLH`，默认 near/far 为 1/1000、FOV 120°。FirstPersonCamera 根据 yaw/pitch 更新目标方向，并将 pitch 限制在 ±89°。

ParallelLight 的位置、颜色、方向被写入全局常量，当前 Shader 只实现方向光。Collider 只有接口，BoxCollider 尚未实现，物理模块仍是骨架。

## 源码入口

- `include/UI/Object/Object.h`
- `src/UI/Object/Transform.cpp`
- `include/UI/Object/GameObject/GameObject.h`
- `src/UI/Object/Camera`
- `include/UI/Light`、`include/UI/Phys`
