#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform vec2 positions[10];
uniform float radii[10];
uniform float softnesses[10];
uniform vec3 colors[10];
uniform int numLights;
uniform float aspectRatio;

void main()
{
    vec3 col = texture(screenTexture, TexCoords).rgb;
    vec3 finalColor = vec3(0.25, 0.25, 0.25) * col;

    for (int i = 0; i < numLights; ++i) {
        vec2 adjustedCoords = vec2(TexCoords.x, TexCoords.y * aspectRatio);
        vec2 adjustedPosition = vec2(positions[i].x, positions[i].y * aspectRatio);
        float distanceToCenter = length(adjustedCoords - adjustedPosition);
        float alpha = 1.0 - smoothstep(radii[i], radii[i] + softnesses[i], distanceToCenter);
        finalColor = mix(finalColor, col * colors[i], alpha);
    }

    FragColor = vec4(finalColor, 1.0);
}