# Backgammon（五子棋人机对战）

这是一个基于 **Qt Widgets + C++** 的五子棋人机对战项目（15x15 棋盘）。

- 玩家执黑落子（鼠标点击棋盘）
- 电脑执白，使用极大极小搜索 + alpha-beta 剪枝选择落点
- 内置胜负判断（横/竖/斜连五）
- 实时胜率曲线图
- 历史对局记录与统计
- 可调搜索深度（支持自定义 1-20 层）

## 项目说明

- 人机对战逻辑完整：包含落子、胜负判断、重置流程，开箱即玩。
- AI 具备搜索能力：使用极大极小搜索与 alpha-beta 剪枝，明显强于随机策略。
- 智能防守体系：紧急判断（冲四/活四封堵）、活三必杀检测（双活三拦截）、评估排序候选点。
- 棋盘交互精确：点击坐标映射已做吸附与边界处理，落子与网格对齐稳定。
- 界面风格现代：木纹棋盘 + 3D 棋子 + 半透明磨砂面板 + 实时胜率曲线。
- 工程结构清晰：核心代码集中在 `src/`，构建链路基于 CMake，便于继续迭代。
- 使用门槛低：支持双击 `build.bat` 编译、双击 `run.bat` 运行（可自动补编译）。

## AI 算法逻辑

当前 AI 的核心思路是"紧急判断 + 活三检测 + 候选点评估排序 + 极大极小搜索 + alpha-beta 剪枝 + 启发式局面评估"。

### 1. 紧急判断（最高优先级）

AI 每次落子前先检查紧急情况，跳过搜索直接响应：
- **自己能直接五连** → 立即走获胜位置
- **对手有冲四/活四** → 立即封堵（冲四封一端，活四则无法封堵，走正常搜索）

### 2. 活三必杀检测

- 使用 5 子滑窗在横、竖、两条对角线四个方向上扫描，识别活三（三子连线两端开口）和跳活三。
- 如果对手在多个方向同时形成活三（关键位置数 ≥ 4），属于必杀局面，AI 会优先选择能同时拦截最多活三方向的位置。
- 同理，如果自己有双活三机会，也会优先进攻。

### 3. 候选点生成

- AI 不会在整个 15x15 棋盘上暴力枚举所有空位。
- 只会把"已有棋子八邻域附近的空点"加入候选集合。
- 这样可以把搜索重心放在当前战场附近，明显减少无意义分支。

### 4. 评估排序

- 对每个候选点计算综合分值 = 己方落子增益 + 对手落子价值 × 2。
- 按综合分降序排列，只搜索前 N 个最高分候选点。
- 先搜高分点可以让 alpha-beta 剪枝效率提升数倍，同时防止忽略重要防守位置。

### 5. 搜索方式

- AI 执白，玩家执黑。
- 搜索时使用极大极小搜索：
  - 轮到 AI 时，尽量选择让局面分数更高的走法。
  - 轮到玩家时，假设玩家会选择最压制 AI 的应手。
- 当前默认搜索深度是 `8` 层，可通过下拉框自定义 1-20 层。
  - 这里的"层"表示双方轮流向后推演的搜索层级，而不是单独只算 AI 自己的步数。

### 6. alpha-beta 剪枝

- 在搜索过程中，如果某条分支已经可以判断"不可能比当前已知最优结果更好"，就会提前截断。
- 这样可以在不改变最终决策结果的前提下，大幅降低搜索量。

### 7. 局面评估

- 当搜索到达深度上限，或者已经分出胜负时，程序会对当前棋盘做启发式评分。
- 评分器会统计常见棋型，包括：
  - 活一、活二、活三、活四
  - 冲一、冲二、冲三、冲四
  - 五连
- 最终局面分数使用：

```text
白方总分 - 黑方总分
```

- 分数越高，说明局面对 AI（白方）越有利；分数越低，说明局面对玩家（黑方）越有利。
- 通过 S 型曲线（tanh）映射为胜率百分比，显示在右侧胜率曲线图中。

### 8. 最终落子

- AI 会遍历候选点，搜索每个候选点的后续局面分数。
- 如果有多个候选点同分，会从这些最高分候选中随机选择一个。
- 这样可以避免 AI 每一局都走完全相同的固定套路。

## 项目结构

```
src/
├── backgammon.cpp/h       # 主窗口：棋盘绘制、交互、胜率更新
├── ComputerMove.cpp/h     # AI 决策：极大极小搜索、候选点生成
├── Evaluation.cpp/h       # 局面评估：棋型评分、活三检测、胜率映射
├── judgeWinner.cpp/h      # 胜负判断：五连检测
├── playerstatsstore.cpp/h # 玩家数据持久化
├── winratechart.cpp/h     # 胜率曲线图绘制
├── historydialog.cpp/h    # 历史对局对话框（支持多选删除）
├── resultdialog.cpp/h     # 对局结果对话框
├── types.h                # 类型定义
├── domain/               # DDD 领域层（2025年重构）
│   ├── values/            # 值对象：Position, Piece, Move, GameResult
│   ├── aggregates/       # 聚合根：GameBoard
│   └── services/         # 领域服务：WinDetector, BoardEvaluator, AIEngine
└── application/          # DDD 应用层：GameService
tests/                     # 单元测试（GoogleTest）
docs/
├── DDD_ARCHITECTURE.md       # DDD 架构设计文档
└── CPP11_TO_CPP20_UPGRADE.md # C++11 到 C++20 升级指南
CMakeLists.txt             # CMake 构建入口（支持 Qt5/Qt6，C++20）
build.bat                  # 一键编译脚本
run.bat                    # 一键运行脚本
DEVELOPMENT.md             # 面向后续开发者的详细开发说明
```

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

## 单元测试

项目使用 **GoogleTest** 框架进行单元测试。

### 运行测试

```bat
cmake --build build --target TestUnit
build\TestUnit.exe
```

或使用 CTest：

```bat
ctest --test-dir build --output-on-failure
```

### 测试覆盖

- `types.h`：棋子枚举值、评分枚举关系
- `judgeWinner`：胜负判断（横/竖/斜连五）
- `Evaluation`：局面评估（空盘、黑优、白优、五连）

### 新增功能测试

每次新增功能后：
1. 在 `tests/test_unit.cpp` 中添加对应的测试函数
2. 运行 `TestUnit.exe` 验证通过
3. 确认无误后再提交代码

## 说明

- 当前仓库已验证可在 `g++ + Qt5 + CMake + Ninja` 下编译并启动。
- 编译中可能出现 `qrand/qsrand` 的弃用警告，不影响程序运行。
- 如果你要继续改功能、接手维护或让其他 AI 继续开发，优先阅读 [DEVELOPMENT.md](DEVELOPMENT.md)。
