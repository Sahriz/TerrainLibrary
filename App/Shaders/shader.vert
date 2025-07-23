#version 430 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 projM;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat3 normalMatrix;

out vec3 FragPos;
out vec3 Normal;


void main()
{
    FragPos = vec3(uModel*vec4(aPos, 1.0f));
    Normal = mat3(transpose(inverse(uModel)))*normalize(aNormal);
    gl_Position =  projM * uView * uModel * vec4(aPos, 1.0);
}