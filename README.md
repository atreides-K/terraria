# Project Terraria


## To do
- [ ] make a badass triangle
- [ ] make paper presentation slide template


    

## Pre-requisites
as per -> https://eliemichel.github.io/LearnWebGPU/getting-started/hello-webgpu.html
## Dawn – Quick Setup

```bash
# Clone Dawn
git clone https://dawn.googlesource.com/dawn dawn
cd dawn

# Fetch dependencies
# Option 1: Using depot_tools
cp scripts/standalone.gclient .gclient
gclient sync

# Option 2: Without depot_tools
# python tools/fetch_dawn_dependencies.py

# Build with CMake 
mkdir -p out/Debug
cd out/Debug
cmake ../..
make
```
In my setups I have created a lib dir in my home dir and built dawn there

**Requirements:** Git, C++20 compiler, Python 3, CMake ≥3.16, Go ≥1.23.
**Linux dependencies (Ubuntu):**

```bash
sudo apt-get install libxrandr-dev libxinerama-dev libxcursor-dev mesa-common-dev libx11-xcb-dev pkg-config nodejs npm
```

# Terraria Setup
Enter the location of your dawn build(here i gave mine)
```bash
cmake -B build -DDAWN_DIR=$HOME/lib/dawn/out/Debug

cmake --build build
```
