#version 430 core

in vec3 FragPos;
in vec3 Normal;

uniform float Width;
uniform float Height;
uniform float Time;

out vec4 FragColor;



void main()
{
    float ambientStrength = 0.01f;
    vec3 ambientColor = vec3(1.0f, 1.0f, 1.0f); // White color
    vec3 ambient = ambientStrength * ambientColor;

    float diffuseStrength = 0.8f;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(vec3(0.0f, -1, 0.0f)); // Light coming from above
    float diff = max(dot(norm, -lightDir), 0.0f);
    vec3 diffuse = diff * ambientColor * diffuseStrength;

    vec3 result = ambient + diffuse;
    FragColor = vec4(result, 1.0f);
}
