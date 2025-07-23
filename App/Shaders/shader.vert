#version 460 core

layout(location = 0) in vec3 aPos;

uniform mat4 projM;
uniform mat4 uModel;
uniform mat4 uView;

out vec4 position;


void main()
{
    mat4 hardcoded = mat4(
        vec4(1.73205, 0, 0, 0),
        vec4(0, 1.73205, 0, 0),
        vec4(0, 0, -1.002, -1),
        vec4(0, 0, -0.2002, 0)
    );
    position = vec4(aPos, 1.0);
    gl_Position =  projM * uView * uModel * vec4(aPos, 1.0);
}