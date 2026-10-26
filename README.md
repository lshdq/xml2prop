# XML 转 Properties

这是一个基于 Qt 6、CMake 和 Qt Quick/QML 的 Windows 桌面工具，用于把 XML 配置转换为 Java `.properties` 文本。

## 依赖与构建

- CMake 3.21 或更高版本
- Qt 6.3 或更高版本：Core、Gui、Qml、Quick、QuickControls2、QuickDialogs2、Test
- C++17 编译器

```powershell
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

测试包括严格 UTF-8 文件解码、本地/UNC `file://` URL 分类、失败保存不破坏既有目标内容，以及在 offscreen 平台从生产一致的 `qrc:/Xml2Prop/qml/Main.qml` 入口加载界面的转换、文件载入、文件保存、错误、清空流程和打开/保存 `FileDialog` 对象 smoke test。UNC 测试只验证路径分类，不执行真实网络共享读写；测试不打开真实系统文件对话框。

## 使用

启动 `xml2prop` 后，可在左侧输入 XML，点击“转换”查看右侧结果；也可以打开 UTF-8 XML 文件，并将结果保存为 UTF-8 `.properties` 文件。解析和文件读写错误会显示在顶部状态栏。

## 映射规则

- 嵌套元素使用点号路径：`<server><database><host>db</host>` 转为 `server.database.host=db`。
- 元素属性追加 `.@`：`server.@port=8080`。
- 同一父节点下的重复同名元素使用从 0 开始的下标，例如 `items.item[0]`、`items.item[1]`。
- 空叶子输出 `key=`；纯空白格式化文本不会成为配置值。
- 键和值中的反斜杠、分隔符和控制字符按 Java properties 规则转义。
- 文件对话框传递标准 `file://` URL，由 C++ 使用 `QUrl` 解析本地路径；普通路径和 UNC 本地文件 URL 均受支持，非本地 URL 会显示错误。保存使用原子替换，失败时不会截断既有目标。
- 转换失败时返回空结果，不保留部分转换内容。
