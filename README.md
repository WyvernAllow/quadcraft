# Quadcraft
A Minecraft clone

# How to Build
## Prerequisites
* [git](https://git-scm.com/)
* [CMake](https://cmake.org/)
* [Vulkan SDK](https://vulkan.lunarg.com/)
* Any C++17 compatible C++ compiler (gcc, clang, MSVC, etc.)

## Building
```shell
git clone --recursive https://github.com/WyvernAllow/quadcraft.git
cd quadcraft
cmake -S . -B build
cmake --build build
```

The resulting executable will be located somewhere in the `build/` directory.

# License
* Quadcraft is licensed under the zlib license, available in [LICENSE.txt](https://github.com/WyvernAllow/quadcraft/blob/master/LICENSE.txt)