#version 330 core

layout (location = 0) in vec2 vertex;

out vec2 pos;

void main(void) {
  pos = vertex;
  gl_Position = vec4(2 * (pos - .5), 0, 1);
}
