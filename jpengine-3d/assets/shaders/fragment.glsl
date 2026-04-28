#version 330 core
in vec3 v_color;
in vec2 v_uv;
out vec4 frag_color;

uniform sampler2D brick_texture;

void main() {
    vec4 texture_color = texture(brick_texture, v_uv);
    frag_color = texture_color * vec4(v_color, 1.0);
}
