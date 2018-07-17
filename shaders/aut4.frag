#version 330 core

uniform sampler2D grid;

in vec2 pos;

out vec4 frag_color;

void main(void) {
  float val = texture(grid, pos).r;
  frag_color = vec4(val, val, val, 1.0);
}
