cmake_minimum_required(VERSION 3.16)

project(BackgammonQt LANGUAGES CXX)

# 保持与原 VS 工程一致的 C++ 标准版本。
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 让 CMake 自动处理 Qt 的 moc/uic/rcc 代码生成。
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 在同一个 CMake 文件中同时兼容 Qt5 和 Qt6。
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# 游戏逻辑与界面相关源码。
set(BACKGAMMON_SOURCES
    Backgammon/Backgammon/main.cpp
    Backgammon/Backgammon/backgammon.cpp
    Backgammon/Backgammon/ComputerMove.cpp
    Backgammon/Backgammon/Evaluation.cpp
    Backgammon/Backgammon/judgeWinner.cpp
)

set(BACKGAMMON_HEADERS
    Backgammon/Backgammon/backgammon.h
    Backgammon/Backgammon/ComputerMove.h
    Backgammon/Backgammon/Evaluation.h
    Backgammon/Backgammon/judgeWinner.h
    Backgammon/Backgammon/types.h
)

set(BACKGAMMON_FORMS
    Backgammon/Backgammon/backgammon.ui
)

set(BACKGAMMON_RESOURCES
    Backgammon/Backgammon/backgammon.qrc
)

# 生成 Windows GUI 可执行文件（不弹出额外控制台窗口）。
add_executable(Backgammon WIN32
    ${BACKGAMMON_SOURCES}
    ${BACKGAMMON_HEADERS}
    ${BACKGAMMON_FORMS}
    ${BACKGAMMON_RESOURCES}
)

# 根据检测到的 Qt 主版本链接 Widgets 模块。
target_link_libraries(Backgammon PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
