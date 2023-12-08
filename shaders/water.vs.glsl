#version 330

in vec3 in_position;

out vec2 texcoord;

uniform float time;
uniform float factor;

void main()
{
    gl_Position = vec4(in_position.xy, 0.0, 1.0);
    if (factor<=0.25) texcoord = (in_position.xy + 1) / 2.f;
    else {
        vec3 finalPosition = in_position + vec3(sin(time*2)*0.04*(factor-0.25)*(in_position.y+1+2*(factor-0.25)), 0.0, 0.0);
        texcoord = (finalPosition.xy + 1) / 2.f;
    }
}
