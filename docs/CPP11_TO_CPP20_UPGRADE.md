# C++11 到 C++20 升级指南

本文档描述如何将五子棋项目从 C++11 升级到 C++20，并介绍新特性的应用。

## 1. 编译器要求

C++20 需要较新的编译器支持：

| 编译器 | 最低版本 | C++20 支持情况 |
|--------|---------|----------------|
| MSVC   | VS 2019 16.10+ | 完整支持 |
| GCC    | 11.0+  | 完整支持 |
| Clang  | 13.0+  | 完整支持 |

当前项目使用 **CMake** 构建，可通过以下命令检查编译器支持：

```bash
cmake --version
```

## 2. 项目配置升级

### 2.1 CMake 配置变更

在 `CMakeLists.txt` 中修改标准版本：

```cmake
# C++11 (旧)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# C++20 (新)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # 关闭编译器扩展
```

### 2.2 模块支持 (可选)

C++20 引入了模块系统。若要启用模块支持：

```cmake
# 启用 C++20 模块 (实验性)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_MODULES ON)
```

**注意**：模块支持在部分编译器中仍属实验性，建议谨慎使用。

## 3. 主要语言特性升级

### 3.1  Concepts (概念)

用于约束模板参数，替代 SFINAE：

```cpp
// C++11: 使用 SFINAE
template<typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
add(T a, T b) { return a + b; }

// C++20: 使用 Concepts
template<typename T>
requires std::integral<T>
T add(T a, T b) { return a + b; }

// 或使用简写形式
template<std::integral T>
T add(T a, T b) { return a + b; }
```

### 3.2 Ranges (范围)

简化容器和算法操作：

```cpp
#include <ranges>

// C++11: 传统方式
std::vector<int> nums = {1, 2, 3, 4, 5};
std::vector<int> even_nums;
std::copy_if(nums.begin(), nums.end(), std::back_inserter(even_nums),
    [](int n) { return n % 2 == 0; });

// C++20: 使用 Ranges
auto even_nums = nums | std::views::filter([](int n) { return n % 2 == 0; });
```

### 3.3 三个月 (Three-Way Comparison)

简化比较操作符：

```cpp
// C++11: 需要定义多个比较操作符
struct Point {
    int x, y;
    bool operator==(const Point& other) const { return x == other.x && y == other.y; }
    bool operator<(const Point& other) const { return x < other.x || (x == other.x && y < other.y); }
    // 还需要 !=, <=, >, >=
};

// C++20: 使用 spaceship operator
struct Point {
    int x, y;
    auto operator<=>(const Point&) const = default;  // 编译器自动生成所有比较
};
```

### 3.4 constexpr 扩展

C++20 扩展了 constexpr 函数的能力：

```cpp
// C++11: constexpr 限制较多
constexpr int square(int x) { return x * x; }

// C++20: constexpr 可以包含更多操作
constexpr std::array<int, 5> fibonacci() {
    std::array<int, 5> arr{};
    arr[0] = 1;
    arr[1] = 1;
    for (int i = 2; i < 5; ++i) {
        arr[i] = arr[i-1] + arr[i-2];
    }
    return arr;
}
```

### 3.5 std::format

类型安全的字符串格式化：

```cpp
#include <format>

// C++11: 使用 printf 或 QString
printf("Player: %s, Wins: %d\n", name.c_str(), wins);

// C++20: 使用 std::format
std::string msg = std::format("Player: {}, Wins: {}", name, wins);
```

### 3.6 std::span

安全地传递数组视图：

```cpp
#include <span>

// C++11: 传递指针+大小
void processArray(const int* arr, size_t size);

// C++20: 使用 std::span
void processArray(std::span<const int> arr);
```

### 3.7 结构化绑定

解包元组和对象：

```cpp
#include <tuple>

// C++11: 手动访问
auto pair = std::make_pair(1, "hello");
int a = pair.first;
const std::string& b = pair.second;

// C++20: 结构化绑定
auto [a, b] = std::make_pair(1, "hello");

// 适用于对象成员
struct Point { int x; int y; };
Point p{1, 2};
auto [px, py] = p;
```

## 4. 项目实际应用

### 4.1 值对象使用 constexpr

```cpp
// position.h - 使用 constexpr 优化
class Position {
public:
    static constexpr int BOARD_SIZE = 15;

    constexpr Position(int row, int col) noexcept
        : row_(row), col_(col) {}

    constexpr bool isValid() const noexcept {
        return row_ >= 0 && row_ < BOARD_SIZE &&
               col_ >= 0 && col_ < BOARD_SIZE;
    }
};
```

### 4.2 使用 std::array 替代 C 风格数组

```cpp
// C++11: 使用 std::array
std::array<std::array<Piece, BOARD_SIZE>, BOARD_SIZE> board_;

// 初始化
board_ = {};  // 值初始化
```

### 4.3 使用 std::optional (C++17+)

```cpp
#include <optional>

// 表示可能不存在的值
std::optional<Position> findBestMove();
```

### 4.4 使用 std::variant (C++17+)

```cpp
#include <variant>

// 表示多种可能的状态
std::variant<GamePlaying, GameWon, GameDraw> gameState_;
```

## 5. 迁移步骤

1. **更新 CMakeLists.txt**
   ```cmake
   set(CMAKE_CXX_STANDARD 20)
   ```

2. **逐步启用新特性**
   - 从 `constexpr` 优化开始
   - 使用 `std::array` 替代原生数组
   - 采用结构化绑定

3. **测试验证**
   ```bash
   cmake -S . -B build
   cmake --build build --config Debug
   ctest --test-dir build -C Debug --output-on-failure
   ```

4. **可选：模块化改造**
   - 创建 `.ixx` 模块接口文件
   - 使用 `import` 替代 `#include`
   - 注意：模块支持在部分 IDE 中尚不完善

## 6. 注意事项

- **向后兼容**：C++20 完全向后兼容 C++11
- **IDE 支持**：部分 IDE 对 C++20 新特性（尤其是模块）支持不完整
- **编译器版本**：确保使用支持 C++20 的编译器版本
- **Qt 兼容性**：Qt6 对 C++20 支持更好，如使用 Qt5 需注意

## 7. 相关文档

- [DDD 架构设计](./DDD_ARCHITECTURE.md)
- [C++20 标准文档](https://en.cppreference.com/w/cpp/20)
