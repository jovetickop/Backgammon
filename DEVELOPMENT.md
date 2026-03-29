# Backgammon 开发说明

这份文档是给后续开发者，尤其是接手项目的 AI，快速建立完整上下文用的。目标不是介绍"怎么用"，而是说明"项目现在怎么工作、改哪里、哪些地方容易踩坑、扩展时先看什么"。

## 1. 项目定位

这是一个基于 **Qt QML** 的五子棋人机对战程序，不是联网游戏，也没有服务端。当前功能重点有三类：

- 棋盘对局与 AI 落子
- 局面胜率显示与对局统计
- 基于用户名的本地持久化历史记录

## 2. 技术栈与运行前提

- 语言：C++
- UI：**Qt QML**（已从 Widgets 迁移）
- 构建：CMake
- 支持的 Qt：Qt5
- 常用编译链：`g++ + Qt + Ninja`
- 目标平台：Windows

## 3. 代码目录总览

### QML 界面（qml/）

- [qml/GameUI.qml](qml/GameUI.qml)
  主界面组合，包含背景、左侧面板、右侧棋盘
- [qml/BoardView.qml](qml/BoardView.qml)
  棋盘组件，负责网格绘制、棋子渲染、点击交互
- [qml/ControlPanel.qml](qml/ControlPanel.qml)
  左侧控制面板，包含按钮、设置、统计信息
- [qml/WinRateChart.qml](qml/WinRateChart.qml)
  胜率走势图组件
- [qml/ResultDialog.qml](qml/ResultDialog.qml)
  游戏结果对话框

### C++ 源码（src/）

- [src/main.cpp](src/main.cpp)
  程序入口。创建 QGuiApplication，加载 QML 界面
- [src/gamebridge.h](src/gamebridge.h)
- [src/gamebridge.cpp](src/gamebridge.cpp)
  **核心桥接类**，连接 QML 和 C++ 业务逻辑
- [src/ComputerMove.h](src/ComputerMove.h)
- [src/ComputerMove.cpp](src/ComputerMove.cpp)
  AI 搜索逻辑。候选点生成、极大极小搜索、alpha-beta 剪枝
- [src/Evaluation.h](src/Evaluation.h)
- [src/Evaluation.cpp](src/Evaluation.cpp)
  棋型评估器。把棋盘切成线段，统计活三、冲四、五连等评分
- [src/judgeWinner.h](src/judgeWinner.h)
- [src/judgeWinner.cpp](src/judgeWinner.cpp)
  胜负判断器。只做"五连是否成立"的逻辑
- [src/playerstatsstore.h](src/playerstatsstore.h)
- [src/playerstatsstore.cpp](src/playerstatsstore.cpp)
  本地持久化层。负责用户胜负数据读写
- [src/types.h](src/types.h)
  棋子枚举和一些基础类型

### DDD 领域层（src/domain/）

- [src/domain/aggregates/game_board.cpp](src/domain/aggregates/game_board.cpp)
  棋盘聚合根
- [src/domain/services/ai_engine.cpp](src/domain/services/ai_engine.cpp)
  AI 引擎
- [src/domain/services/board_evaluator.cpp](src/domain/services/board_evaluator.cpp)
  局面评估器
- [src/domain/services/win_detector.cpp](src/domain/services/win_detector.cpp)
  胜判定检测器

### 工程根目录

- [CMakeLists.txt](CMakeLists.txt)
  构建入口
- [build.bat](build.bat)
  Windows 一键编译脚本
- [run.bat](run.bat)
  Windows 一键运行脚本

## 4. 程序启动流程

入口在 [src/main.cpp](src/main.cpp)。

1. 创建 `QGuiApplication`
2. 创建 `QQmlApplicationEngine`
3. 注册 `GameBridge` 到 QML 上下文
4. 加载 `qml/GameUI.qml`
5. 启动事件循环

## 5. QML 与 C++ 交互

### GameBridge 桥接类

[src/gamebridge.cpp](src/gamebridge.cpp) 是核心桥接类，提供以下接口：

**游戏控制**：
- `startGame()` / `resetGame()` - 开始/重置游戏
- `setPlayerStarts(bool)` - 设置谁先手
- `setDifficulty(int)` - 设置 AI 难度
- `handlePlayerMove(int row, int col)` - 处理玩家落子

**信号**（QML 接收）：
- `gameStarted` - 游戏开始
- `movePlaced(int row, int col, bool isPlayer)` - 棋子放置
- `moveCountChanged(int count)` - 手数变化
- `winRateUpdated(int playerRate, int aiRate)` - 胜率更新
- `gameOver(int winner)` - 游戏结束
- `statsUpdated(...)` - 统计更新

**数据获取**：
- `getMoveCount()` / `getPlayerWinRate()` / `getAiWinRate()`
- `getPlayerRateHistory()` / `getAiRateHistory()`

## 6. 界面特性

- 深色渐变背景 (`#0f0c29` → `#302b63` → `#24243e`)
- 玻璃拟态面板效果
- 棋子落下弹性动画（OutBack easing）
- 鼠标悬停高亮
- 渐变按钮
- 阴影效果（DropShadow）

## 7. 当前状态总结

截至这份文档编写时，项目已具备：

- 可玩的五子棋人机对战（QML 界面）
- AI 搜索与启发式评估
- 左侧历史统计和当前局势胜率
- 炫酷的动效和视觉效果
- 基于 QML 的现代化界面

## 8. 待完成功能

以下功能需要从旧代码迁移或重新实现：

- 登录对话框（当前简化）
- 历史对局记录查看
- 完整的 AI 落子逻辑（当前是简化版）
- 胜负判定（当前是简化版）

## 9. 构建与运行

```bash
# 构建
cmake -B build
cmake --build build

# 运行（需要设置 Qt 路径）
set PATH=C:\msys64\ucrt64\bin;%PATH%
build\Backgammon.exe
```

或使用：
```bash
run.bat
```
