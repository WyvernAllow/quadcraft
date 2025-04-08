# Quadcraft
A Minecraft clone

# Building
## Prerequisites
* [git](https://git-scm.com/)
* [CMake](https://cmake.org/)
* Any C++17 compatible C++ compiler (clang, gcc, msvc, etc.)

## Build steps
```shell
# 1) Clone the repository and all submodules
git clone --recursive https://github.com/WyvernAllow/quadcraft.git
cd quadcraft

# 2) Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# 3) Build
cmake --build build
```

*`quadcraft.exe` will then be located somewhere in the `build/` directory, depending on the build tools that were used.
The `res/` directory must be placed in the working directory of the executable.*

# Libraries used
* [glad](https://gen.glad.sh/) for loading OpenGL functions
* [glfw](https://www.glfw.org/) for windowing and input
* [glm](https://github.com/g-truc/glm) for mathematics
* [imgui](https://github.com/ocornut/imgui) for debug UI
* [spdlog](https://github.com/gabime/spdlog) for logging
* [stb_image](https://github.com/nothings/stb/tree/master) for loading images

# License
Quadcraft is licensed under the Zlib License, see [LICENSE.txt](https://github.com/WyvernAllow/quadcraft/blob/master/LICENSE.txt) for more details.