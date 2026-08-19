---
name: add-slot-resource
description: Add or modify Slot project Mesh, Material, ShaderProgram, and UI Node types using the ResourceManager, builtin Shader registration, RenderableComponent bindings, native layout ownership, tests, and project documentation. Use for requests to create a new render resource, bind resources to scene objects, add HLSL programs, or extend declarative/immediate UI controls in Slot.
---

# Add Slot Resource

Read the repository `AGENTS.md` and inspect `git status` before editing. Preserve unrelated changes and follow LLVM naming, English diagnostics, and synchronous Chinese design comments.

## Choose the workflow

- For geometry or imported vertex/index data, follow **Mesh**.
- For shared surface parameters, follow **Material**.
- For ordinary VS/PS programs, follow **ShaderProgram**. Define stage descriptions and their Program directly in `ResourceManager::RegisterBuiltinResources`.
- For native-layout controls, follow **UI Node**.
- When a request crosses types, add dependencies first: ShaderProgram, Material, Mesh, then scene/UI binding.

Use canonical lowercase asset IDs such as `builtin://mesh/name`. Add the constant to `include/Resource/BuiltinResource.h`. Keep asset IDs at serialization/loading boundaries; resolve them to indexed `ResourceRef<T>` values for render caches rather than querying strings per frame.

## Add a Mesh

1. Add the type under `include/Mesh` and `src/Mesh`. Derive from `Mesh` only when construction or import behavior is specialized.
2. Fill `V`, `I`, and `Name`. Preserve clockwise front faces and the `uint16_t` index limit. Explain procedural geometry formulas and boundary choices in Chinese comments.
3. Add its asset ID to `BuiltinResource.h` and call `AddMesh` in `ResourceManager::RegisterBuiltinResources`. Do not add a static registry object.
4. Bind it through `GameObject::Renderable.Mesh = ResourceReference<Mesh>(...)`.
5. Test resolution through `ResourceManager` and validate non-empty vertices/indices, bounds, winding, and normals as applicable.

`ResourceManager::AddMesh` computes normals after construction. Do not compute them twice unless the mesh requires authored split normals; document that exception before changing the import path.

## Add a Material

1. Add a `Material` subclass only when reusable defaults or loading behavior justify a type. Plain data can be constructed directly during resource registration.
2. Add its canonical ID and register ownership with `ResourceManager::AddMaterial`.
3. Set its `Program` reference and bind it through `Renderable.Material`; do not allocate a private default material in each object.
4. If fields change, update `DX12Material`, `asset/shader/Core/Const.hlsl`, upload alignment, and ABI tests together. Root CBV material slots must remain 256-byte aligned.
5. Verify that the HLSL reads the material fields instead of shadowing them with constants.

## Add a ShaderProgram

1. Add the HLSL file under `asset/shader/`. Keep shared constant ABI in `asset/shader/Core`.
2. Add builtin constants for both stage IDs and the Program ID to `include/Resource/BuiltinResource.h`.
3. Construct the VS and PS `Shader` descriptions in `ResourceManager::RegisterBuiltinResources`, then construct the `ShaderProgram` from the returned indexed references. Keep source, entry, target, and fixed state explicit at this registration boundary.
4. Use the Program builtin constant from Material or scene defaults; do not repeat asset ID literals.
5. Bind the Program through `Material::Program`; GameObject must not reference ShaderProgram directly.
6. Run the DXC compilation tests. When changing cbuffers, input semantics, registers, or packing, update both C++ and HLSL ABI definitions in the same change.

Reserve Shader subclasses for render-pass behavior such as shadow, post-process, or compute orchestration. Ordinary source, entry points, targets, fixed state, and resource identity belong to direct builtin registration.

## Add a UI Node

1. Add the Node under `include/UI/Layout` and `src/UI/Layout`. Use `unique_ptr` ownership through `BaseNode`; keep `LayoutStyle`, `LayoutBox`, and the C++ child tree consistent.
2. Create or reuse a `UIObject` visual. Bind its mesh/material/program through `RenderableComponent`.
3. Override `TypeName`, `ContentHost`, or `SetProperty` only when needed. Property values and errors exposed to users must be English.
4. Register XAML construction in `UI/Declarative/ControlFactory.cpp`.
5. Add an ImmediateUI method only if callers need the control in immediate declarations; use stable keys and rebuild the render index only on topology changes.
6. Add tests under `tests/UI/Controls`, `tests/UI/Declarative`, and `tests/UI/Layout` according to the affected boundary.

## Validate

Run from an MSVC 2026 developer shell:

```powershell
cmake --build cmake-build-debug --target slot slot_ui_controls_tests --config Debug
ctest --test-dir cmake-build-debug -C Debug --output-on-failure
git diff --check
```

Update the matching `docs` page when ownership, binding, shader ABI, scene behavior, or the public UI vocabulary changes.
