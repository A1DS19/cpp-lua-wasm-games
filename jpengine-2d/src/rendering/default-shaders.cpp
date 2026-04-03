#include "rendering/default-shaders.hpp"

using namespace jpengine;

const char* DefaultShaders::basic_shader_vert = R"(#version 300 es
in vec2 a_pos;
in vec2 a_uvs;
in vec4 a_color;

out vec2 frag_uvs;
out vec4 frag_color;

uniform mat4 u_projection;

void main(){
  gl_Position = u_projection * vec4(a_pos, 0.0f, 1.0f);
  frag_uvs = a_uvs;
  frag_color = a_color;
}
)";

const char* DefaultShaders::basic_shader_frag = R"(#version 300 es
precision mediump float;

in vec2 frag_uvs;
in vec4 frag_color;

out vec4 color;
uniform sampler2D u_texture;

void main(){
  color = texture(u_texture, frag_uvs) * frag_color;
}
)";

const char* DefaultShaders::font_shader_vert = R"(#version 300 es
in vec2 a_pos;
in vec2 a_uvs;
in vec4 a_color;

out vec2 frag_uvs;
out vec4 frag_color;

uniform mat4 u_projection;

void main(){
  gl_Position = u_projection * vec4(a_pos, 0.0f, 1.0f);
  frag_uvs = a_uvs;
  frag_color = a_color;
}
)";

const char* DefaultShaders::font_shader_frag = R"(#version 300 es
precision mediump float;

in vec2 frag_uvs;
in vec4 frag_color;

out vec4 color;
uniform sampler2D u_font_atlas;

void main(){
  vec4 font_color = vec4(1,1,1, texture(u_font_atlas, frag_uvs).a);
  color = font_color * frag_color;
}
)";
