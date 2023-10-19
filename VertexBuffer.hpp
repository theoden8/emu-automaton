#pragma once

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <incgraphics.h>

#include <Debug.hpp>
#include <Logger.hpp>


namespace gl {

enum class BufferElementType {
  INTEGER, FLOAT,
  VEC2, VEC3, VEC4,
  IVEC2, IVEC3, IVEC4,
  MAT2, MAT3, MAT4
};

#define using_gltype(gltypeval) static constexpr GLenum gltype = gltypeval
template <BufferElementType A> struct a_cast_type { using type = void; };
template <> struct a_cast_type <BufferElementType::INTEGER> { using type = GLint;    using vtype = GLint;     using_gltype(GL_INT       ); };
template <> struct a_cast_type <BufferElementType::FLOAT> { using type = GLfloat;    using vtype  = GLfloat;  using_gltype(GL_FLOAT     ); };
template <> struct a_cast_type <BufferElementType::VEC2> { using type = glm::vec2;   using vtype  = GLfloat;  using_gltype(GL_FLOAT_VEC2); };
template <> struct a_cast_type <BufferElementType::VEC3> { using type = glm::vec3;   using vtype  = GLfloat;  using_gltype(GL_FLOAT_VEC3); };
template <> struct a_cast_type <BufferElementType::VEC4> { using type = glm::vec4;   using vtype  = GLfloat;  using_gltype(GL_FLOAT_VEC4); };
template <> struct a_cast_type <BufferElementType::IVEC2> { using type = glm::ivec2; using vtype  = GLint;    using_gltype(GL_INT_VEC2  ); };
template <> struct a_cast_type <BufferElementType::IVEC3> { using type = glm::ivec3; using vtype  = GLint;    using_gltype(GL_INT_VEC3  ); };
template <> struct a_cast_type <BufferElementType::IVEC4> { using type = glm::ivec4; using vtype  = GLint;    using_gltype(GL_INT_VEC4  ); };
template <> struct a_cast_type <BufferElementType::MAT2> { using type = glm::mat2;   using vtype  = GLfloat;  using_gltype(GL_FLOAT_MAT2); };
template <> struct a_cast_type <BufferElementType::MAT3> { using type = glm::mat3;   using vtype  = GLfloat;  using_gltype(GL_FLOAT_MAT3); };
template <> struct a_cast_type <BufferElementType::MAT4> { using type = glm::mat4;   using vtype  = GLfloat;  using_gltype(GL_FLOAT_MAT4); };
#undef using_gltype

template <typename T> struct gl_scalar_enum { static constexpr GLenum value = 0; };
template <> struct gl_scalar_enum<GLint> { static constexpr GLenum value = GL_INT; };
template <> struct gl_scalar_enum<GLfloat> { static constexpr GLenum value = GL_FLOAT; };

template <typename T>
struct BufferAccessor {

  T &buffer;
  size_t i;

  explicit BufferAccessor(T &buffer, size_t i):
    buffer(buffer), i(i)
  {}

  using atype = a_cast_type<T::element_type>;
  BufferAccessor operator=(typename atype::type vec) {
    T::bind(buffer);
    glBufferSubData(
      T::array_type,
      i*sizeof(typename atype::type),
      sizeof(typename atype::vtype),
      glm::value_ptr(vec)
    ); GLERROR
    T::unbind();
  }
};

template <GLenum BufferT, BufferElementType ElementT>
struct Buffer {
  using self_t = gl::Buffer<BufferT, ElementT>;

  static constexpr GLenum array_type = BufferT;
  static constexpr BufferElementType element_type = ElementT;

  GLuint vbo = 0;

  size_t numberOfElements = 0;
  size_t numberOfScalars = 0;
  size_t numberOfScalarsPerElement = 0;
  size_t sizeOfBuffer = 0;
  GLenum drawMode = GL_STATIC_DRAW;

  Buffer()
  {}

  static void init(GLuint &vbo) {
    glGenBuffers(1, &vbo); GLERROR
  }

  static void init(self_t &buf) {
    buf.init();
  }

  void init() {
    self_t::init(this->vbo);
  }

  GLuint id() const {
    return vbo;
  }

  static void bind(GLuint vbo) {
    glBindBuffer(BufferT, vbo); GLERROR
  }

  static void bind(const self_t &buf) {
    buf.bind();
  }

  void bind() const {
    self_t::bind(vbo);
  }

  template <GLenum DRAW_MODE, typename T>
  void allocate(const std::vector<T> &host_data, size_t start = 0, size_t count = SIZE_MAX) {
    if(count == SIZE_MAX) {
      count = host_data.size() - start;
    }
    constexpr size_t sizeOfScalar = sizeof(typename a_cast_type<ElementT>::vtype);
    static_assert(sizeOfScalar == sizeof(T));
    constexpr size_t sizeOfElement = sizeof(typename a_cast_type<ElementT>::type);
    static_assert(sizeOfElement / sizeOfScalar * sizeOfScalar == sizeOfElement);
    constexpr size_t element_size = sizeOfElement / sizeOfScalar;
    numberOfScalars = count;
    numberOfElements = numberOfScalars / element_size;
    numberOfScalarsPerElement = element_size;
    sizeOfBuffer = numberOfScalars * sizeOfScalar;
    ASSERT(numberOfElements * numberOfScalarsPerElement == numberOfScalars);

    this->bind();
    glBufferData(BufferT, sizeOfBuffer, &host_data[start], DRAW_MODE); GLERROR
    Logger::Debug("[buf: %lu] allocated buffer data: bytes=%lu, no_scalars=%lu, no_elements=%lu, no_scalars_per_element=%lu\n",
                  vbo, sizeOfBuffer, numberOfScalars, numberOfElements, numberOfScalarsPerElement);
    this->unbind();

    drawMode = DRAW_MODE;
  }

  template <GLenum DRAW_MODE, typename T>
  void allocate(std::vector<T> &&host_data, size_t start = 0, size_t count = SIZE_MAX) {
    allocate<DRAW_MODE>(host_data, start, count);
  }

  decltype(auto) operator[](size_t i) {
    ASSERT(drawMode != GL_STATIC_DRAW);

    return gl::BufferAccessor<self_t>(*this, i);
  }

  template <typename T>
  void set_subdata(std::vector<T> &&host_data, size_t start=0, size_t count=SIZE_MAX) {
    set_subdata(host_data, start, count);
  }

  template <typename T>
  void set_subdata(const std::vector<T> &host_data, size_t start=0, size_t count=SIZE_MAX) {
    ASSERT(drawMode != GL_STATIC_DRAW);

    if(count == SIZE_MAX) {
      count = host_data.size() - start;
    }
    constexpr size_t sizeOfScalar = sizeof(typename a_cast_type<ElementT>::vtype);
    static_assert(sizeOfScalar == sizeof(T));
    constexpr size_t sizeOfElement = sizeof(typename a_cast_type<ElementT>::type);
    static_assert(sizeOfElement / sizeOfScalar * sizeOfScalar == sizeOfElement);
    constexpr size_t element_size = sizeOfElement / sizeOfScalar;

    this->bind();
    glBufferSubData(BufferT, start * sizeof(sizeOfScalar), sizeof(T) * count, &host_data[start]); GLERROR
    Logger::Debug("[buf: %lu] set buffer subdata: bytes=%lu, no_elems=%lu, no_elements=%lu, no_elems_per_element\n",
                  vbo, sizeOfBuffer, numberOfScalars, numberOfElements, numberOfScalarsPerElement);
    this->unbind();
  }

  static void unbind() {
    glBindBuffer(BufferT, 0); GLERROR
  }

  static void clear(GLuint &vbo) {
    glDeleteBuffers(1, &vbo); GLERROR
  }

  static void clear(self_t &buf) {
    buf.clear();
  }

  void clear() {
    clear(vbo);
  }
};

} // namespace gl
