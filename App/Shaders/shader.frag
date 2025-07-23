#version 460 core

in vec4 position;

uniform float Width;
uniform float Height;
uniform float Time;
out vec4 FragColor;

void main()
{
    vec2 outCol = gl_FragCoord.xy;
    FragColor = vec4(vec3(5.0f * (position.y + 0.1f)), 1.0f); // Orange color
}
