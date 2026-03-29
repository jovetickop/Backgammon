# DDD 架构设计方案

## 1. 概述

本文档描述五子棋游戏项目的领域驱动设计（DDD）架构重构方案。DDD 是一种通过深入理解业务领域来设计软件系统的方法论，核心思想是将业务逻辑与技术实现分离，使代码更好地反映业务概念。

## 2. DDD 核心概念

### 2.1 值对象（Value Object）

**定义**：没有唯一标识的不可变对象，两个值对象相等取决于其属性值而非身份。

**在五子棋中的应用**：
- `Position`：坐标值对象，表示棋盘上的落子位置（行、列）
- `Piece`：棋子值对象，表示棋子类型（空、黑、白）
- `Move`：落子值对象，包含位置和棋子类型

**设计原则**：
- 不可变：创建后不能修改
- 相等性基于属性值
- 无副作用的方法

### 2.2 聚合根（Aggregate Root）

**定义**：聚合是一组相关对象的集合，聚合根是聚合的入口点，负责维护聚合内部的一致性规则。外部对象只能通过聚合根来操作聚合内部的对象。

**在五子棋中的应用**：`GameBoard`（棋盘聚合根）

**职责**：
- 管理 15x15 棋盘状态
- 提供落子、悔棋等操作入口
- 确保棋盘状态的一致性（如不能重复落子）
- 维护业务规则（边界检查、空位检查）

**设计原则**：
- 外部只能通过聚合根的方法修改状态
- 聚合根负责保护业务不变式
- 聚合内部可以互相引用，但外部只能引用聚合根

### 2.3 领域服务（Domain Service）

**定义**：当某个操作不属于任何实体或值对象时，创建领域服务来承载这部分业务逻辑。

**在五子棋中的应用**：

1. **WinDetector**（胜负检测服务）
   - 检测横向、纵向、斜向是否形成五连
   - 判断当前局面是否已分出胜负

2. **BoardEvaluator**（局面评估服务）
   - 评估当前局面分值
   - 检测各类棋型（活三、冲四、活四等）
   - 计算双方优势

3. **AIEngine**（AI引擎服务）
   - 极大极小搜索
   - Alpha-Beta 剪枝
   - 候选点生成与排序

**设计原则**：
- 领域服务是无状态的
- 不持有业务状态，只处理业务逻辑
- 可以被多个聚合根共享使用

### 2.4 应用层（Application Layer）

**定义**：应用层协调领域对象完成用例，不包含业务逻辑，只负责流程控制。

**在五子棋中的应用**：`GameService`（游戏流程服务）

**职责**：
- 协调棋盘、胜负检测、局面评估等服务
- 管理游戏流程（开始、落子、结束）
- 处理UI与领域层的交互
- 不执行业务规则，只调度领域对象

**设计原则**：
- 薄层：只做流程编排，不含业务逻辑
- 应用服务可以被UI层直接调用
- 处理事务边界（如果有）

## 3. 分层架构

```
┌─────────────────────────────────────────┐
│           Presentation (UI)             │
│    backgammon.cpp / dialogs / charts    │
└─────────────────────────────────────────┘
                    │
                    ▼
┌─────────────────────────────────────────┐
│           Application Layer             │
│            GameService                  │
└─────────────────────────────────────────┘
                    │
                    ▼
┌─────────────────────────────────────────┐
│              Domain Layer               │
│  ┌─────────────┐  ┌─────────────────┐   │
│  │ 聚合根      │  │ 领域服务        │   │
│  │ GameBoard   │  │ WinDetector     │   │
│  └─────────────┘  │ BoardEvaluator  │   │
│  ┌─────────────┐  │ AIEngine        │   │
│  │ 值对象      │  └─────────────────┘   │
│  │ Position    │                        │
│  │ Piece       │                        │
│  │ Move        │                        │
│  └─────────────┘                        │
└─────────────────────────────────────────┘
                    │
                    ▼
┌─────────────────────────────────────────┐
│         Infrastructure Layer             │
│     PlayerStatsStore (持久化)           │
└─────────────────────────────────────────┘
```

## 4. C++11 兼容性设计

由于项目使用 C++11 标准，方案设计时避免使用以下 C++17 特性：

### 4.1 避免的特性

| C++17 特性 | 替代方案 |
|-----------|---------|
| `std::optional<T>` | 使用指针或单独的成功/失败标志 |
| 嵌套命名空间 `namespace A::B {}` | 使用单层命名空间 `namespace game_core {}` |
| `std::string_view` | 使用 `const std::string&` |
| 结构化绑定 `auto [a, b]` | 使用 `auto pair = ...; auto a = pair.first;` |

### 4.2 命名空间设计

采用单层扁平化命名空间策略：

```cpp
namespace game_core {
    // 值对象
    struct Position { int row; int col; };
    enum Piece { NONE, WHITE, BLACK };

    // 聚合根
    class GameBoard { /* ... */ };

    // 领域服务
    class WinDetector { /* ... */ };
    class BoardEvaluator { /* ... */ };
    class AIEngine { /* ... */ };
}
```

## 5. 目录结构

```
src/
├── domain/
│   ├── values/
│   │   ├── position.h          // 坐标值对象
│   │   ├── piece.h             // 棋子值对象
│   │   └── move.h              // 落子值对象
│   ├── aggregates/
│   │   ├── game_board.h        // 棋盘聚合根
│   │   └── game_board.cpp
│   └── services/
│       ├── win_detector.h      // 胜负检测服务
│       ├── win_detector.cpp
│       ├── board_evaluator.h   // 局面评估服务
│       ├── board_evaluator.cpp
│       ├── ai_engine.h         // AI引擎服务
│       └── ai_engine.cpp
├── application/
│   ├── game_service.h          // 游戏流程服务
│   └── game_service.cpp
├── infrastructure/
│   └── player_stats_store.h   // 持久化（已有）
├── presentation/
│   └── （现有UI代码）
├── types.h                     // 保留：基础类型定义
├── judgeWinner.h/cpp           // 旧代码（逐步迁移）
├── Evaluation.h/cpp            // 旧代码（逐步迁移）
└── ComputerMove.h/cpp         // 旧代码（逐步迁移）
```

## 6. 核心接口设计

### 6.1 值对象

```cpp
// position.h
#pragma once
namespace game_core {

// 坐标值对象：不可变，表示棋盘上的一个位置
struct Position {
    const int row;  // 行索引 [0, 14]
    const int col; // 列索引 [0, 14]

    Position(int r, int c) : row(r), col(c) {}

    // 静态工厂方法
    static Position Invalid() { return Position(-1, -1); }
    bool isValid() const { return row >= 0 && row < 15 && col >= 0 && col < 15; }
};

} // namespace game_core
```

### 6.2 聚合根

```cpp
// game_board.h
#pragma once
#include "position.h"
#include "piece.h"
#include <array>

namespace game_core {

// 棋盘聚合根：管理15x15棋盘状态
class GameBoard {
public:
    static const int BOARD_SIZE = 15;

    // 构造函数
    GameBoard();

    // 落子操作（返回是否成功）
    bool placePiece(const Position& pos, Piece piece);

    // 获取指定位置的棋子
    Piece getPiece(const Position& pos) const;

    // 获取棋盘数组引用（供领域服务使用）
    const std::array<std::array<Piece, BOARD_SIZE>, BOARD_SIZE>& board() const { return board_; }

    // 清除棋盘
    void reset();

    // 获取最后落子位置
    Position lastMove() const { return last_move_; }

private:
    std::array<std::array<Piece, BOARD_SIZE>, BOARD_SIZE> board_;  // 棋盘状态
    Position last_move_;                                            // 最后落子位置

    // 业务不变式检查
    bool canPlace(const Position& pos) const;
};

} // namespace game_core
```

### 6.3 领域服务

```cpp
// win_detector.h
#pragma once
#include "game_board.h"

namespace game_core {

// 胜负检测服务
class WinDetector {
public:
    // 判断指定棋子是否已获胜
    bool checkWin(const GameBoard& board, Piece piece);

private:
    // 四个方向的五连检测
    bool checkHorizontal(const GameBoard& board, Piece piece);
    bool checkVertical(const GameBoard& board, Piece piece);
    bool checkDiagonalLeft(const GameBoard& board, Piece piece);
    bool checkDiagonalRight(const GameBoard& board, Piece piece);
};

} // namespace game_core
```

### 6.4 应用服务

```cpp
// game_service.h
#pragma once
#include "game_board.h"
#include "win_detector.h"
#include "board_evaluator.h"
#include "ai_engine.h"
#include "move.h"

namespace game_core {

// 游戏结果枚举
enum GameResult {
    RESULT_NONE,    // 未分胜负
    RESULT_WHITE,   // 白方获胜
    RESULT_BLACK,   // 黑方获胜
    RESULT_DRAW     // 平局（暂未使用）
};

// 游戏流程服务
class GameService {
public:
    GameService();

    // 开始新游戏
    void startNewGame();

    // 玩家落子
    bool playerMove(const Position& pos);

    // AI落子
    Position aiMove(int searchDepth);

    // 获取当前游戏结果
    GameResult getResult() const { return result_; }

    // 获取棋盘引用
    const GameBoard& board() const { return board_; }

private:
    GameBoard board_;              // 棋盘聚合根
    WinDetector win_detector_;     // 胜负检测
    BoardEvaluator evaluator_;     // 局面评估
    AIEngine ai_engine_;           // AI引擎

    Piece current_player_;         // 当前执子方
    GameResult result_;             // 游戏结果

    // 切换玩家
    void switchPlayer();
};

} // namespace game_core
```

## 7. 迁移策略

由于现有代码量大，采用渐进式迁移策略：

### 阶段1：创建新架构
1. 创建 `src/domain/` 目录结构
2. 实现值对象和聚合根
3. 实现领域服务

### 阶段2：实现应用层
1. 创建 `src/application/` 目录
2. 实现 `GameService`
3. 与现有代码并行运行

### 阶段3：UI层适配
1. 修改 `backgammon.cpp` 使用 `GameService`
2. 逐步迁移旧代码到新架构
3. 最终移除旧代码

### 阶段4：测试验证
1. 编写单元测试验证领域层
2. 编写集成测试验证应用层
3. UI 人工测试验收

## 8. 依赖关系

```
GameService (应用层)
    │
    ├── GameBoard (聚合根)
    │       └── Position, Piece (值对象)
    │
    ├── WinDetector (领域服务)
    │       └── 依赖 GameBoard
    │
    ├── BoardEvaluator (领域服务)
    │       └── 依赖 GameBoard
    │
    └── AIEngine (领域服务)
            └── 依赖 GameBoard, BoardEvaluator, WinDetector
```

**关键原则**：领域层内部互相依赖，但应用层依赖领域层，UI层依赖应用层。禁止反向依赖。

## 9. 优势

1. **业务语义清晰**：代码直接映射业务概念
2. **可测试性强**：领域层无UI依赖，可单元测试
3. **可维护性高**：职责分离，修改影响范围可控
4. **可扩展性好**：新增功能只需在对应层添加

## 10. 注意事项

1. **禁止循环依赖**：Domain → Application → Domain 禁止
2. **聚合根保护不变式**：所有状态修改通过聚合根方法
3. **领域服务无状态**：不持有业务状态
4. **C++11兼容**：不使用C++17特有功能
