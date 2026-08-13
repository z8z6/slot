# Slot Project

Slot is a real-time rendering framework powered by DirectX 12 as the underlying backend. 
It is dedicated to building a fully native UI library for the Windows platform. 
By taking advantage of DirectX 12's high-performance graphics APIs, 
Slot achieves low-latency real-time rendering and strives to implement standard Windows native UI features, 
bringing developers an efficient and reliable native interface development solution.

## 项目结构

- include 头文件
- src 源代码
- shader shader代码
- mesh 物体顶点和面
- third 第三方依赖库

## 构建

使用 MSVC 2026 和 Ninja

构建目录: cmake-build-debug

## 技能

见 skills 目录

## 知识文档

见 docs 目录

## 代码要求

### 命名要求

1. LLVM 风格，成员变量大驼峰，不要使用下划线
2. 提示，消息和警告等字符串应该使用英文
3. 类成员变量在函数前，成员函数按名称排序

### 代码注释要求

1. 写代码时必须同步补充中文注释，不把注释留到事后补；
2. 新增的数据结构和函数必须写注释，说明它们在系统中的职责、边界和设计意图。
3. 修改已有代码时，如果原代码缺少注释，必须补充关键注释；至少覆盖关键数据结构、关键控制流、状态转换、资源生命周期、并发/同步、ABI/指令语义/系统调用语义等容易误解的逻辑。
4. 注释不要只复述“代码实现了什么”，而要说明“为什么这样实现”和“这样实现的目的是什么”；尤其要结合图形学基础知识解释决策逻辑。
5. 注释应服务于后续维护者理解约束和取舍：例如为什么需要特殊处理某个边界条件、为什么不能直接转发、为什么要保持某种状态不变量、为什么某处必须按特定顺序更新。
6. 注释保持准确、简洁、贴近代码；当实现变化导致注释失效时，必须同步更新或删除过时注释。
7. 对于代码中涉及的图形学公式、算法或复杂逻辑，请在注释中简要说明其原理和用途。注释中可以包含举例。
