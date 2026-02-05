# Burraq Engine

A work-in-progress rendering engine built from scratch with Vulkan.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://github.com/mosamaasif/Burraq_Engine/blob/master/LICENSE)

## About

Burraq Engine is a barebone rendering engine I built to learn the Vulkan graphics API. It features a modular architecture with a complete Vulkan abstraction layer, an event-driven input system, and a configurable graphics pipeline with automatic shader reflection. The repo includes a Minecraft-style voxel demo as a reference implementation.

## Features

- **Vulkan Rendering Pipeline** — Swapchain management, descriptor sets, push constants, and 3-frame pipelining
- **Automatic Shader Reflection** — Descriptor set layouts generated from SPIR-V bytecode via SPIR-V-Reflect
- **Mesh Loading** — OBJ model loading with vertex/index buffer management
- **Texture System** — 2D textures and cubemap support for skyboxes
- **Skybox Rendering** — Configurable skybox with dedicated shaders and cubemap sampling
- **Camera System** — Perspective camera with mouse-look and keyboard controls
- **Event System** — Type-safe event dispatcher for keyboard, mouse, and window events
- **Input Management** — Centralized keyboard (512 keys) and mouse (8 buttons) tracking
- **Logging** — Multi-level logger (Info/Trace/Warn/Error/Critical) with color-coded console and file output
- **Minecraft Demo** — Chunk-based voxel world (5x5 chunks, 16x16x16 blocks each) with multiple block types

## Built With

- C++ / Vulkan
- [GLFW](https://www.glfw.org/) — Window management
- [GLM](https://github.com/g-truc/glm) — Mathematics
- [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) — GPU memory management
- [SPIR-V-Reflect](https://github.com/KhronosGroup/SPIRV-Reflect) — Shader reflection
- [stb_image](https://github.com/nothings/stb) — Image loading
- [meshoptimizer](https://github.com/zeux/meshoptimizer) — Mesh optimization

## Getting Started

### Prerequisites

- Windows (the engine uses Win32 APIs)
- [Vulkan SDK](https://vulkan.lunarg.com/)
- Visual Studio 2019+

### Building

1. Clone the repo
   ```sh
   git clone https://github.com/mosamaasif/Burraq_Engine.git
   ```
2. Open `Burraq.sln` in Visual Studio
3. Build and run the `Minecraft` project (the demo application)

Shaders are pre-compiled to SPIR-V. To recompile after editing, run `Minecraft/Resources/Shaders/ShaderCompilerScript.bat` (requires `glslangValidator` from the Vulkan SDK).

## License

Distributed under the MIT License. See [LICENSE](LICENSE) for details.
