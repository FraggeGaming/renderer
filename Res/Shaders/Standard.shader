#shader vertex
#version 460 core

layout(location = 0)in vec4 a_position;
layout(location = 1)in vec2 a_texCord;
layout(location = 2)in vec3 a_normal;
layout(location = 3)in vec4 a_Color;

layout(std430, binding = 3) buffer TransformSSBO
{
    mat4 ssbo_Models[];
};


//uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;



out vec4 v_Color;
out vec3 FragPos;
out vec3 Normal;

void main()
{
    v_Color = a_Color; // to visualize normals, vec4(a_normal * 0.5 + 0.5, 0.5)
    vec4 worldPos = ssbo_Models[gl_DrawID] * vec4(a_position.xyz, 1.0);
    gl_Position = projection * view * worldPos;
    FragPos = worldPos.xyz; 
   
    //mat4 normalMatrix4 = transpose(inverse(model));
    
   
   //mat3 normalMatrix3 = mat3(normalMatrix4);
    
    
    Normal = mat3(transpose(inverse(ssbo_Models[gl_DrawID]))) * a_normal;  

}


#shader fragment
#version 460 core

in vec4 v_Color;
in vec3 FragPos; 
in vec3 Normal;

uniform float ambient;
uniform vec3 lightPos;

layout(location = 0) out vec4 fColor;



void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0); 
    vec3 diffuse = diff * v_Color.rgb; 

    vec3 result = v_Color.rgb * ambient + diffuse;

    fColor = vec4(result, 1.0);
}