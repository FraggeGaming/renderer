#include "../Renderer/RenderTypes.h"
#pragma once

enum MeshMode{
    STATIC, 
    DYNAMIC,
    CHUNKED
};
struct MeshCapsule{
    bool isLoaded = false;
    bool isDirty = false;
    int meshID;

    MeshMode mode = MeshMode::CHUNKED;
};
