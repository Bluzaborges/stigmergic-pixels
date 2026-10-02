![Colorful agent trails in the Stigmergic Pixels simulation](images/banner.png)

# Stigmergic Pixels

A real-time SDL2 experiment with agents initialized at random positions and moving in random directions. Each agent leaves a fading trail, senses nearby trail intensity and turns according to the enabled interaction rules. Species receive distinct colors, and agents never follow trails left by another species.

## Install and run on Windows

1. Download the ZIP from the [latest release](https://github.com/Bluzaborges/stigmergic-pixels/releases) and extract all its files into one folder.
2. Open the extracted folder and run `stigmergic-pixels.exe`.

## Build from source

You need Git, a C++17-compatible compiler, CMake 3.21 or later, Ninja, and SDL2. Dear ImGui 1.92.9 is downloaded automatically by CMake during the first configuration.

### Install build tools on Windows (MSYS2 UCRT64)

From an MSYS2 UCRT64 terminal:

```bash
pacman -S --needed git mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-SDL2
```

### Install build tools on Debian or Ubuntu

```bash
sudo apt update
sudo apt install git build-essential cmake ninja-build libsdl2-dev
```

### Compile the application

Create an optimized build from the repository root:

```bash
cmake --preset release
cmake --build --preset release
```

### Run the local build

```bash
./build/release/stigmergic-pixels
```

On Windows, use `./build/release/stigmergic-pixels.exe`.

## Controls

- `Attraction`: follow trails left by the same species
- `Repulsion`: avoid the shared trail with one species, or trails left by other species when multiple species are active
- `Agents`: change the population from 1 to 10,000,000 agents
- `Species`: divide the agents into one to eight color-coded species
- `Speed`: change movement speed while preserving continuous trails
- `Diffusion`: control how quickly trails spread into neighboring pixels
- `Evaporation`: control how quickly trails disappear
- `Reset`: clear the trails and randomize every agent again
- `Space`: pause or resume the simulation
- `R`: clear the trails and randomize every agent again
- `Escape`: close the application

## Develop with Visual Studio Code

Install the Microsoft **C/C++** and **CMake Tools** extensions. On Windows, open the repository from an MSYS2 UCRT64 terminal:

```bash
cd /c/path/to/stigmergic-pixels
code .
```

To debug, open the command palette, run **CMake: Select Configure Preset**, select **Debug**, add a breakpoint, and start **CMake: Debug**.

Make sure the CMake Tools output references `C:\msys64\ucrt64` instead of `C:\mingw64`.
