# xml2prop 项目指令

## 项目概览

- 这是一个 Qt 6、CMake、Qt Quick/QML Windows 桌面应用。
- C++ 负责 XML 解析、properties 生成和文件读写；QML 负责界面。
- 不要在项目根目录创建 `AGENTS.md`，项目指令仅维护在本文件。

## 构建与测试

```powershell
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

## 转换规则

- 嵌套元素使用点号路径，例如 `server.database.host`。
- XML 属性使用 `.@`，例如 `server.@port`。
- 同名兄弟元素重复出现时使用从 0 开始的下标，例如 `items.item[0].name`。
- 文件采用 UTF-8 读写；properties 特殊字符按 Java properties 约定转义。
