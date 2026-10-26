# 需求：XML 转 Properties

## 背景与目标

提供一个 Windows 桌面工具，将 XML 配置转换为 `.properties` 配置格式，既支持文件转换，也支持在界面左右输入框中直接转换。

## 功能描述

### 核心功能

1. 左侧输入 XML 文本，点击转换后在右侧输出 properties 文本。
2. 打开 UTF-8 XML 文件并载入左侧编辑框。
3. 将右侧 properties 内容保存为 UTF-8 文件。
4. 展示 XML 解析错误和文件读写错误。
5. 支持清空输入和输出。

### 边界与约束

- XML 使用点号路径展开：`server.database.host`。
- XML 属性使用 `.@` 展开：`server.@port`。
- 同名兄弟节点重复出现时使用 `[0]`、`[1]` 下标。
- 空叶子节点输出空值；纯空白格式化文本不作为配置值输出。
- 混合内容 XML 以元素文本和子元素为主，不保证保留原始 XML 排版。
- 文件操作同时接受普通本地路径和本地 `file://` URL（包括 UNC 文件 URL），非本地 URL 必须返回明确错误。
- 不引入网络服务或外部运行时依赖。

## 输入输出

- 输入：XML 文本或 UTF-8 XML 文件。
- 输出：properties 文本或 UTF-8 `.properties` 文件。
- properties 键和值中的反斜杠、分隔符、控制字符按 Java properties 规则转义。

## 非功能需求（性能/安全/兼容性）

- 目标平台为 Windows。
- 使用 Qt 6.3 或更高版本、Qt Quick/QML 和 CMake。
- 解析失败不得生成误导性的部分结果。
- 文件读写失败必须返回明确错误信息。
- 转换核心可脱离界面进行自动化测试。

## 依赖

- Qt 6.3 或更高版本的 Core、Gui、Qml、Quick、QuickControls2、QuickDialogs2、Test。
- CMake 3.21 或更高版本。

## 验收标准

- 能将嵌套 XML 转成点号路径 properties。
- 能转换 XML 属性和重复节点。
- 非法 XML 能显示错误而不是输出部分结果。
- 界面可打开 XML 文件、转换文本并保存 properties 文件。
- 核心转换测试全部通过。
- 项目根目录不存在 `AGENTS.md`，项目指令位于 `.opencode/AGENTS.md`。
