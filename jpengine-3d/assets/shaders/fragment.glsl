#version 330 core

struct Light {
    vec3 color;
    vec3 position;
};

uniform Light u_light;

in vec2 v_uv;
in vec3 v_normal;
in vec3 v_frag_pos;

out vec4 frag_color;

uniform sampler2D base_color_texture;

void main() {
    // If the mesh didn't supply normals (attribute 3 unbound → v_normal is zero),
    // fall back to "fully lit" so the texture still shows through. Otherwise
    // do per-pixel Lambert diffuse against the light position.
    vec3 raw_normal = v_normal;
    float n_len = length(raw_normal);

    vec3 diffuse_comp;
    if (n_len < 0.001) {
        diffuse_comp = u_light.color;
    } else {
        vec3 normal = raw_normal / n_len;
        vec3 light_direction = normalize(u_light.position - v_frag_pos);
        float diffuse = max(dot(normal, light_direction), 0.0);
        diffuse_comp = diffuse * u_light.color;
    }

    vec4 texture_color = texture(base_color_texture, v_uv);
    frag_color = texture_color * vec4(diffuse_comp, 1.0);
}
