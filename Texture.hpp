#pragma once

#include <incgraphics.h>

#include <Logger.hpp>
#include <Debug.hpp>

namespace gl {

template <GLenum TextureType>
struct Texture {
  GLuint tex;


  GLuint id() const { return tex; }
  static void init(Texture<TextureType> &tx) {
    tx.init();
  }

  static void init(GLuint &tex) {
    glGenTextures(1, &tex); GLERROR
  }

  void init() {
    gl::Texture<TextureType>::init(tex);
  }

  static void bind(Texture<TextureType> &tx) {
    tx.bind();
  }

  static void bind(GLuint tex) {
    glBindTexture(TextureType, tex); GLERROR
  }

  void bind() {
    gl::Texture<TextureType>::bind(tex);
  }

  template <typename T>
  static void param(GLenum pname, T value) {
    if constexpr(std::is_integral<T>::value) {
      glTexParameteri(TextureType, pname, value); GLERROR
    } else if constexpr(std::is_floating_point<T>::value) {
      glTexParameterf(TextureType, pname, value); GLERROR
    } else {
      throw std::runtime_error("failed to identify the type");
    }
  }

  static void set_active(int ind) {
    glActiveTexture(GL_TEXTURE0 + ind); GLERROR
  }

  static GLuint get_active() {
    GLint active_tex;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &active_tex); GLERROR
    return active_tex;
  }

  static void unbind() {
    glBindTexture(TextureType, 0); GLERROR
  }

  static void clear(gl::Texture<TextureType> &tx) {
    tx.clear();
  }

  static void clear(GLuint &tex) {
    glDeleteTextures(1, &tex); GLERROR
  }

  void clear() {
    Texture<TextureType>::clear(tex);
  }
};
} // namespace gl
