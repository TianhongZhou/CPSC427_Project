#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform int enter_combat;
uniform int xFlip;
uniform vec2 spritesheetSize;
uniform sampler2D shadowMap;
uniform mat4 lightSpaceMatrix;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
    vec2 uv = texcoord;

    if (xFlip == 1) {
        float frameWidth = 1.0 / spritesheetSize.x;
        float frameOffset = floor(uv.x / frameWidth) * frameWidth;
        uv.x = frameOffset + frameWidth - (uv.x - frameOffset);
    }

    color = vec4(fcolor, 1.0) * texture(sampler0, uv);
    if (enter_combat == 1) {
        color = vec4(1.0, 0.0, 0.0, 1.0) * texture(sampler0, uv);
    }
}
