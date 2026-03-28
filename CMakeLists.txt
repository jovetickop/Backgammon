cmake_minimum_required(VERSION 3.16)

project(BackgammonQt LANGUAGES CXX)

# 保持与原工程一致的 C++ 标准。
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用 Qt 自动代码生成。
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 同时支持 Qt5 和 Qt6。
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# 核心源码。
set(BACKGAMMON_SOURCES
    src/main.cpp
    src/backgammon.cpp
    src/ComputerMove.cpp
    src/Evaluation.cpp
    src/judgeWinner.cpp
    src/logindialog.cpp
    src/playerstatsstore.cpp
    src/resultdialog.cpp
    src/winratechart.cpp
)

set(BACKGAMMON_HEADERS
    src/backgammon.h
    src/ComputerMove.h
    src/Evaluation.h
    src/judgeWinner.h
    src/logindialog.h
    src/playerstatsstore.h
    src/resultdialog.h
    src/types.h
    src/winratechart.h
)

set(BACKGAMMON_FORMS
    src/backgammon.ui
)

set(BACKGAMMON_RESOURCES
    src/backgammon.qrc
)

# 生成 Windows GUI 程序。
add_executable(Backgammon WIN32
    ${BACKGAMMON_SOURCES}
    ${BACKGAMMON_HEADERS}
    ${BACKGAMMON_FORMS}
    ${BACKGAMMON_RESOURCES}
)

# 链接 Qt Widgets。
target_link_libraries(Backgammon PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
