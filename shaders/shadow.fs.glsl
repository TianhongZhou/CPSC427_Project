#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D spriteTexture;
uniform vec4 shadowColor;
uniform int isShadow;

void main()
{
    vec4 mainColor = texture(spriteTexture, TexCoords);

    if (isShadow == 1)
    {
        FragColor = shadowColor * mainColor.a;
    }
    else
    {
        FragColor = mainColor;
    }
}