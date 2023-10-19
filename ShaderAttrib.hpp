#pragma once

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <incgraphics.h>

#include <Debug.hpp>
#include <Logger.hpp>

#include <VertexBuffer.hpp>

namespace gl {

template <typename VertexBuffer> struct Attrib;

template <GLenum BufferT, BufferElementType ElementT>
struct Attrib <gl::Buffer<BufferT, ElementT>> {
  using VertexBuffer = gl::Buffer<BufferT, ElementT>;
  using self_t = Attrib<VertexBuffer>;

  static constexpr GLenum array_type = VertexBuffer::array_type;
  static constexpr BufferElementType element_type = VertexBuffer::element_type;

  std::string location = "";
  VertexBuffer *buf = nullptr;

  Attrib(){}
  Attrib(std::string loc):
    location(loc)
  {}

  Attrib(std::string loc, VertexBuffer &shaderbuffer):
    location(loc)
  {
    select_buffer(shaderbuffer);
  }

  bool operator==(const self_t &other) const {
    return location == other.location;
  }

  GLuint id() const {
    ASSERT(buf != nullptr);
    return buf->id();
  }

  void select_buffer(VertexBuffer &sel_buf) {
    buf = &sel_buf;
  }

  GLuint loc(GLuint program_id) const {
    ASSERT(location != "");
    GLuint lc = glGetAttribLocation(program_id, location.c_str()); GLERROR
    return lc;
  }

  static void bind(const self_t &attr) {
    attr.bind();
  }

  void bind()const {
    ASSERT(buf != nullptr);
    VertexBuffer::bind(*buf);
  }

  bool is_active(GLuint program_id) {
    ASSERT(buf != nullptr);
    char name[81];
    GLsizei length;
    GLint size;
    GLenum t;
    glGetActiveAttrib(program_id, buf->vbo, 80, &length, &size, &t, name); GLERROR
    return t == a_cast_type<ElementT>::gltype && location == name;
  }

  decltype(auto) operator[](size_t i) {
    ASSERT(buf != nullptr);
    return (*buf)[i];
  }

  void deselect_buffer() {
    buf = nullptr;
  }

  static void unbind() {
    glBindBuffer(BufferT, 0); GLERROR
  }

  static void clear(self_t &attr) {
    attr.clear();
  }

  void clear() {
    if(buf != nullptr) {
      deselect_buffer();
    }
  }
};

} // namespace gl
