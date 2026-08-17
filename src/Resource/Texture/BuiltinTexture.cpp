#include "Texture/BuiltinTexture.h"

#include "Resource/BuiltinResource.h"

#include <stdexcept>

using namespace z8;

GrassBlockTexture::GrassBlockTexture() {
  Id = builtin::texture::GrassBlockTexture;

  // 内建资源在 ResourceManager 构造完成时必须已可上传；解码失败不能
  // 留下一个 ID 有效但像素不完整的半构造资源。
  std::string error;
  if (!Load(L"asset/texture/grass-block.png", &error))
    throw std::runtime_error(error);
}
