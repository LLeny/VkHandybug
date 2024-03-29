# VkHandybug

Debugger for the Atari Lynx emulator [Handy](https://handy.sourceforge.net/).

Based on ImGui, GLFW/Vulkan, reasonably cross platform (Linux - GCC 12.1, Windows - Visual Studio 2022, macOS - Xcode 14.2).

![](/assets/screen1.jpg)

# Binaries
Latest Windows binaries and Linux Flatpak can be found on the [releases](https://github.com/LLeny/VkHandybug/releases) page.

# Usage
## First run
You will first need to set the Lynx ROM image path, please use the ```MAIN -> Settings...``` menu to do so.

## Breakpoint condition
The breakpoint condition uses [Lua](https://www.lua.org/), please refer to [Lua's documentation](https://www.lua.org/docs.html) for more information on the syntax.
You can access the Lynx memory by using the different accessors ```RAM, ROM, MIKEY, SUZY, CPU, CART, EEPROM``` by simply using them as arrays, for example ```RAM[0x0012], MIKEY[0xFD02]```.
You can also access the CPU state by using the ```REGS``` accessor, it has the following properties: ```A, X, Y, PC, PS, SP```, for example ```REGS.X```. 

The condition can be edited by simply clicking on the condition cell of the corresponding breakpoint.
```
RAM[0x50FD] > 12 and REGS.X ~= 0
```

If you need the breakpoint condition to be able to trigger at any ```PC```, use a breakpoint address of ```0```.

## Symbols
To load symbols create a file with the same name as the cart but with the .lbl extension.
The *lbl* file can be a Vice label file (generated by [cl65](https://cc65.github.io/doc/cl65.html) -Ln option) or simply be a list of "addr label".

```
  0034 here
  4ADE andhere
```

# Building
## Dependencies

- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/), please refer to your system's [vulkan "Getting Started"](https://www.lunarg.com/vulkan-sdk/) page for installation.
- [CMake](https://cmake.org/) is required as the build files generator.

## Compiling
### Linux
```
git clone https://github.com/LLeny/VkHandybug.git
cd VkHandybug
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

#### AppImage
```
cmake --build . --target=vkhandybug.AppImage
```

The binary should be created in the folder ```./bin```   

### OSX
```
git clone https://github.com/LLeny/VkHandybug.git
cd VkHandybug
mkdir build
cd build
cmake -G Xcode -DCMAKE_BUILD_TYPE=Release ..
```
Open the generated ```vkhandybug.xcodeproj``` with XCode   
Select ```vkhandybug``` scheme  
Run / build   

# Dependencies
- [cereal](https://uscilab.github.io/cereal/)
- [Dear ImGui](https://github.com/ocornut/imgui)
- [GLFW](https://www.glfw.org/)
- [hash-library](https://create.stephan-brumme.com/hash-library/)
- [ImGuiFileDialog](https://github.com/aiekick/ImGuiFileDialog)
- [imgui_memory_editor](https://github.com/ocornut/imgui_club)
- [Lua](https://www.lua.org/)
- [miniaudio](https://miniaud.io/)
- [sol2](https://github.com/ThePhD/sol2)
- [vk-bootstrap](https://github.com/charles-lunarg/vk-bootstrap)
- [Vulkan Memory Allocator](https://gpuopen.com/vulkan-memory-allocator/)
- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/)
- [zlib](https://github.com/madler/zlib)
- [{fmt}](https://fmt.dev/latest/index.html)
