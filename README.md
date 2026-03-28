# Backgammon（五子棋人机对战）

这是一个基于 **Qt Widgets + C++** 的五子棋人机对战项目（15x15 棋盘）。

- 玩家执黑落子（鼠标点击棋盘）
- 电脑执白，使用极大极小搜索 + alpha-beta 剪枝选择落点
- 内置胜负判断（横/竖/斜连五）

## 项目说明

- 人机对战逻辑完整：包含落子、胜负判断、重置流程，开箱即玩。
- AI 具备搜索能力：使用极大极小搜索与 alpha-beta 剪枝，明显强于随机策略。
- 棋盘交互精确：点击坐标映射已做吸附与边界处理，落子与网格对齐稳定。
- 界面风格已升级：更大棋盘 + 半透明磨砂面板，观感更现代。
- 工程结构清晰：核心代码集中在 `src/`，构建链路基于 CMake，便于继续迭代。
- 使用门槛低：支持双击 `build.bat` 编译、双击 `run.bat` 运行（可自动补编译）。

## 项目结构

- `src/`：核心源码（UI、AI、胜负判断）
- `CMakeLists.txt`：CMake 构建入口（支持 Qt5/Qt6）
- `build.bat`：一键编译脚本
- `run.bat`：一键运行脚本

## 编译与运行

### 1. 一键编译

```bat
build.bat -clean -config Release
```

常用参数：

- `-config Debug|Release`
- `-buildDir build`
- `-qtPrefixPath <Qt根目录>`
- `-generator <生成器名称>`

### 2. 一键运行

```bat
run.bat -config Release
```

如果未编译可自动编译后运行：

```bat
run.bat -buildIfMissing -config Release
```

### 3. 直接使用 CMake（可选）

```bat
cmake -S . -B build -G Ninja -DCMAKE_CXX_COMPILER=g++
cmake --build build --config Release
```

## 说明

- 当前仓库已验证可在 `g++ + Qt5 + CMake + Ninja` 下编译并启动。
- 编译中可能出现 `qrand/qsrand` 的弃用警告，不影响程序运行。
