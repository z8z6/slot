#include "Target/DirectX/DX12Shader.h"
#include "Resource/ResourceManager.h"

#include <gtest/gtest.h>

#include <filesystem>

namespace z8 {
TEST(DX12ShaderTest, CompilesAllRegisteredShaderModelSixShadersWithDxc) {
  std::filesystem::current_path(SLOT_SOURCE_DIR);
  ResourceManager resources;
  DX12ShaderLibrary library(resources);
  ASSERT_GT(resources.GetShaders().Size(), 0U);

  library.CompileAll();

  EXPECT_EQ(library.Size(), resources.GetShaders().Size());
  resources.GetShaders().Visit(
      [&](ResourceHandle<Shader> handle, const Shader& shader) {
        SCOPED_TRACE(shader.Name);
        const auto* binary = library.TryGet(handle);
        ASSERT_NE(binary, nullptr);
        EXPECT_NE(binary->ByteCode, nullptr);
        EXPECT_NE(shader.Target.find("_6_"), std::string::npos);
      });
}

TEST(DX12ShaderTest, KeepsFxcSupportForLegacyTargets) {
  std::filesystem::current_path(SLOT_SOURCE_DIR);
  Shader shader;
  shader.Name = "LegacyMissingPixel";
  shader.FileName = L"shader/Missing.hlsl";
  shader.Entry = "PS";
  shader.Target = "ps_5_0";

  DX12Shader binary(&shader);
  binary.Compile();
  EXPECT_NE(binary.ByteCode, nullptr);
  EXPECT_GT(binary.GetByteCode().BytecodeLength, 0U);
}
} // namespace z8
