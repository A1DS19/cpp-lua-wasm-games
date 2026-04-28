#version 330 core
in vec2 v_uv;
out vec4 frag_color;

uniform sampler2D base_color_texture;

void main() {
    vec4 texture_color = texture(base_color_texture, v_uv);
    frag_color = texture_color;
}
