#include "Target/DirectX/DX12Shader.h"

#include <gtest/gtest.h>

#include <filesystem>

namespace z8 {
TEST(DX12ShaderTest, CompilesAllRegisteredShaderModelSixShadersWithDxc) {
  std::filesystem::current_path(SLOT_SOURCE_DIR);
  auto& registry = DX12ShaderRegistry::Instance();
  ASSERT_FALSE(registry.Shaders.empty());

  registry.CompileAll();

  EXPECT_EQ(registry.Binaries.size(), registry.Shaders.size());
  for (const auto& [name, shader] : registry.Shaders) {
    SCOPED_TRACE(name);
    ASSERT_NE(shader->Binary, nullptr);
    EXPECT_NE(shader->Binary->ByteCode, nullptr);
    EXPECT_NE(shader->Target.find("_6_"), std::string::npos);
  }
}

TEST(DX12ShaderTest, KeepsFxcSupportForLegacyTargets) {
  std::filesystem::current_path(SLOT_SOURCE_DIR);
  PixelShader shader;
  shader.Name = "LegacyMissingPixel";
  shader.FileName = L"shader/Missing.hlsl";
  shader.Target = "ps_5_0";

  DX12Shader binary(&shader);
  binary.Compile();
  EXPECT_NE(binary.ByteCode, nullptr);
  EXPECT_GT(binary.GetByteCode().BytecodeLength, 0U);
}
} // namespace z8
