#include "../Renderer/RenderTypes.h"
#pragma once

struct BufferedMesh{
    int id;
    Mesh MeshComponent;
    bool isLoaded = false;
    bool isDirty = false;
};
