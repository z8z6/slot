# Lucide icons

本目录保存 Panel 标题栏使用的 Lucide SVG 图标。资源来自 Lucide 官方仓库，
下载日期为 2026-08-13；授权条款见同目录 `LICENSE`。

UI 当前将这些资源 URI 映射到着色器中的单色矢量轮廓，以避免为小尺寸图标
逐个创建纹理描述符。SVG 原文件作为统一的视觉资源来源保留，后续纹理系统可在
不改变 `ImageNode::Source` 协议的情况下切换为文件渲染。

Source: https://github.com/lucide-icons/lucide
