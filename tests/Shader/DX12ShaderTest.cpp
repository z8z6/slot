#include "Resource/ResourceManager.h"
#include "Target/DirectX/DX12Shader.h"

#include <gtest/gtest.h>

#include <filesystem>

namespace z8 {

TEST(DX12ShaderTest, CompilesRegisteredShaders) {
  std::filesystem::current_path(SLOT_SOURCE_DIR);
  ResourceManager resources;
  DX12ShaderLibrary library(resources);

  ASSERT_GT(resources.Shaders.Size(), 0U);
  library.CompileAll();

  EXPECT_EQ(library.Size(), resources.Shaders.Size());
  resources.Shaders.Visit(
      [&](ResourceRef<BaseShaderComponent> shaderRef, const BaseShaderComponent&) {
        const auto* binary = library.TryGet(shaderRef);
        ASSERT_NE(binary, nullptr);
        EXPECT_NE(binary->ByteCode, nullptr);
      });
}

} // namespace z8
