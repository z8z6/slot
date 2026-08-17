#pragma once

#include <string_view>

namespace z8::builtin::mesh {
inline constexpr std::string_view MeshPrefix = "builtin://mesh";
inline constexpr std::string_view CubeMesh = "builtin://mesh/cube";
inline constexpr std::string_view GridMesh = "builtin://mesh/grid";
inline constexpr std::string_view MountainMesh = "builtin://mesh/mountain";
inline constexpr std::string_view RectMesh = "builtin://mesh/rect";
inline constexpr std::string_view SkullMesh = "builtin://mesh/skull";
inline constexpr std::string_view SphereMesh = "builtin://mesh/sphere";
inline constexpr std::string_view TriangleMesh = "builtin://mesh/triangle";
} // namespace z8::builtin::mesh

namespace z8::builtin::material {
inline constexpr std::string_view MaterialPrefix = "builtin://material";
inline constexpr std::string_view MetalMaterial = "builtin://material/metal";
inline constexpr std::string_view UIMaterial = "builtin://material/ui";
inline constexpr std::string_view GrassBlockMaterial =
    "builtin://material/grass-block";
} // namespace z8::builtin::material

namespace z8::builtin::texture {
inline constexpr std::string_view TexturePrefix = "builtin://texture";
inline constexpr std::string_view GrassBlockTexture =
    "builtin://texture/grass-block";
} // namespace z8::builtin::texture

namespace z8::builtin::shader {
inline constexpr std::string_view ShaderPrefix = "builtin://shader";
inline constexpr std::string_view GameObjectVertex =
    "builtin://shader/game-object/vertex";
inline constexpr std::string_view GameObjectPixel =
    "builtin://shader/game-object/pixel";
inline constexpr std::string_view MissingVertex =
    "builtin://shader/missing/vertex";
inline constexpr std::string_view MissingPixel =
    "builtin://shader/missing/pixel";
inline constexpr std::string_view TimeVertex = "builtin://shader/time/vertex";
inline constexpr std::string_view TimePixel = "builtin://shader/time/pixel";
inline constexpr std::string_view UIObjectVertex =
    "builtin://shader/ui-object/vertex";
inline constexpr std::string_view UIObjectPixel =
    "builtin://shader/ui-object/pixel";
} // namespace z8::builtin::shader

namespace z8::builtin::shader::program {
inline constexpr std::string_view ShaderProgramPrefix =
    "builtin://shader-program";
inline constexpr std::string_view GameObjectProgram =
    "builtin://shader-program/game-object";
inline constexpr std::string_view UIObjectProgram =
    "builtin://shader-program/ui-object";
inline constexpr std::string_view MissingProgram =
    "builtin://shader-program/missing";
inline constexpr std::string_view TimeProgram = "builtin://shader-program/time";
} // namespace z8::builtin::shader::program

namespace z8::builtin::audio {}
