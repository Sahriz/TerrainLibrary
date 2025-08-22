#version 430 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform float Width;
uniform float Height;
uniform float Time;
uniform sampler2D uTexture;

out vec4 FragColor;

void main()
{

    vec3 norm = normalize(Normal);
    vec3 normalColor = abs(norm);

    FragColor = texture(uTexture, TexCoord);
}
