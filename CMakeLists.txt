cmake_minimum_required(VERSION 3.16)

project(BackgammonQt LANGUAGES CXX)

# 使用 C++17 以兼容 Qt5
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置 Qt5 路径
set(CMAKE_PREFIX_PATH "C:/msys64/ucrt64")

# 启用 Qt 自动代码生成。
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 强制使用 Qt5 Widgets
find_package(Qt5 REQUIRED COMPONENTS Widgets Multimedia Sql Test)

# 启用测试
enable_testing()

# 核心源码 - Qt Widgets UI
set(BACKGAMMON_SOURCES
    src/main.cpp
    src/backgammon.cpp
    src/logindialog.cpp
    src/historydialog.cpp
    src/resultdialog.cpp
    src/replaydialog.cpp
    src/winratechart.cpp
    src/gamebridge.cpp
    src/AiWorker.cpp
    src/playerstatsstore.cpp
    src/soundmanager.cpp
    src/sgf_serializer.cpp
    # DDD 领域层
    src/domain/aggregates/game_board.cpp
    src/domain/services/win_detector.cpp
    src/domain/services/board_evaluator.cpp
    src/domain/services/ai_engine.cpp
    src/domain/services/transposition_table.cpp
    src/domain/services/opening_book.cpp
    src/domain/services/difficulty_config.cpp
    # DDD 应用层
    src/application/game_service.cpp
)

set(BACKGAMMON_HEADERS
    src/backgammon.h
    src/logindialog.h
    src/historydialog.h
    src/resultdialog.h
    src/replaydialog.h
    src/winratechart.h
    src/gamebridge.h
    src/playerstatsstore.h
    src/types.h
    # DDD 领域层头文件
    src/domain/domain.h
    src/domain/values/values.h
    src/domain/aggregates/aggregates.h
    src/domain/services/services.h
    # DDD 应用层头文件
    src/application/application.h
)

# UI 文件
set(BACKGAMMON_UI
    src/backgammon.ui
)

# 资源文件
set(BACKGAMMON_RESOURCES
    src/backgammon.qrc
)

# 生成 Windows GUI 程序
add_executable(Backgammon WIN32
    ${BACKGAMMON_SOURCES}
    ${BACKGAMMON_HEADERS}
    ${BACKGAMMON_UI}
    ${BACKGAMMON_RESOURCES}
)

# 链接 Qt5 Widgets
target_link_libraries(Backgammon PRIVATE
    Qt5::Widgets
    Qt5::Core
    Qt5::Gui
    Qt5::Multimedia
    Qt5::Sql
)

# 简单的单元测试（使用GoogleTest）
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# 确保GoogleTest找到Qt的头文件
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${CMAKE_CURRENT_SOURCE_DIR}/src")

set(UNIT_TEST_SOURCES
    tests/test_unit.cpp
    src/types.h
    src/playerstatsstore.cpp
    # DDD 领域层
    src/domain/aggregates/game_board.cpp
    src/domain/services/win_detector.cpp
    src/domain/services/board_evaluator.cpp
    src/domain/services/ai_engine.cpp
    src/domain/services/transposition_table.cpp
    src/domain/services/opening_book.cpp
    src/domain/services/difficulty_config.cpp
    src/application/game_service.cpp
)

add_executable(TestUnit ${UNIT_TEST_SOURCES})
target_include_directories(TestUnit PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${gtest_SOURCE_DIR}/include
)
# 测试文件自带 main()，链接 gtest（不含 gtest_main）
target_link_libraries(TestUnit PRIVATE
    gtest
    Qt5::Core
    Qt5::Widgets
)

# 注册测试
add_test(NAME TestUnit COMMAND TestUnit)
