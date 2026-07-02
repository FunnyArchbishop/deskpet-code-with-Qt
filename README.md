# DeskPet Qt — 桌面宠物

基于 **Qt6** 的桌宠应用，纯 Qt 库实现，跨平台无原生 Windows API 依赖。
使用 **PNG 图片** 作为角色素材，支持透明通道，可随时替换为自己的角色。

## 特性

- ? 透明置顶无边框窗口
- ?? PNG 图片素材，原生透明通道（无需色键）
- ? 自带橙猫默认素材，可替换任意 PNG 图片
- ? 动画状态：空闲（眨眼）、行走、坐下、特殊触发
- ?? 左键拖拽移动、右键菜单触发特殊动画
- ? 自动随机移动 + 屏幕边界弹跳

## 快速开始

### 前置要求
- CMake 3.16+
- Qt6 (Widgets 模块)
- 支持 C++17 的编译器

### 1. 生成精灵图片
```bash
pip install Pillow
python generate_sprites.py
```

### 2. 构建 & 运行
```bash
cmake -B build -DCMAKE_PREFIX_PATH="<Qt6安装路径>"
cmake --build build
./build/deskpet-qt          # Linux / macOS
build\deskpet-qt.exe        # Windows
```

## 自定义角色

替换 `assets/` 目录下的 PNG 图片即可：

| 文件 | 用途 |
|---|---|
| `sprite_idle.png` | 站立 / 空闲 |
| `sprite_blink.png` | 眨眼 |
| `sprite_walk1.png` | 走路帧1 |
| `sprite_walk2.png` | 走路帧2 |
| `sprite_sit.png` | 坐下 |
| `triggers/happy.png` | 开心触发动画 |

- 建议使用 **128×128** 以上的 **PNG** 图片（带透明通道）
- 程序会自动缩放到 512×512 窗口
- 文件名保持不变，直接替换即可

## 项目结构

```
deskpet-code-with-Qt/
├── CMakeLists.txt           # CMake 构建配置
├── main.cpp                 # 程序入口 (QApplication)
├── petwindow.h/cpp          # 宠物窗口 (透明 + 渲染 + 交互)
├── pet_state.h              # 状态枚举 & 配置常量
├── sprite_loader.h/cpp      # 图片加载器 (PNG → QPixmap)
├── generate_sprites.py      # 默认精灵图片生成脚本
└── assets/                  # 精灵图片素材
    ├── sprite_idle.png
    ├── sprite_blink.png
    ├── sprite_walk1.png
    ├── sprite_walk2.png
    ├── sprite_sit.png
    └── triggers/
        └── happy.png
```