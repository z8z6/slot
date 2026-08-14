"""根据 Shader manifest 生成 Slot 的显式 ShaderProgram 注册代码。"""

import argparse
import json
from pathlib import Path


def cpp_string(value: str) -> str:
    """生成与 C++ UTF-8 字符串字面量兼容的转义文本。"""
    return json.dumps(value, ensure_ascii=False)


def generate(manifest_paths: list[Path]) -> str:
    """将所有 manifest 按路径排序，保证生成结果和资源句柄稳定。"""
    lines = [
        "// 此文件由 scripts/generate_shader_registry.py 生成，请勿手动修改。",
        '#include "Resource/ResourceManager.h"',
        "",
        "#include <memory>",
        "#include <string>",
        "",
        "namespace z8 {",
        "void RegisterGeneratedShaders(ResourceManager& resources) {",
    ]

    for index, path in enumerate(sorted(manifest_paths)):
        data = json.loads(path.read_text(encoding="utf-8"))
        required = {"assetId", "name", "source", "vertex", "pixel"}
        missing = required.difference(data)
        if missing:
            raise ValueError(f"{path}: missing fields: {sorted(missing)}")

        stage_handles = {}
        for stage_name, default_target in (("vertex", "vs_6_0"),
                                           ("pixel", "ps_6_0")):
            stage = data[stage_name]
            handle_name = f"shader{index}{stage_name.title()}"
            stage_handles[stage_name] = handle_name
            shader_name = f'{data["name"]}_{"V" if stage_name == "vertex" else "P"}'
            # Shader 阶段与 ShaderProgram 属于不同资源池，名称前缀也必须明确区分；
            # 以 Program 的末段派生可读路径，但不复用 shader-program 类别前缀。
            program_slug = data["assetId"].rsplit("/", 1)[-1]
            shader_id = f'builtin://shader/{program_slug}/{stage_name}'
            lines.extend([
                f"  auto {handle_name}Description = std::make_unique<Shader>();",
                f"  {handle_name}Description->AssetId = {cpp_string(shader_id)};",
                f"  {handle_name}Description->Name = {cpp_string(shader_name)};",
                f"  {handle_name}Description->FileName = L{cpp_string(data['source'])};",
                f"  {handle_name}Description->Entry = {cpp_string(stage['entry'])};",
                f"  {handle_name}Description->Target = "
                f"{cpp_string(stage.get('target', default_target))};",
                f"  const auto {handle_name} = "
                f"resources.Add(std::move({handle_name}Description));",
            ])

        program_name = f"program{index}"
        lines.extend([
            f"  auto {program_name} = std::make_unique<ShaderProgram>();",
            f"  {program_name}->AssetId = {cpp_string(data['assetId'])};",
            f"  {program_name}->Name = {cpp_string(data['name'])};",
            f"  {program_name}->VertexShader = {stage_handles['vertex']};",
            f"  {program_name}->PixelShader = {stage_handles['pixel']};",
            f"  {program_name}->EnableDepth = "
            f"{'true' if data.get('enableDepth', True) else 'false'};",
            f"  {program_name}->EnableBlend = "
            f"{'true' if data.get('enableBlend', False) else 'false'};",
            f"  resources.Add(std::move({program_name}));",
            "",
        ])

    lines.extend(["}", "} // namespace z8", ""])
    return "\n".join(lines)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("manifests", nargs="+", type=Path)
    arguments = parser.parse_args()

    arguments.output.parent.mkdir(parents=True, exist_ok=True)
    arguments.output.write_text(generate(arguments.manifests), encoding="utf-8")


if __name__ == "__main__":
    main()
