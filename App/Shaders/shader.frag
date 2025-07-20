#version 460 core

uniform float Width;
uniform float Height;
uniform float Time;
out vec4 FragColor;

void main()
{
    vec2 outCol = gl_FragCoord.xy;
    FragColor = vec4(abs(sin(Time + outCol.x/Width)), abs(cos(Time + outCol.y/Height)), 0.0f, 1.0f); // Orange color
}
