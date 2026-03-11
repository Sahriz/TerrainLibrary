#version 430 core

in vec3 FragPos;
in vec3 Normal;

uniform float Width;
uniform float Height;
uniform float Time;
uniform vec3 playerPos;

out vec4 FragColor;

void main()
{
    //Directional lighting
    vec3 baseColor = vec3(1.0, 1.0, 1.0);
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(playerPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    float ambient = 0.15;
    vec3 litColor = baseColor * (diff + ambient);
    
    //Distance-based visibility
    float dist = distance(FragPos, playerPos);
    float maxDistance = 32.0;
    float visibility = 1.0 - clamp(dist / maxDistance, 0.0, 1.0);
    visibility = pow(visibility, 1.0);
    
    vec3 finalColor = litColor * visibility;

    FragColor = vec4(finalColor, 1.0);
}
