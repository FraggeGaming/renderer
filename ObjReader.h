#include <fstream>
#include <unordered_map>

#include "Engine/Renderer/RenderTypes.h"

struct ObjFace {
    unsigned int v_idx = 0;
    unsigned int vt_idx = 0;
    unsigned int vn_idx = 0;

    bool operator == (const ObjFace& face) const noexcept {
        return v_idx == face.v_idx && vt_idx == face.vt_idx && vn_idx == face.vn_idx;
    }

    Vertex GenerateVertex(
        std::vector<glm::vec3>& o_vertex, 
        std::vector<glm::vec3>& o_vnormal,
        std::vector<glm::vec2>& o_texcord){
        
            Vertex v;

            //handle negative index
            if(v_idx < 0) {
                v_idx = o_vertex.size() + v_idx -1;
            }

            if(vt_idx < 0) {
                vt_idx = o_vertex.size() + vt_idx -1;
            }

            if(vn_idx < 0) {
                vn_idx = o_vertex.size() + vn_idx -1;
            }


            //Fetch data
            if(v_idx > 0){
                v.pos = o_vertex[v_idx-1];
            }

            if(vt_idx > 0){
                v.texCord = o_texcord[vt_idx-1];
            }

            if(vn_idx > 0){
                v.normal = o_vnormal[vn_idx-1];
            }

            v.color = glm::vec4(1.0f);

            return v;
    }
};

namespace std {
    template<>
    struct hash<ObjFace> {
        std::size_t operator()(const ObjFace& face) const noexcept {
            std::size_t v = std::hash<unsigned int>{}(face.v_idx);
            std::size_t vt = std::hash<unsigned int>{}(face.vt_idx);
            std::size_t vn = std::hash<unsigned int>{}(face.vn_idx);

            return v^(vt << 1)^(vn << 2);
        }
    };
}


struct ObjMaterial {
    glm::vec4 color;
    bool hasColor;
};

class ObjReader {
private:
    bool m_ObjParam[4];
public:
    //Should return a mesh 
    enum ObjData{
        VERTEX, TEXCORD, NORMAL, COLOR
    };

    ObjReader();

    ~ObjReader();

    void SetReadParam(ObjData param, bool value);

    void NormalizeMesh(Mesh& mesh);
    std::vector<ObjFace> ReadFaceData(std::string& line);

    Mesh ReadObject(const std::string& path);

    std::unordered_map<std::string, ObjMaterial> ReadMaterial(const std::string& path);
};