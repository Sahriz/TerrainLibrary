#version 430 core

in vec3 FragPos;
in vec3 Normal;

uniform float Width;
uniform float Height;
uniform float Time;

out vec4 FragColor;

void main()
{

    vec3 norm = normalize(Normal);
    vec3 normalColor = abs(norm);

    FragColor = vec4(normalColor, 1.0f);
}
