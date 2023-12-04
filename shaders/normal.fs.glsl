#version 330

in vec2 texcoord;
in vec3 vcs_pos;

uniform sampler2D sampler0;
uniform sampler2D normal_map;
uniform vec3 light_pos;
uniform vec3 light_color;

out vec4 color;

void main() {
    vec2 uv = texcoord;

    vec3 tex_color = texture(sampler0, uv).xyz;
    vec3 normalTex = texture(normal_map, uv).xyz * 2.0 - 1.0;

    vec3 N = vec3(normalTex.x, normalTex.z, normalTex.y);
    vec3 L = 2.0 * light_pos - 1.0 - vcs_pos;
    N = normalize(N);
    L = normalize(L);
    float i = dot(N, L);

    color = vec4(tex_color * i, 1.0);
}