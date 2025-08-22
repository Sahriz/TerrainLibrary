#version 430 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 projM;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat3 normalMatrix;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;


void main()
{
    FragPos = vec3(uModel*vec4(aPos, 1.0f));
    Normal = normalMatrix*aNormal;
    gl_Position =  projM * uView * uModel * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}