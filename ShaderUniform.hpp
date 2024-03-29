#pragma once

#include <string>
#include <type_traits>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <incgraphics.h>
#include <Debug.hpp>
#include <Logger.hpp>

namespace gl {
enum class UniformType {
  INTEGER, UINTEGER, FLOAT,
  VEC2, VEC3, VEC4,
  IVEC2, IVEC3, IVEC4,
  UVEC2, UVEC3, UVEC4,
  MAT2, MAT3, MAT4,
  SAMPLER2D
};

#define using_sc static constexpr GLenum
template <UniformType U> struct u_cast_type { using type = void; };
template <> struct u_cast_type <UniformType::INTEGER> { using type = GLint; using_sc gltype = GL_INT; };
template <> struct u_cast_type <UniformType::UINTEGER> { using type = GLuint; using_sc gltype = GL_UNSIGNED_INT; };
template <> struct u_cast_type <UniformType::FLOAT> { using type = GLfloat; using_sc gltype = GL_FLOAT; };
template <> struct u_cast_type <UniformType::VEC2> { using type = glm::vec2; using_sc gltype = GL_FLOAT_VEC2; };
template <> struct u_cast_type <UniformType::VEC3> { using type = glm::vec3; using_sc gltype = GL_FLOAT_VEC3; };
template <> struct u_cast_type <UniformType::VEC4> { using type = glm::vec4; using_sc gltype = GL_FLOAT_VEC4; };
template <> struct u_cast_type <UniformType::IVEC2> { using type = glm::ivec2; using_sc gltype = GL_INT_VEC2; };
template <> struct u_cast_type <UniformType::IVEC3> { using type = glm::ivec3; using_sc gltype = GL_INT_VEC3; };
template <> struct u_cast_type <UniformType::IVEC4> { using type = glm::ivec4; using_sc gltype = GL_INT_VEC4; };
template <> struct u_cast_type <UniformType::UVEC2> { using type = glm::uvec2; using_sc gltype = GL_UNSIGNED_INT_VEC2; };
template <> struct u_cast_type <UniformType::UVEC3> { using type = glm::uvec3; using_sc gltype = GL_UNSIGNED_INT_VEC3; };
template <> struct u_cast_type <UniformType::UVEC4> { using type = glm::uvec4; using_sc gltype = GL_UNSIGNED_INT_VEC4; };
template <> struct u_cast_type <UniformType::MAT2> { using type = glm::mat2; using_sc gltype = GL_FLOAT_MAT2; };
template <> struct u_cast_type <UniformType::MAT3> { using type = glm::mat3; using_sc gltype = GL_FLOAT_MAT3; };
template <> struct u_cast_type <UniformType::MAT4> { using type = glm::mat4; using_sc gltype = GL_FLOAT_MAT4; };
template <> struct u_cast_type <UniformType::SAMPLER2D> { using type = GLuint; using_sc gltype = GL_SAMPLER_2D; };
#undef using_sc


template <UniformType U>
struct Uniform {
  using type = typename u_cast_type<U>::type;
  static constexpr GLenum gltype = u_cast_type<U>::gltype;
  using dtype = std::conditional_t<
    std::is_fundamental<type>::value,
      std::remove_reference_t<type>,
      std::add_const_t<std::add_lvalue_reference_t<type>
    >
  >;
  GLuint uniformId = 0;
  GLuint programId = 0;
  std::string location;
  explicit Uniform(std::string loc):
    location(loc)
  {}
  GLuint id() const {
    return uniformId;
  }
  GLuint loc() const {
    if(location == "") {
      TERMINATE("location is unset\n");
    }
    GLuint lc = glGetUniformLocation(programId, location.c_str()); GLERROR
    return lc;
  }
  void set_id(GLuint program_id) {
    if(programId != 0) {
      TERMINATE("program id already set");
    }
    programId = program_id;
    uniformId = loc();
  }
  bool is_active() {
    char name[81];
    GLsizei length;
    GLint size;
    GLenum t;
    glGetActiveUniform(programId, uniformId, 80, &length, &size, &t, name); GLERROR
    return t == gltype && location == name;
  }
  std::string str() const {
    char s[256];
    snprintf(s, 256, "[U[p=%u][u=%u][loc=%s]]", unsigned(programId), unsigned(uniformId), location.c_str());
    return s;
  }
  void unset_id() {
    programId = 0;
  }
  void set_data(dtype data);
  type get_data() const;
};

#define CHECK_PROGRAM_ID \
  if(programId == 0) { \
    TERMINATE("unable to set data to a uniform without program id set\n"); \
  }

template <>
void gl::Uniform<gl::UniformType::INTEGER>::set_data(Uniform<gl::UniformType::INTEGER>::dtype data) {
  CHECK_PROGRAM_ID;
  glUniform1i(uniformId, data); GLERROR
}

template <>
void gl::Uniform<gl::UniformType::UINTEGER>::set_data(Uniform<gl::UniformType::UINTEGER>::dtype data) {
  CHECK_PROGRAM_ID;
  glUniform1ui(uniformId, data); GLERROR
}

template <>
void gl::Uniform<gl::UniformType::FLOAT>::set_data(Uniform<gl::UniformType::FLOAT>::dtype data) {
  CHECK_PROGRAM_ID;
  glUniform1f(uniformId, data); GLERROR
}

template <>
void gl::Uniform<gl::UniformType::VEC2>::set_data(Uniform<gl::UniformType::VEC2>::dtype data) {
  CHECK_PROGRAM_ID;
  glUniform2f(uniformId, data.x, data.y); GLERROR
}

template <>
void gl::Uniform<gl::UniformType::VEC3>::set_data(Uniform<gl::UniformType::VEC3>::dtype data) {
  CHECK_PROGRAM_ID;
  glUniform3f(uniformId, data.x, data.y, data.z); GLERROR
}

template <>
void gl::Uniform<UniformType::VEC4>::set_data(Uniform<UniformType::VEC4>::dtype data) {
  CHECK_PROGRAM_ID;
  glUniform4f(uniformId, data.x, data.y, data.z, data.t); GLERROR
}

template <>
void gl::Uniform<gl::UniformType::IVEC2>::set_data(Uniform<gl::UniformType::IVEC2>::dtype data) {
  CHECK_PROGRAM_ID;
  glUniform2i(uniformId, data.x, data.y); GLERROR
}

template <>
void gl::Uniform<gl::UniformType::IVEC3>::set_data(Uniform<gl::UniformType::IVEC3>::dtype data) {
  CHECK_PROGRAM_ID;
  glUniform3i(uniformId, data.x, data.y, data.z); GLERROR
}

template <>
void gl::Uniform<UniformType::IVEC4>::set_data(Uniform<UniformType::IVEC4>::dtype data) {
  CHECK_PROGRAM_ID;
  glUniform4i(uniformId, data.x, data.y, data.z, data.t); GLERROR
}

template <>
void gl::Uniform<gl::UniformType::UVEC2>::set_data(Uniform<gl::UniformType::UVEC2>::dtype data) {
  CHECK_PROGRAM_ID;
  glUniform2ui(uniformId, data.x, data.y); GLERROR
}

template <>
void gl::Uniform<gl::UniformType::UVEC3>::set_data(Uniform<gl::UniformType::UVEC3>::dtype data) {
  CHECK_PROGRAM_ID;
  glUniform3ui(uniformId, data.x, data.y, data.z); GLERROR
}

template <>
void gl::Uniform<UniformType::UVEC4>::set_data(Uniform<UniformType::UVEC4>::dtype data) {
  CHECK_PROGRAM_ID;
  glUniform4ui(uniformId, data.x, data.y, data.z, data.t); GLERROR
}


template <>
void gl::Uniform<UniformType::MAT2>::set_data(Uniform<UniformType::MAT2>::dtype data) {
  CHECK_PROGRAM_ID;
  glUniformMatrix2fvARB(uniformId, 1 , GL_FALSE, glm::value_ptr(data)); GLERROR
}

template <>
void gl::Uniform<UniformType::MAT3>::set_data(Uniform<UniformType::MAT3>::dtype data) {
  CHECK_PROGRAM_ID;
  glUniformMatrix3fvARB(uniformId, 1 , GL_FALSE, glm::value_ptr(data)); GLERROR
}

template <>
void gl::Uniform<UniformType::MAT4>::set_data(Uniform<UniformType::MAT4>::dtype data) {
  CHECK_PROGRAM_ID;
  glUniformMatrix4fvARB(uniformId, 1 , GL_FALSE, glm::value_ptr(data)); GLERROR
}

template <>
void gl::Uniform<UniformType::SAMPLER2D>::set_data(Uniform<UniformType::SAMPLER2D>::dtype data) {
  CHECK_PROGRAM_ID;
  glUniform1i(uniformId, data); GLERROR
}


template <>
typename gl::Uniform<gl::UniformType::INTEGER>::type Uniform<gl::UniformType::INTEGER>::get_data() const {
  CHECK_PROGRAM_ID;
  typename gl::Uniform<gl::UniformType::INTEGER>::type val;
  glGetUniformiv(programId, uniformId, &val); GLERROR
  return val;
}

template <>
typename gl::Uniform<gl::UniformType::UINTEGER>::type Uniform<gl::UniformType::UINTEGER>::get_data() const {
  CHECK_PROGRAM_ID;
  typename gl::Uniform<gl::UniformType::UINTEGER>::type val;
  glGetUniformuiv(programId, uniformId, &val); GLERROR
  return val;
}

template <>
typename gl::Uniform<gl::UniformType::FLOAT>::type Uniform<gl::UniformType::FLOAT>::get_data() const {
  CHECK_PROGRAM_ID;
  typename gl::Uniform<gl::UniformType::FLOAT>::type val;
  glGetUniformfv(programId, uniformId, &val); GLERROR
  return val;
}

template <>
typename gl::Uniform<gl::UniformType::VEC2>::type Uniform<gl::UniformType::VEC2>::get_data() const {
  CHECK_PROGRAM_ID;
  typename gl::Uniform<gl::UniformType::VEC2>::type val;
  glGetUniformfv(programId, uniformId, glm::value_ptr(val)); GLERROR
  return val;
}

template <>
typename gl::Uniform<gl::UniformType::VEC3>::type Uniform<gl::UniformType::VEC3>::get_data() const {
  CHECK_PROGRAM_ID;
  typename gl::Uniform<gl::UniformType::VEC3>::type val;
  glGetUniformfv(programId, uniformId, glm::value_ptr(val)); GLERROR
  return val;
}

template <>
typename gl::Uniform<gl::UniformType::VEC4>::type Uniform<gl::UniformType::VEC4>::get_data() const {
  CHECK_PROGRAM_ID;
  typename gl::Uniform<gl::UniformType::VEC4>::type val;
  glGetUniformfv(programId, uniformId, glm::value_ptr(val)); GLERROR
  return val;
}

template <>
typename gl::Uniform<gl::UniformType::IVEC2>::type Uniform<gl::UniformType::IVEC2>::get_data() const {
  CHECK_PROGRAM_ID;
  typename gl::Uniform<gl::UniformType::IVEC2>::type val;
  glGetUniformiv(programId, uniformId, glm::value_ptr(val)); GLERROR
  return val;
}

template <>
typename gl::Uniform<gl::UniformType::IVEC3>::type Uniform<gl::UniformType::IVEC3>::get_data() const {
  CHECK_PROGRAM_ID;
  typename gl::Uniform<gl::UniformType::IVEC3>::type val;
  glGetUniformiv(programId, uniformId, glm::value_ptr(val)); GLERROR
  return val;
}

template <>
typename gl::Uniform<gl::UniformType::IVEC4>::type Uniform<gl::UniformType::IVEC4>::get_data() const {
  CHECK_PROGRAM_ID;
  typename gl::Uniform<gl::UniformType::IVEC4>::type val;
  glGetUniformiv(programId, uniformId, glm::value_ptr(val)); GLERROR
  return val;
}

template <>
typename gl::Uniform<gl::UniformType::UVEC2>::type Uniform<gl::UniformType::UVEC2>::get_data() const {
  CHECK_PROGRAM_ID;
  typename gl::Uniform<gl::UniformType::UVEC2>::type val;
  glGetUniformuiv(programId, uniformId, glm::value_ptr(val)); GLERROR
  return val;
}

template <>
typename gl::Uniform<gl::UniformType::UVEC3>::type Uniform<gl::UniformType::UVEC3>::get_data() const {
  CHECK_PROGRAM_ID;
  typename gl::Uniform<gl::UniformType::UVEC3>::type val;
  glGetUniformuiv(programId, uniformId, glm::value_ptr(val)); GLERROR
  return val;
}

template <>
typename gl::Uniform<gl::UniformType::UVEC4>::type Uniform<gl::UniformType::UVEC4>::get_data() const {
  CHECK_PROGRAM_ID;
  typename gl::Uniform<gl::UniformType::UVEC4>::type val;
  glGetUniformuiv(programId, uniformId, glm::value_ptr(val)); GLERROR
  return val;
}


template <>
typename gl::Uniform<gl::UniformType::MAT2>::type Uniform<gl::UniformType::MAT2>::get_data() const {
  CHECK_PROGRAM_ID;
  typename gl::Uniform<gl::UniformType::MAT2>::type val;
  glGetUniformfv(programId, uniformId, glm::value_ptr(val)); GLERROR
  return val;
}

template <>
typename gl::Uniform<gl::UniformType::MAT3>::type Uniform<gl::UniformType::MAT3>::get_data() const {
  CHECK_PROGRAM_ID;
  typename gl::Uniform<gl::UniformType::MAT3>::type val;
  glGetUniformfv(programId, uniformId, glm::value_ptr(val)); GLERROR
  return val;
}

template <>
typename gl::Uniform<gl::UniformType::MAT4>::type Uniform<gl::UniformType::MAT4>::get_data() const {
  CHECK_PROGRAM_ID;
  typename gl::Uniform<gl::UniformType::MAT4>::type val;
  glGetUniformfv(programId, uniformId, glm::value_ptr(val)); GLERROR
  return val;
}

template <>
typename gl::Uniform<gl::UniformType::SAMPLER2D>::type Uniform<gl::UniformType::SAMPLER2D>::get_data() const {
  CHECK_PROGRAM_ID;
  typename gl::Uniform<gl::UniformType::INTEGER>::type val;
  glGetUniformiv(programId, uniformId, &val); GLERROR
  return val;
}

#undef CHECK_PROGRAM_ID
}
