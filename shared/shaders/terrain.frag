#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 VertexColor;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform float time;

void main() {
    // Enhanced terrain lighting
    vec3 result = VertexColor;
    
    // Dynamic lighting based on position
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    
    // Add some variation based on height
    float heightFactor = (FragPos.y + 10.0) / 20.0; // Assuming height range -10 to 10
    vec3 terrainColor = mix(
        vec3(0.2, 0.4, 0.1), // Dark green (valleys)
        vec3(0.9, 0.9, 0.8), // White (peaks)
        clamp(heightFactor, 0.0, 1.0)
    );
    
    // Combine vertex color with terrain-based color
    result = mix(VertexColor, terrainColor, 0.6);
    
    // Apply lighting
    vec3 ambient = 0.3 * lightColor;
    vec3 diffuse = diff * lightColor;
    vec3 finalColor = (ambient + diffuse) * result;
    
    FragColor = vec4(finalColor, 1.0);
}