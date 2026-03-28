# Backgammon 开发说明

这份文档是给后续开发者，尤其是接手项目的 AI，快速建立完整上下文用的。目标不是介绍“怎么用”，而是说明“项目现在怎么工作、改哪里、哪些地方容易踩坑、扩展时先看什么”。

## 1. 项目定位

这是一个基于 Qt Widgets 的五子棋人机对战程序，不是联网游戏，也没有服务端。当前功能重点有三类：

- 棋盘对局与 AI 落子
- 局面胜率显示与对局统计
- 基于用户名的本地持久化历史记录

目前没有真正意义上的账号系统，所谓“登录”本质上是选择一个本地用户名，然后把历史统计挂在这个用户名下面。

## 2. 技术栈与运行前提

- 语言：C++
- UI：Qt Widgets
- 构建：CMake
- 支持的 Qt：Qt5 / Qt6
- 常用编译链：`g++ + Qt + Ninja`
- 目标平台：Windows

当前仓库主要按 Windows 桌面程序来维护。`build.bat` 和 `run.bat` 都是围绕 Windows 环境写的。

## 3. 代码目录总览

核心代码都在 `src/` 下：

- [src/main.cpp](C:/code/Backgammon/src/main.cpp)
  程序入口。负责启动 Qt、加载用户历史、弹登录框、再创建主窗口。
- [src/backgammon.h](C:/code/Backgammon/src/backgammon.h)
- [src/backgammon.cpp](C:/code/Backgammon/src/backgammon.cpp)
  主窗口和主要交互逻辑。棋盘绘制、玩家落子、AI 落子、局面评估、结算、历史统计更新都在这里。
- [src/backgammon.ui](C:/code/Backgammon/src/backgammon.ui)
  主窗口布局文件。左侧信息面板、按钮、右侧棋盘视图都在这里定义。
- [src/ComputerMove.h](C:/code/Backgammon/src/ComputerMove.h)
- [src/ComputerMove.cpp](C:/code/Backgammon/src/ComputerMove.cpp)
  AI 搜索逻辑。候选点生成、极大极小搜索、alpha-beta 剪枝都在这里。
- [src/Evaluation.h](C:/code/Backgammon/src/Evaluation.h)
- [src/Evaluation.cpp](C:/code/Backgammon/src/Evaluation.cpp)
  棋型评估器。把棋盘切成线段，统计活三、冲四、五连等评分。
- [src/judgeWinner.h](C:/code/Backgammon/src/judgeWinner.h)
- [src/judgeWinner.cpp](C:/code/Backgammon/src/judgeWinner.cpp)
  胜负判断器。只做“五连是否成立”的逻辑。
- [src/logindialog.h](C:/code/Backgammon/src/logindialog.h)
- [src/logindialog.cpp](C:/code/Backgammon/src/logindialog.cpp)
  登录对话框。当前用可编辑下拉框输入用户名，同时预览该用户名的历史数据。
- [src/playerstatsstore.h](C:/code/Backgammon/src/playerstatsstore.h)
- [src/playerstatsstore.cpp](C:/code/Backgammon/src/playerstatsstore.cpp)
  本地持久化层。负责用户胜负数据、上次登录用户、最近登录用户名单的读写。
- [src/resultdialog.h](C:/code/Backgammon/src/resultdialog.h)
- [src/resultdialog.cpp](C:/code/Backgammon/src/resultdialog.cpp)
  对局结束时的自定义结算弹框。
- [src/winratechart.h](C:/code/Backgammon/src/winratechart.h)
- [src/winratechart.cpp](C:/code/Backgammon/src/winratechart.cpp)
  左侧胜率走势图绘制控件。
- [src/types.h](C:/code/Backgammon/src/types.h)
  棋子枚举和一些基础类型。

工程根目录重要文件：

- [CMakeLists.txt](C:/code/Backgammon/CMakeLists.txt)
  构建入口。新增源文件时必须同步更新这里。
- [build.bat](C:/code/Backgammon/build.bat)
  Windows 一键编译脚本。
- [run.bat](C:/code/Backgammon/run.bat)
  Windows 一键运行脚本。
- [README.md](C:/code/Backgammon/README.md)
  面向使用者的简版说明。

## 4. 当前程序启动流程

入口在 [src/main.cpp](C:/code/Backgammon/src/main.cpp)。

完整流程如下：

1. 创建 `QApplication`
2. 创建 `PlayerStatsStore`
3. 调用 `Load()` 读取本地用户历史
4. 弹出 `LoginDialog`
5. 用户输入或选择用户名后点击进入
6. 将该用户名写为 `lastUser`
7. 同时把该用户名放到 `recentUsers` 头部
8. 立即保存到本地 JSON
9. 读取该用户的历史胜负记录
10. 用这个 `PlayerRecord` 创建 `Backgammon`

这里有两个设计点要注意：

- 登录不依赖结算后再保存。只要用户名被选中并进入程序，就会立刻更新“上一个用户”和“最近用户列表”。
- 这样做的目的是保证即使用户只登录不下棋，下次打开时用户名下拉和默认用户名也能恢复。

## 5. 登录与用户历史的设计

### 5.1 登录不是真正认证

当前没有密码、没有远程服务、没有 session，只有用户名切换。本质上这是“本地用户档案选择器”，不是账号认证系统。

如果以后要接入真实登录，不要在现有 `LoginDialog` 上直接堆功能。建议单独抽象认证层，把“账号认证”和“本地战绩绑定”拆开。

### 5.2 登录对话框的行为

登录框在 [src/logindialog.cpp](C:/code/Backgammon/src/logindialog.cpp) 中以代码构建，不是 `.ui` 文件。

当前行为：

- 用户名输入控件是 `QComboBox`
- 该 `QComboBox` 是 `editable`
- 下拉项来自 `PlayerStatsStore::RecentUsers()`
- 默认文本来自 `PlayerStatsStore::LastUser()`
- 输入框内容变化时，会调用 `UpdatePreview()`
- `UpdatePreview()` 会实时显示该用户名的历史局数、胜场、败场、胜率

如果以后你发现“下拉没有内容”，先排查下面几件事：

- 本地 `players.json` 是否存在
- JSON 是否是旧格式
- `Load()` 有没有把旧格式迁移出 `recentUsers`
- `main.cpp` 是否在登录成功后调用了 `SetLastUser()` 和 `TouchRecentUser()`

### 5.3 旧数据兼容

[src/playerstatsstore.cpp](C:/code/Backgammon/src/playerstatsstore.cpp) 兼容两种存储格式：

旧格式：

```json
{
  "kop": {
    "displayName": "kop",
    "wins": 0,
    "losses": 3
  }
}
```

新格式：

```json
{
  "players": {
    "kop": {
      "displayName": "kop",
      "wins": 0,
      "losses": 3
    }
  },
  "lastUser": "kop",
  "recentUsers": [
    "kop"
  ]
}
```

兼容逻辑是：

- 如果检测到顶层只有玩家对象，没有 `players` 字段，则按旧格式解析
- 如果旧格式里没有 `recentUsers`，会根据已有玩家列表回填一个最近用户列表
- 如果没有 `lastUser`，会把最近用户列表第一项作为默认用户

这个兼容层是为了让之前已经保存过战绩的用户，在升级到新版本后也能看到用户名下拉。

### 5.4 本地数据存储位置

本地数据由 `PlayerStatsStore::StoragePath()` 决定，当前使用：

- `QStandardPaths::AppDataLocation + "/players.json"`

在当前 Windows 环境下，实际路径一般类似：

- `C:\Users\<用户名>\AppData\Roaming\Backgammon\players.json`

如果用户反馈“历史战绩丢了”，先看这个文件是否存在，以及启动的可执行文件是否还是同一个应用名。

## 6. 主窗口核心职责

[src/backgammon.cpp](C:/code/Backgammon/src/backgammon.cpp) 是当前项目最核心的文件。

它同时负责：

- 棋盘绘制
- 鼠标落子
- AI 落子
- 胜负判断
- 胜率估算与趋势图更新
- 历史胜负统计刷新
- 对局结算弹框
- 最近一手 AI 棋子高亮
- 视图缩放

如果未来需要做大的架构调整，这个类是第一拆分对象。当前它职责偏多，但在项目规模还不大时可接受。

## 7. 棋盘与坐标系统

主窗口里棋盘显示用的是 `QGraphicsScene + QGraphicsView`。

关键常量定义在 [src/backgammon.cpp](C:/code/Backgammon/src/backgammon.cpp) 顶部匿名命名空间中：

- `kBoardSize = 15`
- `kBoardCenter = 7`
- `kGridSize`
- `kPieceSize`
- `kLineMin`
- `kLineMax`
- `kScenePadding`

关键转换函数：

- `BoardToScene(int index)`
  棋盘数组坐标转成场景坐标
- `SceneToBoard(double pos)`
  场景坐标吸附回最近的棋盘格点

这套映射是后续所有绘制和点击判断的基础。如果要改棋盘尺寸、格距、棋子大小，优先改这些常量，不要在各处硬编码。

## 8. 对局过程的数据流

### 8.1 开局

点击开始按钮会进入 [src/backgammon.cpp](C:/code/Backgammon/src/backgammon.cpp) 的 `slotStartBtnClicked()`。

当前设计是 AI 先手：

- 设置按钮文字为“清除”
- 标记 `m_bStarted = true`
- 清空本局胜率历史
- AI 先在天元位置下白子
- 更新本局胜率估算

如果以后要支持“玩家先手 / AI 先手切换”，优先改这里以及与之相关的初始化逻辑。

### 8.2 玩家落子

玩家点击主窗口时，`mousePressEvent()` 会：

1. 判断游戏是否已开始
2. 把窗口坐标映射到 `graphicsView`
3. 再映射到 `scene`
4. 从 `scene` 坐标吸附到棋盘数组坐标
5. 检查越界和是否已有棋子
6. 绘制黑子
7. 更新局面胜率
8. 判断玩家是否获胜

### 8.3 AI 落子

如果玩家未直接获胜，则：

1. 创建 `ComputerMove`
2. 调用 `MaxMinSearch(m_arrBoard, m_nDeep)`
3. 读取 AI 选出的 `(x, y)`
4. 在棋盘上落白子
5. 高亮最近一手 AI 棋子
6. 更新胜率
7. 判断 AI 是否获胜

### 8.4 结算

胜负成立时会：

1. 调用 `RecordGameResult()`
2. 更新累计胜负
3. 持久化当前用户记录
4. 弹 `ResultDialog`
5. 清空棋盘
6. 重置按钮状态

## 9. AI 逻辑说明

这一部分和 README 中的概述一致，但这里补充一些接手时需要知道的点。

### 9.1 搜索入口

AI 决策入口在 [src/ComputerMove.cpp](C:/code/Backgammon/src/ComputerMove.cpp) 的 `MaxMinSearch()`。

头文件 [src/ComputerMove.h](C:/code/Backgammon/src/ComputerMove.h) 表明它当前有三层职责：

- 候选点生成 `GenCandidator`
- 最小层搜索 `MinSearch`
- 最大层搜索 `MaxSearch`

### 9.2 评分方向

[src/Evaluation.h](C:/code/Backgammon/src/Evaluation.h) 已经明确：

- `EvaluateBoard()` 返回的是 `白方分 - 黑方分`

这意味着：

- 分数越大，局面对 AI 越有利
- 分数越小，局面对玩家越有利

如果你以后改颜色、改先手、改阵营定义，一定要同步检查：

- 搜索最大化的是不是仍然对应 AI
- 胜率图里谁是 player / 谁是 ai
- `ScoreToAiWinRate()` 的映射方向是否仍然正确

### 9.3 当前胜率图不是概率模型

左侧“预估胜率”不是通过大量模拟或真实博弈胜率训练出来的统计概率，而是：

- 先用 `Evaluation` 得到启发式分数
- 再通过 `ScoreToAiWinRate()` 压缩映射到 `0% ~ 100%`

所以它更接近“局面强弱可视化”，不是严格意义上的 calibrated probability。

如果以后要做更真实的胜率，可能需要：

- 更深搜索结果做归一化
- 多次 rollout / 蒙特卡洛模拟
- 或训练模型输出胜率

### 9.4 搜索深度

当前 `Backgammon` 内部用 `m_nDeep` 保存搜索深度，默认值是 `8`。

后续如果要开放成配置项，建议不要直接写死在 UI 代码里，最好：

- 单独做设置结构体
- 或新增“游戏配置”对象统一管理

## 10. 左侧信息面板的含义

主窗口左侧当前大致分成几个块：

- 当前用户名
- 历史总局数
- 我 / AI 的历史胜率
- 当前这一局的手数
- 当前这一局的预估胜率
- 胜率走势曲线
- `开始/清除` 按钮
- `?` 按钮

其中：

- 历史数据来自 `PlayerStatsStore`
- 当前局走势来自 `m_playerRateHistory` 和 `m_aiRateHistory`
- `?` 按钮会弹出 AI 运行逻辑说明对话框

如果要继续改左侧信息布局，优先检查 [src/backgammon.ui](C:/code/Backgammon/src/backgammon.ui) 和 [src/backgammon.cpp](C:/code/Backgammon/src/backgammon.cpp) 两处是否同步。

## 11. WinRateChart 的职责边界

[src/winratechart.cpp](C:/code/Backgammon/src/winratechart.cpp) 是纯绘制控件，职责应该尽量保持简单：

- 接收两条序列
- 在 `paintEvent()` 中画出来

不要把“如何算胜率”放进去。胜率计算应该继续留在主窗口或更上层逻辑。

当前推荐分工是：

- `Backgammon` 负责生成数据
- `WinRateChart` 负责渲染数据

## 12. ResultDialog 的职责边界

[src/resultdialog.cpp](C:/code/Backgammon/src/resultdialog.cpp) 只负责显示结算信息，不负责结算本身。

也就是说：

- 谁赢了
- 这一局多少手
- 累计打了多少局
- 双方累计胜场是多少

这些数据都应该在主窗口算好，再传给 `ResultDialog`。

如果以后要加：

- 连胜提示
- 徽章
- 数据统计明细
- 再来一局 / 换先手 / 返回登录

建议还是保持 `ResultDialog` 做纯展示，业务逻辑不要反向塞回对话框里。

## 13. 本地持久化的边界

当前只持久化“用户累计战绩”和“最近用户名单”，没有持久化：

- 单局回放
- 胜率曲线历史
- 每一步棋谱
- 时间戳
- 先后手信息
- 搜索深度配置

如果未来要做“历史对局记录”，建议不要继续往 `players.json` 里硬塞。

更合理的做法是拆分成两层：

- `players.json`：只放用户概览
- `games/` 目录或单独 `games.json`：保存每一局完整记录

建议至少包含：

- 用户名
- 开始时间
- 结束时间
- 先手方
- 赢家
- 每一步坐标
- 当时的搜索深度

## 14. 现在最容易踩坑的点

### 14.1 中文乱码

这个项目之前出现过源文件编码混乱，表现为源码里中文注释或字符串显示成乱码。后续新增中文字符串时，尽量使用：

```cpp
QString::fromUtf8(u8"中文")
```

这样在 Windows 编辑器、终端和 Qt 之间更稳定。

### 14.2 `build.bat` 的重新配置规则

[build.bat](C:/code/Backgammon/build.bat) 当前为了避免静默复用错误缓存，已有 `CMakeCache.txt` 时：

- 如果传了 `-generator`
- 或传了 `-qtPrefixPath`

脚本会要求配合 `-reconfigure` 使用。

所以如果你改了生成器或 Qt 路径，不要直接重复运行普通 `build.bat`，要显式：

```bat
build.bat -reconfigure
```

### 14.3 `QGraphicsView` 缩放

棋盘缩放依赖 `showEvent()` 和 `resizeEvent()` 里调用 `UpdateBoardView()`，其中核心是：

- `resetTransform()`
- `fitInView(..., Qt::KeepAspectRatio)`

如果以后又遇到“棋盘不见了”或“缩放越来越离谱”，先查这里是不是被改坏了。

### 14.4 胜率手数增长逻辑

当前 `UpdateWinRateEstimate()` 每调用一次就会：

- 更新双方预估胜率
- `++m_nMoveCount`
- 向两条历史序列各追加一个点

因此它记录的是“每次落子后的状态点”，不是“一回合一条记录”。如果你想把横轴改成“回合数”而不是“落子数”，这里必须同步改。

### 14.5 AI 最近一手高亮

最近一手 AI 棋子的高亮逻辑在 `SetLastAiPiece()`。

注意：

- 新高亮之前会恢复旧棋子的普通样式
- 当前高亮只是画笔和底色变化，没有动画

如果以后要加呼吸光圈或动效，最好仍然沿用“只有一个当前高亮目标”的思路，不要让旧高亮残留。

## 15. 推荐的扩展顺序

如果要继续把项目往“完整产品”方向推，建议优先级如下：

1. 先后手设置
2. 完整历史对局记录
3. 登录页最近用户体验继续优化
4. AI 难度 / 搜索深度配置
5. 棋谱回放
6. 导出历史记录

原因很简单：

- 这些功能都能建立在现有本地单机架构上
- 不需要先引入网络层
- 能直接复用现有 `PlayerStatsStore`、主窗口和结算流程

## 16. 如果后续 AI 要继续改代码，建议先读哪些文件

按场景分：

### 改 UI 或布局

先读：

- [src/backgammon.ui](C:/code/Backgammon/src/backgammon.ui)
- [src/backgammon.cpp](C:/code/Backgammon/src/backgammon.cpp)
- [src/logindialog.cpp](C:/code/Backgammon/src/logindialog.cpp)
- [src/resultdialog.cpp](C:/code/Backgammon/src/resultdialog.cpp)
- [src/winratechart.cpp](C:/code/Backgammon/src/winratechart.cpp)

### 改 AI

先读：

- [src/ComputerMove.cpp](C:/code/Backgammon/src/ComputerMove.cpp)
- [src/Evaluation.cpp](C:/code/Backgammon/src/Evaluation.cpp)
- [src/judgeWinner.cpp](C:/code/Backgammon/src/judgeWinner.cpp)
- [src/backgammon.cpp](C:/code/Backgammon/src/backgammon.cpp)

### 改用户系统或历史数据

先读：

- [src/main.cpp](C:/code/Backgammon/src/main.cpp)
- [src/logindialog.cpp](C:/code/Backgammon/src/logindialog.cpp)
- [src/playerstatsstore.cpp](C:/code/Backgammon/src/playerstatsstore.cpp)
- [src/backgammon.cpp](C:/code/Backgammon/src/backgammon.cpp)

### 改构建脚本

先读：

- [build.bat](C:/code/Backgammon/build.bat)
- [run.bat](C:/code/Backgammon/run.bat)
- [CMakeLists.txt](C:/code/Backgammon/CMakeLists.txt)

## 17. 当前项目状态总结

截至这份文档编写时，项目已具备：

- 可玩的五子棋人机对战
- AI 搜索与启发式评估
- 左侧历史统计和当前局势胜率
- 登录页用户名输入与最近用户下拉
- 基于用户名的本地持久化
- 自定义结算弹框
- AI 逻辑说明弹框

但它仍然是一个“轻量本地桌面项目”，不是大型架构。继续开发时，应该优先维持：

- 代码职责清晰
- 视图与业务分层
- 本地数据格式稳定
- UI 改动不要破坏棋盘交互和缩放

如果要做大功能，建议开始逐步拆分：

- 游戏状态对象
- 配置对象
- 对局记录仓库
- 独立的设置/历史页面

这样后续无论是人还是 AI，接手成本都会明显更低。
