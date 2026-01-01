#include "ObjReader.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <cfloat>

#define DEBUG_OBJ 1

#if DEBUG_OBJ
    #define DBG(x) std::cout << x << std::endl
#else
    #define DBG(x)
#endif



ObjReader::ObjReader()
{
}


ObjReader::~ObjReader()
{
}

void ObjReader::SetReadParam(ObjData param, bool value)
{
    m_ObjParam[(int)param] = value;

}
std::unordered_map<std::string, ObjMaterial> ObjReader::ReadMaterial(const std::string &path)
{
    std::unordered_map<std::string, ObjMaterial> objMaterials;

    if(path.substr(path.find_last_of(".") + 1) != "mtl"){
        std::cout << "ERROR: UNSUPPORTED FILE FORMAT. ONLY ACCEPTS .mtl";
        return objMaterials;
    }

    std::ifstream stream(path);

    if (!stream.is_open()) {
        std::cout << "FAILED TO OPEN FILE: " << path << std::endl;
        return objMaterials;
    }

    std::string line;
    std::string currentMaterial;

    std::cout << "reading object: " << path << std::endl;

    while (getline(stream, line)){

        if(line.empty() || line.size() < 2)
            continue;

        if (line.rfind("newmtl", 0) == 0) {
            
            std::stringstream ss(line);

            std::string newmtl;

            ss >> newmtl >> currentMaterial;

            std::cout << currentMaterial << std::endl;

            ObjMaterial mat = {};
            objMaterials[currentMaterial] = mat;
            std::cout << "Current material: " << currentMaterial << std::endl;
        }


        if(line.rfind("Kd", 0) == 0)
        {
           if(!currentMaterial.empty()){
                std::stringstream ss(line.substr(3));
                float x, y, z;

                ss >> x >> y >> z;
                objMaterials[currentMaterial].hasColor = true;
                objMaterials[currentMaterial].color = {x, y, z, 1.0f};  
           }
        }

        //TODO implement ka, ks etc

        else if (line[0] == '#'){
            std::cout << line << std::endl;

        }

    }
    
    return objMaterials;
}

Mesh ObjReader::ReadObject(const std::string &path)
{

    if(path.substr(path.find_last_of(".") + 1) != "obj"){
        std::cout << "ERROR: UNSUPPORTED FILE FORMAT. ONLY ACCEPTS .OBJ";
        return Mesh();
    }

    
    //open the material file if exist
    std::string mtlPath = path.substr(0, path.find_last_of('.')) + ".mtl";
    

    std::unordered_map<std::string, ObjMaterial> objMaterials = ReadMaterial(mtlPath);

    std::ifstream stream(path);

    if (!stream.is_open()) {
        std::cout << "FAILED TO OPEN FILE: " << path << std::endl;
    }

    std::string line;

    std::vector<glm::vec3> o_vertex;
    std::vector<glm::vec3> o_vnormal;
    std::vector<glm::vec2> o_texcord;

    std::cout << "reading object: " << path << std::endl;
    Mesh mesh;
    std::unordered_map<ObjFace, uint32_t> FaceIndexMap;
    bool hasNormal = false;
    std::string currentMaterial;


    while (getline(stream, line)){

        if(line.empty() || line.size() < 2)
            continue;


        if(line[0] == 'o')
        {
            std::cout << line.substr(2) << path << std::endl;
        }

        if(line[0] == 'v')
        {
            if (line[1] == ' ' && m_ObjParam[ObjData::VERTEX])
            {
                std::stringstream ss(line.substr(2));
                float x, y, z;
                ss >> x >> y >> z;

                o_vertex.push_back({x, y, z});
                //Read Vertex data
            }

            else if (line[1] == 'n' && m_ObjParam[ObjData::NORMAL])
            {
                hasNormal = true;
                //Read Vertex normal
                std::stringstream ss(line.substr(2));
                float x, y, z;
                ss >> x >> y >> z;

                o_vnormal.push_back({x, y, z});
            }

            else if (line[1] == 't' && m_ObjParam[ObjData::TEXCORD])
            {
                //Read text cord
                std::stringstream ss(line.substr(2));
                float x, y;
                ss >> x >> y;

                o_texcord.push_back({x, y});
            }
        }

        else if (line[0] == 'f') 
        {
            std::vector<uint32_t> faceIndices;


            std::vector<ObjFace> f = ReadFaceData(line);
            for (size_t i = 0; i < f.size(); i++)
            {
                //If this face already exist, no need to dublicate it
                auto idx = FaceIndexMap.find(f[i]);
                if(idx != FaceIndexMap.end()){
                    faceIndices.push_back(idx->second); // Push the index to the idx buffer
                }
                else{
                    Vertex v = f[i].GenerateVertex(o_vertex, o_vnormal, o_texcord);
                    uint32_t n_idx = mesh.vertices.size();
                    mesh.vertices.push_back(v);


                    FaceIndexMap[f[i]] = n_idx;
                    faceIndices.push_back(n_idx);            
                }
            }
            
            //The provided basic triangulation and process the face
            for (size_t i = 1; i < faceIndices.size()-1; i++)
            {
                
                
                mesh.indices.push_back(faceIndices[0]);
                mesh.indices.push_back(faceIndices[i]);
                mesh.indices.push_back(faceIndices[i + 1]);

                //Recalculate the normal if not exist
                if(!hasNormal){

                    glm::vec3 A = mesh.vertices[faceIndices[0]].pos;
                    glm::vec3 B = mesh.vertices[faceIndices[i]].pos; 
                    glm::vec3 C = mesh.vertices[faceIndices[i+1]].pos;
                    // //Calculate the normal if none is given using right handed rule 
                    glm::vec3 n = glm::normalize(glm::cross(B-A, C-A)); 
               
                    mesh.vertices[faceIndices[0]].normal = n; 
                    mesh.vertices[faceIndices[i]].normal = n; 
                    mesh.vertices[faceIndices[i+1]].normal = n;
                }

                //Fetch material
                if(objMaterials.find(currentMaterial) != objMaterials.end() && objMaterials[currentMaterial].hasColor){
                    ObjMaterial mat = objMaterials[currentMaterial];
                    mesh.vertices[faceIndices[0]].color = mat.color; 
                    mesh.vertices[faceIndices[i]].color = mat.color; 
                    mesh.vertices[faceIndices[i+1]].color = mat.color;
                } else { //Just for fun, add the normal as color
                    //mesh.vertices[faceIndices[0]].color = glm::vec4(mesh.vertices[faceIndices[0]].normal, 1.0f); 
                    //mesh.vertices[faceIndices[i]].color = glm::vec4(mesh.vertices[faceIndices[i]].normal, 1.0f);  
                    //mesh.vertices[faceIndices[i+1]].color = glm::vec4(mesh.vertices[faceIndices[i+1]].normal, 1.0f); 
                }

            }        }  

        else if (line[0] == '#'){
            std::cout << line << std::endl;
            currentMaterial = "";

        }

        if (line.rfind("usemtl", 0) == 0) {
            std::stringstream ss(line);

            std::string usemtl;

            ss >> usemtl >> currentMaterial;
        }
    }


    
    std::cout << "Finished reading object" << std::endl;

    
    NormalizeMesh(mesh);
    
    return mesh;
}

void ObjReader::NormalizeMesh(Mesh& mesh){

    //Find max and min values for all vertex
    glm::vec3 min( FLT_MAX);
    glm::vec3 max(-FLT_MAX);
    for (size_t  i = 0; i < mesh.vertices.size(); i++)
    {
        min.x = std::min(min.x, mesh.vertices[i].pos.x);
        min.y = std::min(min.y, mesh.vertices[i].pos.y);
        min.z = std::min(min.z, mesh.vertices[i].pos.z);

        max.x = std::max(max.x, mesh.vertices[i].pos.x);
        max.y = std::max(max.y, mesh.vertices[i].pos.y);
        max.z = std::max(max.z, mesh.vertices[i].pos.z);
        
    }


    //Normalize the vertex
    for (size_t  i = 0; i < mesh.vertices.size(); i++)
    {
        mesh.vertices[i].pos = 2.0f*( (mesh.vertices[i].pos-min) / (max - min) ) -1.0f;
    }
  
}

std::vector<ObjFace> ObjReader::ReadFaceData(std::string& line)
{
    std::vector<ObjFace> faces;
    std::stringstream ss(line.substr(2));
    std::string ref;

    
    while (ss >> ref)
    {
        int part = 0;
        std::string temp;
        std::stringstream s2(ref);

        ObjFace face;

        //Bcs we need to support v/vt/vn and v//vt etc... so delim it
        while(getline(s2, temp,'/'))
        {
            if(!temp.empty())
            {
                if(part == 0) face.v_idx = std::stoi(temp);

                else if (part == 1) face.vt_idx = std::stoi(temp);

                else if (part == 2)face.vn_idx = std::stoi(temp);
            }

            part++;
        }

        faces.push_back(face);
 

    } 
    return faces;
}