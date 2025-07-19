# Using Intrusive List as a Library

## Project Structure

The intrusive list is now organized as a proper header-only library:

```
IntrusiveList/
├── intrusive_list/           # Header-only library
│   ├── CMakeLists.txt       # INTERFACE library definition
│   └── intrusive_list.hpp   # Main header file
├── CMakeLists.txt           # Main project with tests and examples
├── example.cpp              # Complete usage example
├── test_intrusive_list.cpp  # Comprehensive test suite
└── README.md                # Documentation
```

## Using in Your Own Project

### Method 1: Copy the Header File

Simply copy `intrusive_list/intrusive_list.hpp` to your project and include it:

```cpp
#include "intrusive_list.hpp"

struct MyData {
    int value;
    intrusive::list_node link;
    
    MyData(int v) : value(v) {}
};

int main() {
    intrusive::intrusive_list<MyData, &MyData::link> my_list;
    
    MyData obj1(42);
    MyData obj2(84);
    
    my_list.push_back(obj1);
    my_list.push_back(obj2);
    
    for (const auto& item : my_list) {
        std::cout << item.value << std::endl;
    }
    
    return 0;
}
```

### Method 2: Use as CMake Subdirectory

Add this project as a subdirectory in your CMake project:

```cmake
# In your CMakeLists.txt
add_subdirectory(path/to/IntrusiveList/intrusive_list)

# Create your executable
add_executable(my_app main.cpp)

# Link against the intrusive_list library
target_link_libraries(my_app PRIVATE intrusive_list)
```

Then in your C++ code:

```cpp
#include "intrusive_list.hpp"
// Use as shown above
```

### Method 3: Use with FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
  intrusive_list
  GIT_REPOSITORY https://github.com/your-repo/IntrusiveList.git
  GIT_TAG        main
  SOURCE_SUBDIR  intrusive_list  # Only fetch the library part
)

FetchContent_MakeAvailable(intrusive_list)

# Link against the library
target_link_libraries(my_app PRIVATE intrusive_list)
```

## Building and Testing This Project

```bash
# Clone the repository
git clone <repository-url>
cd IntrusiveList

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build everything (library, tests, examples)
cmake --build .

# Run tests
ctest

# Run example
./Debug/example.exe      # Windows
./example               # Linux/macOS
```

## Key Features

- **Header-Only**: No compilation required for the library itself
- **C++17 Compatible**: Uses modern C++ features
- **STL-Compatible**: Standard iterator interface
- **Zero Allocation**: No dynamic memory allocation
- **Type Safe**: Compile-time checks
- **Cross-Platform**: Works on Windows (MSVC), Linux (GCC), macOS (Clang)

## Performance

All operations are O(1) except iteration which is O(n):
- `push_front()`, `push_back()`: O(1)
- `pop_front()`, `pop_back()`: O(1)  
- `insert()`, `erase()`: O(1)
- Memory overhead: Only 2 pointers per object (16 bytes on 64-bit)

## Safety Notes

1. **Object Lifetime**: Objects must outlive the list
2. **Single Membership**: An object can only be in one instance of a specific list type at a time
3. **Automatic Cleanup**: Objects are automatically unlinked when destroyed
4. **Thread Safety**: Not thread-safe, requires external synchronization 