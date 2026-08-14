#include "Core/ScenePicker.h"

#include "Core/Scene.h"
#include "Mesh/Mesh.h"
#include "Object/Camera/BaseCamera.h"
#include "Object/GameObject/GameObject.h"
#include "Resource/ResourceManager.h"

#include <DirectXMath.h>

#include <cmath>
#include <limits>

using namespace DirectX;

namespace z8 {
namespace {

bool IntersectTriangle(FXMVECTOR origin, FXMVECTOR direction,
                       FXMVECTOR first, GXMVECTOR second, HXMVECTOR third,
                       float& distance) {
  constexpr float epsilon = 1.0e-7f;
  const auto edge1 = XMVectorSubtract(second, first);
  const auto edge2 = XMVectorSubtract(third, first);
  const auto perpendicular = XMVector3Cross(direction, edge2);
  const float determinant = XMVectorGetX(XMVector3Dot(edge1, perpendicular));
  // 编辑器选择需要命中正反两面；这里不能沿用光栅器的背面剔除规则。
  if (std::abs(determinant) <= epsilon)
    return false;
  const float inverseDeterminant = 1.0f / determinant;
  const auto offset = XMVectorSubtract(origin, first);
  const float u =
      XMVectorGetX(XMVector3Dot(offset, perpendicular)) * inverseDeterminant;
  if (u < 0.0f || u > 1.0f)
    return false;
  const auto cross = XMVector3Cross(offset, edge1);
  const float v = XMVectorGetX(XMVector3Dot(direction, cross)) *
                  inverseDeterminant;
  if (v < 0.0f || u + v > 1.0f)
    return false;
  distance = XMVectorGetX(XMVector3Dot(edge2, cross)) * inverseDeterminant;
  return distance > epsilon;
}

} // namespace

GameObject* ScenePicker::Pick(Scene& scene, const ResourceManager& resources,
                              BaseCamera& camera, const ScenePickRect& viewport,
                              float pointerX, float pointerY) {
  if (viewport.Width <= 0.0f || viewport.Height <= 0.0f ||
      pointerX < viewport.Left || pointerY < viewport.Top ||
      pointerX > viewport.Left + viewport.Width ||
      pointerY > viewport.Top + viewport.Height)
    return nullptr;

  const float ndcX =
      ((pointerX - viewport.Left) / viewport.Width) * 2.0f - 1.0f;
  const float ndcY =
      1.0f - ((pointerY - viewport.Top) / viewport.Height) * 2.0f;
  const auto viewProjection = XMLoadFloat4x4(&camera.GetViewProj());
  XMVECTOR viewProjectionDeterminant;
  const auto inverseViewProjection =
      XMMatrixInverse(&viewProjectionDeterminant, viewProjection);
  if (std::abs(XMVectorGetX(viewProjectionDeterminant)) <= 1.0e-8f)
    return nullptr;

  const auto rayOrigin = XMVector3TransformCoord(
      XMVectorSet(ndcX, ndcY, 0.0f, 1.0f), inverseViewProjection);
  const auto rayFar = XMVector3TransformCoord(
      XMVectorSet(ndcX, ndcY, 1.0f, 1.0f), inverseViewProjection);
  const auto rayDirection =
      XMVector3Normalize(XMVectorSubtract(rayFar, rayOrigin));

  GameObject* closestObject = nullptr;
  float closestDistanceSquared = (std::numeric_limits<float>::max)();
  for (auto* object : scene.GOs.get()) {
    const auto meshHandle = resources.Resolve(object->Renderable.Mesh);
    const auto* mesh = resources.TryGet(meshHandle);
    if (!mesh)
      continue;

    object->Transform.UpdateWorld();
    const auto world = XMLoadFloat4x4(&object->Transform.World);
    XMVECTOR worldDeterminant;
    const auto inverseWorld = XMMatrixInverse(&worldDeterminant, world);
    if (std::abs(XMVectorGetX(worldDeterminant)) <= 1.0e-8f)
      continue;
    const auto localOrigin = XMVector3TransformCoord(rayOrigin, inverseWorld);
    const auto localDirection = XMVector3Normalize(
        XMVector3TransformNormal(rayDirection, inverseWorld));

    for (size_t triangle = 0; triangle + 2 < mesh->I.size(); triangle += 3) {
      const auto firstIndex = mesh->I[triangle];
      const auto secondIndex = mesh->I[triangle + 1];
      const auto thirdIndex = mesh->I[triangle + 2];
      if (firstIndex >= mesh->V.size() || secondIndex >= mesh->V.size() ||
          thirdIndex >= mesh->V.size())
        continue;
      float localDistance = 0.0f;
      if (!IntersectTriangle(localOrigin, localDirection,
                             XMLoadFloat3(&mesh->V[firstIndex].Pos),
                             XMLoadFloat3(&mesh->V[secondIndex].Pos),
                             XMLoadFloat3(&mesh->V[thirdIndex].Pos),
                             localDistance))
        continue;
      const auto localHit = XMVectorMultiplyAdd(
          XMVectorReplicate(localDistance), localDirection, localOrigin);
      const auto worldHit = XMVector3TransformCoord(localHit, world);
      const float distanceSquared = XMVectorGetX(
          XMVector3LengthSq(XMVectorSubtract(worldHit, rayOrigin)));
      if (distanceSquared < closestDistanceSquared) {
        closestDistanceSquared = distanceSquared;
        closestObject = object;
      }
    }
  }
  return closestObject;
}

} // namespace z8
