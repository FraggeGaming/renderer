#pragma once
#include <cstdint>

struct GPUMemoryHandle  {
    uint32_t count;         // index count
    uint32_t instanceCount; // usually 1
    uint32_t indexOffset;    // offset in IBO
    uint32_t vboOffset;    // offset in VBO
    uint32_t baseInstance;  // usually 0
    uint32_t ssboIndex;   // index in SSBO
};