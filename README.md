# Quadcraft
Minecraft clone written in C using the new SDL [GPU API](https://wiki.libsdl.org/SDL3/CategoryGPU)

# Build instructions
## Prerequisites
* [git](https://git-scm.com/)
* [cmake](https://cmake.org/)
* [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/)
* A C99 compatible C compiler (gcc, clang, MSVC, etc.)

# Instructions
1. Clone the repository and all submodules
```shell
git clone --recursive https://github.com/WyvernAllow/quadcraft.git
```

2. Configure with CMake
```shell
cmake quadcraft
```

3. Build with CMake
```shell
cmake --build quadcraft --config Release
```

# Dependencies
* [SDL](https://libsdl.org/)
* [FastNoiseLite](https://github.com/Auburn/FastNoiseLite)

# License
Quadcraft is distributed under the zlib license, available in the file "LICENSE.txt"