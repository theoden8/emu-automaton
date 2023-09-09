#version 330 core

uniform sampler2D grid;
uniform int no_states;

in vec2 pos;

out vec4 frag_color;

void main(void) {
  if(no_states < 2) {
    frag_color = vec4(1.0, 0.0, 0.0, 1.0);
  } else {
    // 256 / 2 = 128
    // 0, 0.4999, 1.0
    int state = int(texture(grid, pos).r * (no_states - 1) + 0.5);
    if(no_states == 2) {
      float val = float(state);
      frag_color = vec4(val, val, val, 1.0);
    } else {
      if(state == 0) {
        frag_color = vec4(0.0, 0.0, 0.0, 1.0);
      } else if(state == no_states - 1) {
        frag_color = vec4(1.0, 1.0, 1.0, 1.0);
      } else if(state == 1) {
        frag_color = vec4(0.0, 0.7, 0.7, 1.0);
      } else if(state == 2) {
        frag_color = vec4(0.7, 0.7, 0.0, 1.0);
      } else if(state == 3) {
        frag_color = vec4(0.7, 0.0, 0.7, 1.0);
      } else if(state == 4) {
        frag_color = vec4(0.0, 0.0, 1.0, 1.0);
      } else if(state == 5) {
        frag_color = vec4(0.0, 1.0, 0.0, 1.0);
      } else if(state == 6) {
        frag_color = vec4(1.0, 0.0, 0.0, 1.0);
      } else {
        frag_color = vec4(0.5, 0.5, 0.5, 1.0);
      }
    }
  }
}

