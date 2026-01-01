#include "../Renderer/RenderTypes.h"
#pragma once

struct BufferedMesh{
    
    Mesh MeshComponent;
    bool isLoaded = false;
    bool isDirty = false;
};