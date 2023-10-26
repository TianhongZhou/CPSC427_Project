#version 330 core

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 projection;
uniform vec2 shadowOffset;

void main()
{
    gl_Position = projection * model * vec4(inPos.x + shadowOffset.x, inPos.y + shadowOffset.y, 0.0, 1.0);
    TexCoords = inTexCoords;
}