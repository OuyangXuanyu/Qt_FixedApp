# Qt FixedApp

Qt FixedApp 是一个基于 Qt 6 的跨平台桌面应用，包含用户登录、患者信息管理、蓝牙/串口设备连接、实时信号显示、数据保存和历史数据回放等功能。

当前项目已从 Windows 迁移并适配到 macOS，同时保留 Windows 构建支持。

## 开发环境

- CMake 3.22 或更高版本
- C++20 编译器
- Qt 6.10.2
- Qt 模块：Core、Gui、Widgets、OpenGLWidgets、SerialPort、Bluetooth、Network

项目内已包含 SQLite 源码，无需另外安装 SQLite。

## macOS 构建

```bash
cmake -S . -B cmake-build-debug \
  -DQT_ROOT=/Volumes/Mac_ExDisk/Qt/Qt_Source/6.10.2/macos
cmake --build cmake-build-debug --parallel
```

生成的应用位于：

```text
cmake-build-debug/Qt_Project_FixedApp.app
```

## Windows 构建

请安装 Qt 6.10.2 MSVC 2022 64-bit 套件，然后执行：

```powershell
cmake -S . -B cmake-build-debug -DQT_ROOT=C:/Qt/6.10.2/msvc2022_64
cmake --build cmake-build-debug --parallel
```

也可以直接使用 CLion 或 Qt Creator 打开项目根目录下的 `CMakeLists.txt`。

## 运行数据

用户数据库、患者数据库和采集生成的 CSV 文件保存在系统应用数据目录中，不会写入源码目录：

- macOS：`~/Library/Application Support/Qt Project FixedApp/`
- Windows：`%APPDATA%/Qt Project FixedApp/`

这些运行数据已通过 `.gitignore` 排除，不会提交到 GitHub。

