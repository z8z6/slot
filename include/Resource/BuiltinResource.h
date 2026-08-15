#pragma once

#include <string_view>

namespace z8::builtin {

// 资源前缀既用于构造内建名称，也用于从持久化名称恢复资源类别；保持小写可避免
// Windows 大小写不敏感路径与 URI 大小写敏感语义发生冲突。
inline constexpr std::string_view MeshPrefix = "builtin://mesh";
inline constexpr std::string_view MaterialPrefix = "builtin://material";
inline constexpr std::string_view ShaderPrefix = "builtin://shader";
inline constexpr std::string_view ShaderProgramPrefix =
    "builtin://shader-program";
inline constexpr std::string_view TexturePrefix = "builtin://texture";
inline constexpr std::string_view AudioPrefix = "builtin://audio";

// 内建资源使用规范 URI，避免场景、UI 和后端各自维护易冲突的短名称。
inline constexpr std::string_view CubeMesh = "builtin://mesh/cube";
inline constexpr std::string_view GridMesh = "builtin://mesh/grid";
inline constexpr std::string_view MountainMesh = "builtin://mesh/mountain";
inline constexpr std::string_view RectMesh = "builtin://mesh/rect";
inline constexpr std::string_view SkullMesh = "builtin://mesh/skull";
inline constexpr std::string_view SphereMesh = "builtin://mesh/sphere";

inline constexpr std::string_view MetalMaterial = "builtin://material/metal";
inline constexpr std::string_view UIMaterial = "builtin://material/ui";
inline constexpr std::string_view GrassBlockMaterial =
    "builtin://material/grass-block";

inline constexpr std::string_view GrassBlockTexture =
    "builtin://texture/grass-block";

inline constexpr std::string_view GameObjectProgram =
    "builtin://shader-program/game-object";
inline constexpr std::string_view UIObjectProgram =
    "builtin://shader-program/ui-object";
inline constexpr std::string_view MissingProgram =
    "builtin://shader-program/missing";
inline constexpr std::string_view TimeProgram =
    "builtin://shader-program/time";

} // namespace z8::builtin
