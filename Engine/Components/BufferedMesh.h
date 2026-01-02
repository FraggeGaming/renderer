#include "../Renderer/RenderTypes.h"
#pragma once

struct BufferedMesh{
    bool isLoaded = false;
    bool isDirty = false;
    int meshID;
};
