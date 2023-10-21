#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;

// Passed to fragment shader
out vec2 texcoord;

// Application data
uniform mat3 transform;
uniform mat3 projection;
uniform vec2 currentFrame;
uniform vec2 spritesheetSize;

void main()
{
	if (spritesheetSize.x != 1.0 || spritesheetSize.y != 1.0) {
        vec2 frameSize = 1.0 / spritesheetSize;
        vec2 frameStart = currentFrame * frameSize;
        texcoord = frameStart + in_texcoord * frameSize;
    } else {
        texcoord = in_texcoord;
    }

	vec3 pos = projection * transform * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}