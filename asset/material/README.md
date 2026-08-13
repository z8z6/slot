# Material assets

材质描述文件统一放在本目录。当前内建材质仍由 `ResourceManager` 注册，因为项目
尚未定义稳定的材质序列化格式；引入文件格式后应保持资源 ID，并把描述文件落在
这里，C++ 的 `include/Material` 与 `src/Material` 只保留运行时代码。
