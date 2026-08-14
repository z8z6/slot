---
name: slot-ui-controls
description: Add, modify, debug, or review native Slot UI controls across retained nodes, Theme tokens, pointer and keyboard focus routing, XAML ControlFactory registration, ImmediateUI state synchronization, DirectX UI visuals, tests, and docs. Use for Button, Toggle, Slider, TextInput, TreeView, Menu, ToolBar, list/form controls, popup hierarchies, or any task under include/src/UI that changes a control's layout, interaction, visual state, or declarative API.
---

# Slot UI Controls

Use this workflow to keep one control implementation shared by XAML and ImmediateUI while preserving Slot's retained ownership, native layout, and DX12 rendering boundaries.

## Establish context

1. Read the repository `AGENTS.md`, `docs/ui/Style.md`, and the relevant part of `docs/ui/README.md`.
2. Inspect `git status --short`; preserve unrelated worktree and index changes.
3. Trace the nearest existing control through its header, source, Theme entry, `ControlFactory`, ImmediateUI method, and tests before designing a new type.
4. Keep user-visible diagnostics and property names in English. Add concise Chinese comments with every new structure, function, state transition, ownership boundary, or graphics/layout decision.

## Choose the node boundary

- Derive structural containers from `BaseNode`; they own children and layout only.
- Derive interactive non-visual containers from `BehaviorNode`.
- Derive visible controls from `RectNode` or another `DrawNode` subclass.
- Compose internal visuals with `unique_ptr`, retain only non-owning observer pointers, and call `BaseNode::AddChild` for fixed internal structure.
- Override `ContentHost()` when declarative children must enter an internal content node. XAML explicitly inserts into `ContentHost()`; direct C++ construction must do the same.
- Reuse `ScrollNode` for clipped, scrollable content instead of duplicating viewport, content, scrollbar, and `ScrollBehavior` state.
- Keep Menu popup levels as retained absolute children and toggle visibility instead of rebuilding nodes. Put ToolBar subtrees after normal siblings in painter order so popup rendering and reverse hit testing agree.
- Keep business data outside controls. A TreeView may own selection/expansion UI state, but not the caller's scene model.

Member variables precede functions and use UpperCamelCase without underscores. Order member functions alphabetically where practical.

## Model interaction as explicit state

Use `Focusable`, `Hovered`, `Pressed`, and `Focused` from `BehaviorNode`; do not add independent duplicate flags for those states. Keep only gesture-specific state such as `Armed`, `Dragging`, caret offset, selection, or expansion.

Pointer rules:

- Validate `Enabled`, button, and `Contains` before starting a gesture.
- Return `Capture` when mouse-up or drag must return to the same control.
- Clear gesture flags in `OnPointerCaptureLost` and when disabling the control.
- Activate Button/Toggle only after a complete down/up gesture inside the control.
- Update Slider values during `OnMouseDrag`, not only on mouse-up, so rendering follows resizing and pointer movement immediately.

Keyboard and text rules:

- Set `Focusable = true` only for controls that accept keyboard interaction.
- Handle control keys in `OnKeyDown`; ignore repeated activation with `KeyArgs::WasDown` where one-shot semantics matter.
- Receive printable text through `OnTextInput`, which is fed from `WM_CHAR` after Windows keyboard-layout conversion. Do not derive text from virtual-key codes.
- Store TextInput business text as UTF-8 and move/delete by UTF-8 code point boundaries. Treat full IME composition, selection, clipboard, and surrogate-pair composition as separate extensions rather than silently claiming support.
- Let `Layout` own the single focused handler. A click on empty UI clears focus.
- When a popup crosses a Dock splitter, give the popup target priority over the splitter. Clicking outside the target Menu root closes open top-level branches.

## Add theme and visuals

1. Add a semantic style structure in `include/UI/Style/Theme.h` before adding control-local colors or dimensions.
2. Populate every `Normal/Hovered/Pressed/Selected/Disabled/Focused` role in `src/UI/Style/Theme.cpp` using `StateColorStyle`.
3. Apply geometry tokens in the constructor and resolve state colors in `OnVisualStateChanged`.
4. Keep atomic form controls at their theme height with `FlexGrow = 0`; parent cross-axis layout supplies width. Do not let a Slider or single-line TextInput consume a Column's remaining height.
5. Use absolute internal children for dynamic overlays such as Slider fill/thumb or TextInput caret. Compute their `LayoutBox` in `Synchronize`; do not allocate nodes per frame.
6. Keep the root of indicator-only controls transparent. ImmediateUI restores generic Rect styles before reapplying control state, so `OnVisualStateChanged` must rebuild the control's complete semantic visual, including transparent roots, borders, track, and fill.
7. Add icons through `UIIcon`, the icon registry, the UI shader branch, and a matching asset reference. Do not expose source paths from composite controls.
8. Anchor cascading Menu popup levels to the previous row and flip them left near the client right edge; do not clip deep directories merely because they open in the preferred direction.

Explain screen-space formulas in Chinese comments. Clamp ranges, radii, and travel distance to avoid division by zero or negative geometry.

## Expose declarations

For XAML:

1. Implement `TypeName()` and strict `SetProperty()` parsing.
2. Register the public tag in `src/UI/Declarative/ControlFactory.cpp`.
3. Add aliases only for deliberate compatibility, for example `Slide` mapping to `SliderNode`.
4. Parse Min/Max properties without depending on XML attribute order; maintain a valid temporary range while properties are applied.
5. Return false for invalid booleans, ranges, or enum strings so the loader can report an English error.

For ImmediateUI:

1. Add a strongly typed method only when callers need per-frame declaration.
2. Acquire by stable `type + key`, then consume pending interaction before copying caller state back into the retained node.
3. When no interaction is pending, copy caller state into the node without notification.
4. Extend `ResetStyle` so an omitted style restores the control's constructor/theme box model rather than generic Rect defaults.
5. Apply explicit `UIStyle` last. Do not rebuild the render index for value or visual-state changes; topology alone marks Layout dirty.

Use the established return conventions:

- `Button(...)` returns whether activation was consumed.
- `Toggle(...)`, `Slider(...)`, and `TextInput(...)` return whether the caller value changed and write the new value through the reference.
- `BeginToolBar/EndToolBar` and recursive `BeginMenu/EndMenu` keep stable scopes; `MenuItem(...)` returns whether its leaf command was consumed.

## Test every boundary changed

Add focused tests rather than one end-to-end-only test:

- `tests/UI/Controls`: construction, internal ownership, pointer capture, keyboard behavior, disabled state, dynamic geometry, TreeView expansion/selection, UTF-8 editing.
- `tests/UI/Declarative`: XAML tag/property/nesting and ImmediateUI stable-node/state synchronization.
- `tests/UI/Style`: semantic token mapping and size invariants.
- `tests/UI/Layout`: focus routing or global event behavior when the change belongs to Layout.
- `tests/UI/Controls/ImageNodeTest.cpp`: new semantic icon mapping when applicable.

Add every new implementation source to `slot_test_runtime` and every new test file to its leaf `CMakeLists.txt`. Tests should assert internal capture through `Layout::CapturedHandler`; `Layout::OnMouseDown` intentionally reports a UI hit as `Handled` even when the control internally requested capture.

## Validate and document

Run from the MSVC 2026 environment:

```powershell
cmake --build cmake-build-debug --target slot slot_tests
ctest --test-dir cmake-build-debug --output-on-failure
git diff --check
```

Update `docs/ui/README.md` when the public control vocabulary, input protocol, Theme surface, or declaration examples change. Update `asset/xml/Main.xaml` when a new common control should be visible in the editor demo. Report any validation that could not run; do not hide dependency, shader, or platform failures.
