#version 330 core

uniform usampler2D grid;
uniform uint no_states;
uniform int colorscheme;

in vec2 pos;

out vec4 frag_color;

void main(void) {
  if(no_states < 2u) {
    frag_color = vec4(1.0, 0.0, 0.0, 1.0);
  } else {
    uint state = texture(grid, pos).r;
    if(no_states == 2u) {
      float val = float(state);
      frag_color = vec4(val, val, val, 1.0);
    } else if(no_states <= 7u && colorscheme == 0) {
      if(state == 0u) {
        frag_color = vec4(0.0, 0.0, 0.0, 1.0);
      } else if(state == no_states - 1u) {
        frag_color = vec4(1.0, 1.0, 1.0, 1.0);
      } else if(state == 1u) {
        frag_color = vec4(0.0, 0.7, 0.7, 1.0);
      } else if(state == 2u) {
        frag_color = vec4(0.7, 0.7, 0.0, 1.0);
      } else if(state == 3u) {
        frag_color = vec4(0.7, 0.0, 0.7, 1.0);
      } else if(state == 4u) {
        frag_color = vec4(0.0, 0.0, 1.0, 1.0);
      } else if(state == 5u) {
        frag_color = vec4(0.0, 1.0, 0.0, 1.0);
      } else if(state == 6u) {
        frag_color = vec4(1.0, 0.0, 0.0, 1.0);
      }
    } else {
      float val = float(state) / float(no_states - 1u);
      frag_color = vec4(val, val, val, 1.0);
    }
  }
}

