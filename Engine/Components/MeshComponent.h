#include "../Renderer/RenderTypes.h"
#pragma once

struct MeshComponent{
    
    Mesh MeshComponent;
    bool isDirty = false;
};