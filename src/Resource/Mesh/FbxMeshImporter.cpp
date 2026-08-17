#include "Mesh/FbxMeshImporter.h"

#include <DirectXMath.h>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <fstream>
#include <limits>
#include <optional>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace DirectX;

namespace z8 {
namespace {

struct TextBlock {
  std::string_view Header;
  std::string_view Body;
  size_t Next = 0;
};

struct AttributeLayer {
  enum class Kind { None, Normal, TexCoord } Type = Kind::None;
  std::string Mapping;
  std::string Reference;
  std::vector<double> Values;
  std::vector<int64_t> Indices;
  size_t ElementWidth = 0;
};

struct VertexKey {
  int64_t ControlPoint = -1;
  int64_t Normal = -1;
  int64_t TexCoord = -1;

  auto operator<=>(const VertexKey&) const = default;
};

struct VertexKeyHash {
  size_t operator()(const VertexKey& key) const noexcept {
    size_t hash = std::hash<int64_t>{}(key.ControlPoint);
    hash ^= std::hash<int64_t>{}(key.Normal) + 0x9e3779b9 + (hash << 6U) +
            (hash >> 2U);
    hash ^= std::hash<int64_t>{}(key.TexCoord) + 0x9e3779b9 + (hash << 6U) +
            (hash >> 2U);
    return hash;
  }
};

struct ModelTransform {
  XMFLOAT3 Translation{};
  XMFLOAT3 Rotation{};
  XMFLOAT3 Scaling{1.0f, 1.0f, 1.0f};
  XMFLOAT3 GeometricTranslation{};
  XMFLOAT3 GeometricRotation{};
  XMFLOAT3 GeometricScaling{1.0f, 1.0f, 1.0f};
};

/** 查找匹配花括号；字符串中的括号不参与层级，避免名称破坏块边界。 */
std::optional<size_t> FindClosingBrace(std::string_view text, size_t opening) {
  size_t depth = 0;
  bool inString = false;
  for (size_t i = opening; i < text.size(); ++i) {
    if (text[i] == '"' && (i == 0 || text[i - 1] != '\\')) inString = !inString;
    if (inString) continue;
    if (text[i] == '{') ++depth;
    if (text[i] == '}' && --depth == 0) return i;
  }
  return std::nullopt;
}

std::optional<TextBlock> FindBlock(std::string_view text, std::string_view key,
                                   size_t start = 0) {
  const auto headerStart = text.find(key, start);
  if (headerStart == std::string_view::npos) return std::nullopt;
  const auto opening = text.find('{', headerStart + key.size());
  if (opening == std::string_view::npos) return std::nullopt;
  const auto closing = FindClosingBrace(text, opening);
  if (!closing) return std::nullopt;
  return TextBlock{text.substr(headerStart, opening - headerStart),
                   text.substr(opening + 1, *closing - opening - 1),
                   *closing + 1};
}

template <typename NumberTy>
std::vector<NumberTy> ParseNumbers(std::string_view text) {
  std::vector<NumberTy> values;
  const char* current = text.data();
  const char* end = current + text.size();
  while (current < end) {
    while (current < end && (*current == ',' || *current == ' ' ||
                             *current == '\t' || *current == '\r' ||
                             *current == '\n'))
      ++current;
    if (current == end) break;
    NumberTy value{};
    const auto result = std::from_chars(current, end, value);
    if (result.ec != std::errc{}) {
      ++current;
      continue;
    }
    values.push_back(value);
    current = result.ptr;
  }
  return values;
}

template <typename NumberTy>
std::vector<NumberTy> ParseArray(std::string_view body, std::string_view key) {
  const auto block = FindBlock(body, key);
  if (!block) return {};
  const auto data = block->Body.find("a:");
  if (data == std::string_view::npos) return {};
  return ParseNumbers<NumberTy>(block->Body.substr(data + 2));
}

std::string ParseQuotedProperty(std::string_view body, std::string_view key) {
  const auto position = body.find(key);
  if (position == std::string_view::npos) return {};
  const auto first = body.find('"', position + key.size());
  if (first == std::string_view::npos) return {};
  const auto last = body.find('"', first + 1);
  if (last == std::string_view::npos) return {};
  return std::string(body.substr(first + 1, last - first - 1));
}

std::string ParseGeometryName(std::string_view header) {
  const auto marker = header.find("Geometry::");
  if (marker == std::string_view::npos) return {};
  const auto start = marker + std::string_view("Geometry::").size();
  const auto end = header.find('"', start);
  return std::string(header.substr(start, end - start));
}

std::optional<int64_t> ParseObjectId(std::string_view header) {
  const auto values = ParseNumbers<int64_t>(header);
  if (values.empty()) return std::nullopt;
  return values.front();
}

XMFLOAT3 ParseVectorProperty(std::string_view body, std::string_view name,
                             XMFLOAT3 fallback) {
  const std::string marker = "P: \"" + std::string(name) + "\"";
  const auto start = body.find(marker);
  if (start == std::string_view::npos) return fallback;
  const auto end = body.find_first_of("\r\n", start);
  const auto values = ParseNumbers<double>(body.substr(start, end - start));
  if (values.size() < 3) return fallback;
  const auto offset = values.size() - 3;
  return {static_cast<float>(values[offset]),
          static_cast<float>(values[offset + 1]),
          static_cast<float>(values[offset + 2])};
}

XMMATRIX MakeTransform(const XMFLOAT3& translation, const XMFLOAT3& rotation,
                       const XMFLOAT3& scaling) {
  const XMVECTOR scale = XMLoadFloat3(&scaling);
  const XMVECTOR offset = XMLoadFloat3(&translation);
  // FBX 欧拉角以度存储；此处覆盖常见 XYZ 静态模型，复杂枢轴和 PreRotation
  // 留给完整 FBX SDK，因为错误猜测枢轴顺序比明确保留边界更危险。
  const auto rotate = XMMatrixRotationRollPitchYaw(
      XMConvertToRadians(rotation.x), XMConvertToRadians(rotation.y),
      XMConvertToRadians(rotation.z));
  return XMMatrixScalingFromVector(scale) * rotate *
         XMMatrixTranslationFromVector(offset);
}

XMMATRIX ResolveModelTransform(
    int64_t id, const std::unordered_map<int64_t, ModelTransform>& models,
    const std::unordered_map<int64_t, int64_t>& parents,
    std::unordered_set<int64_t>& visiting) {
  const auto model = models.find(id);
  if (model == models.end() || !visiting.insert(id).second)
    return XMMatrixIdentity();
  auto result = MakeTransform(model->second.Translation, model->second.Rotation,
                              model->second.Scaling);
  if (const auto parent = parents.find(id);
      parent != parents.end() && models.contains(parent->second))
    result *= ResolveModelTransform(parent->second, models, parents, visiting);
  visiting.erase(id);
  return result;
}

AttributeLayer ParseLayer(std::string_view geometry, std::string_view blockName,
                          AttributeLayer::Kind kind) {
  AttributeLayer layer;
  const auto block = FindBlock(geometry, blockName);
  if (!block) return layer;
  layer.Type = kind;
  layer.Mapping = ParseQuotedProperty(block->Body, "MappingInformationType");
  layer.Reference =
      ParseQuotedProperty(block->Body, "ReferenceInformationType");
  if (kind == AttributeLayer::Kind::Normal) {
    layer.Values = ParseArray<double>(block->Body, "Normals:");
    layer.Indices = ParseArray<int64_t>(block->Body, "NormalsIndex:");
    layer.ElementWidth = 3;
  } else {
    layer.Values = ParseArray<double>(block->Body, "UV:");
    layer.Indices = ParseArray<int64_t>(block->Body, "UVIndex:");
    layer.ElementWidth = 2;
  }
  if (layer.Values.empty() || layer.Values.size() % layer.ElementWidth != 0)
    return {};
  return layer;
}

std::optional<int64_t> ResolveAttributeIndex(const AttributeLayer& layer,
                                             int64_t controlPoint,
                                             int64_t polygon,
                                             int64_t polygonVertex) {
  if (layer.Type == AttributeLayer::Kind::None) return std::nullopt;
  int64_t mapped = -1;
  if (layer.Mapping == "ByControlPoint") mapped = controlPoint;
  else if (layer.Mapping == "ByPolygonVertex") mapped = polygonVertex;
  else if (layer.Mapping == "ByPolygon") mapped = polygon;
  else if (layer.Mapping == "AllSame") mapped = 0;
  if (mapped < 0) return std::nullopt;

  if (layer.Reference == "IndexToDirect" || layer.Reference == "Index") {
    if (static_cast<size_t>(mapped) >= layer.Indices.size()) return std::nullopt;
    mapped = layer.Indices[static_cast<size_t>(mapped)];
  } else if (layer.Reference != "Direct") {
    return std::nullopt;
  }
  if (mapped < 0 || static_cast<size_t>(mapped) >=
                        layer.Values.size() / layer.ElementWidth)
    return std::nullopt;
  return mapped;
}

bool IsFinite(const XMFLOAT3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y) &&
         std::isfinite(value.z);
}

} // namespace

FbxMeshImportResult FbxMeshImporter::Parse(
    const std::filesystem::path& fileName, const FbxImportOptions& options) {
  std::ifstream stream(fileName, std::ios::binary);
  if (!stream)
    return {nullptr, "Unable to open FBX file: " + fileName.string()};
  std::ostringstream contents;
  contents << stream.rdbuf();
  if (!stream.good() && !stream.eof())
    return {nullptr, "Unable to read FBX file: " + fileName.string()};
  return ParseText(contents.str(), fileName.stem().string(), options);
}

FbxMeshImportResult FbxMeshImporter::ParseText(
    std::string_view source, std::string_view meshName,
    const FbxImportOptions& options) {
  if (source.starts_with("Kaydara FBX Binary"))
    return {nullptr,
            "Binary FBX is not supported by the built-in importer; use an "
            "ASCII FBX 7.x file or integrate ufbx/Autodesk FBX SDK."};

  auto mesh = std::make_unique<Mesh>();
  // 导入资源没有内建派生类，因此在加载边界把来源名写入
  // Resource 描述；上层仍可在 Add(assetId, ...) 时使用序列化 ID。
  mesh->Id = meshName.empty() ? "FBX" : std::string(meshName);
  bool hasAnyNormal = false;
  bool missingAnyNormal = false;
  size_t geometryOffset = 0;
  size_t geometryCount = 0;

  // Geometry 与 Model 分离存储，连接表决定实例归属；先建立静态变换图，再导入顶点。
  std::unordered_map<int64_t, ModelTransform> models;
  size_t modelOffset = 0;
  while (const auto model = FindBlock(source, "Model:", modelOffset)) {
    modelOffset = model->Next;
    const auto id = ParseObjectId(model->Header);
    if (!id) continue;
    ModelTransform transform;
    transform.Translation =
        ParseVectorProperty(model->Body, "Lcl Translation", {});
    transform.Rotation = ParseVectorProperty(model->Body, "Lcl Rotation", {});
    transform.Scaling = ParseVectorProperty(model->Body, "Lcl Scaling",
                                            {1.0f, 1.0f, 1.0f});
    transform.GeometricTranslation =
        ParseVectorProperty(model->Body, "GeometricTranslation", {});
    transform.GeometricRotation =
        ParseVectorProperty(model->Body, "GeometricRotation", {});
    transform.GeometricScaling = ParseVectorProperty(
        model->Body, "GeometricScaling", {1.0f, 1.0f, 1.0f});
    models.emplace(*id, transform);
  }

  std::unordered_map<int64_t, int64_t> parents;
  if (const auto connections = FindBlock(source, "Connections:")) {
    size_t start = 0;
    while ((start = connections->Body.find("C:", start)) !=
           std::string_view::npos) {
      const auto end = connections->Body.find_first_of("\r\n", start);
      const auto line = connections->Body.substr(start, end - start);
      if (line.find("\"OO\"") != std::string_view::npos) {
        const auto ids = ParseNumbers<int64_t>(line);
        if (ids.size() >= 2) parents[ids[0]] = ids[1];
      }
      if (end == std::string_view::npos) break;
      start = end + 1;
    }
  }

  while (const auto geometry = FindBlock(source, "Geometry:", geometryOffset)) {
    geometryOffset = geometry->Next;
    ++geometryCount;
    const auto positions = ParseArray<double>(geometry->Body, "Vertices:");
    const auto polygonIndices =
        ParseArray<int64_t>(geometry->Body, "PolygonVertexIndex:");
    if (positions.empty() || positions.size() % 3 != 0 ||
        polygonIndices.empty())
      continue;

    const auto normals = ParseLayer(geometry->Body, "LayerElementNormal:",
                                    AttributeLayer::Kind::Normal);
    const auto texCoords = ParseLayer(geometry->Body, "LayerElementUV:",
                                      AttributeLayer::Kind::TexCoord);
    XMMATRIX geometryTransform = XMMatrixIdentity();
    if (const auto geometryId = ParseObjectId(geometry->Header)) {
      if (const auto modelId = parents.find(*geometryId);
          modelId != parents.end() && models.contains(modelId->second)) {
        const auto& model = models.at(modelId->second);
        geometryTransform = MakeTransform(
            model.GeometricTranslation, model.GeometricRotation,
            model.GeometricScaling);
        std::unordered_set<int64_t> visiting;
        geometryTransform *= ResolveModelTransform(modelId->second, models,
                                                   parents, visiting);
      }
    }
    XMVECTOR determinant;
    const XMMATRIX normalTransform =
        XMMatrixTranspose(XMMatrixInverse(&determinant, geometryTransform));
    std::unordered_map<VertexKey, Mesh::IndexTy, VertexKeyHash> vertices;
    int64_t polygon = 0;
    int64_t polygonVertex = 0;
    struct Corner {
      int64_t ControlPoint;
      int64_t PolygonVertex;
    };
    std::vector<Corner> corners;

    const auto emitCorner = [&](const Corner& corner)
        -> std::optional<Mesh::IndexTy> {
      const auto normalIndex = ResolveAttributeIndex(
          normals, corner.ControlPoint, polygon, corner.PolygonVertex);
      const auto texCoordIndex = ResolveAttributeIndex(
          texCoords, corner.ControlPoint, polygon, corner.PolygonVertex);
      const VertexKey key{corner.ControlPoint, normalIndex.value_or(-1),
                          texCoordIndex.value_or(-1)};
      if (const auto existing = vertices.find(key); existing != vertices.end())
        return existing->second;
      if (mesh->V.size() > std::numeric_limits<Mesh::IndexTy>::max())
        return std::nullopt;

      const size_t position = static_cast<size_t>(corner.ControlPoint) * 3;
      XMFLOAT3 point{static_cast<float>(positions[position]),
                     static_cast<float>(positions[position + 1]),
                     static_cast<float>(positions[position + 2])};
      XMStoreFloat3(&point,
                    XMVector3TransformCoord(XMLoadFloat3(&point),
                                            geometryTransform));
      if (options.ConvertToLeftHanded) point.z = -point.z;
      if (!IsFinite(point)) return std::nullopt;

      XMFLOAT3 normal{};
      if (normalIndex) {
        const size_t base = static_cast<size_t>(*normalIndex) * 3;
        normal = {static_cast<float>(normals.Values[base]),
                  static_cast<float>(normals.Values[base + 1]),
                  static_cast<float>(normals.Values[base + 2])};
        auto transformed = XMVector3TransformNormal(XMLoadFloat3(&normal),
                                                    normalTransform);
        if (XMVectorGetX(XMVector3LengthSq(transformed)) <= 1.0e-12f)
          return std::nullopt;
        XMStoreFloat3(&normal, XMVector3Normalize(transformed));
        if (options.ConvertToLeftHanded) normal.z = -normal.z;
        hasAnyNormal = true;
      } else {
        missingAnyNormal = true;
      }

      XMFLOAT2 texCoord{};
      if (texCoordIndex) {
        const size_t base = static_cast<size_t>(*texCoordIndex) * 2;
        texCoord = {static_cast<float>(texCoords.Values[base]),
                    static_cast<float>(texCoords.Values[base + 1])};
        if (options.FlipTextureV) texCoord.y = 1.0f - texCoord.y;
      }
      const auto index = static_cast<Mesh::IndexTy>(mesh->V.size());
      mesh->V.emplace_back(point, normal, texCoord);
      vertices.emplace(key, index);
      return index;
    };

    for (const auto encoded : polygonIndices) {
      const bool endOfPolygon = encoded < 0;
      const int64_t controlPoint = endOfPolygon ? -encoded - 1 : encoded;
      if (controlPoint < 0 ||
          static_cast<size_t>(controlPoint) >= positions.size() / 3)
        return {nullptr, "FBX polygon references an invalid control point."};
      corners.push_back({controlPoint, polygonVertex++});
      if (!endOfPolygon) continue;
      if (corners.size() < 3)
        return {nullptr, "FBX contains a polygon with fewer than three vertices."};

      // 扇形三角化覆盖凸 n-gon；手性反射同时交换后两个角，保证叉积法线不翻转。
      for (size_t triangle = 1; triangle + 1 < corners.size(); ++triangle) {
        const Corner ordered[3] = {
            corners[0],
            options.ConvertToLeftHanded ? corners[triangle + 1]
                                        : corners[triangle],
            options.ConvertToLeftHanded ? corners[triangle]
                                        : corners[triangle + 1],
        };
        for (const auto& corner : ordered) {
          const auto index = emitCorner(corner);
          if (!index)
            return {nullptr,
                    "FBX mesh exceeds the 16-bit vertex limit or contains a "
                    "non-finite position."};
          mesh->I.push_back(*index);
        }
      }
      corners.clear();
      ++polygon;
    }
    if (!corners.empty())
      return {nullptr, "FBX polygon index array is missing an end marker."};

    if (mesh->Id == "FBX") {
      const auto geometryName = ParseGeometryName(geometry->Header);
      if (!geometryName.empty()) mesh->Id = geometryName;
    }
  }

  if (geometryCount == 0)
    return {nullptr, "FBX contains no Geometry blocks."};
  if (mesh->V.empty() || mesh->I.empty())
    return {nullptr, "FBX contains no importable mesh geometry."};

  // 只有每个渲染顶点都有作者法线时才保留；混合数据改为统一重算，避免零法线。
  mesh->NormalMode = hasAnyNormal && !missingAnyNormal
                         ? MeshNormalMode::PreserveAuthored
                         : MeshNormalMode::GenerateSmooth;
  std::string error;
  if (!mesh->Validate(&error)) return {nullptr, std::move(error)};
  if (mesh->NormalMode == MeshNormalMode::GenerateSmooth) mesh->ComputeNormals();
  return {std::move(mesh), {}};
}

} // namespace z8
