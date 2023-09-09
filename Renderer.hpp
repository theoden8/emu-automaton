#pragma once

#include <Logger.hpp>
#include <Debug.hpp>

#include <ShaderProgram.hpp>
#include <ShaderAttrib.hpp>
#include <ShaderUniform.hpp>
#include <Texture.hpp>

#include <Automaton.hpp>
#include <RLEDecoder.hpp>
#include <PlainDecoder.hpp>
#include <Life106Decoder.hpp>

using namespace std::literals::string_literals;

template <typename AUT, typename StorageMode, typename AccessMode> struct Renderer;

template <typename AUT, typename AccessMode>
struct Renderer<AUT, storage_mode::HostBuffer, AccessMode> {
  using StorageT = RenderStorage<storage_mode::HostBuffer>;
  using AccessT = Access<AUT, StorageT, AccessMode>;

  AUT &aut;
  int w, h;
  int tw, th;

  GLuint tex = 0;
  gl::Uniform<gl::UniformType::SAMPLER2D> uSampler;
  gl::Uniform<gl::UniformType::INTEGER> uNstates;

  int8_t current_buf = 0;
  static constexpr bool doublebuffer = (AUT::update_mode == ::update_mode::ALL);
  bool extrabuf = false;
  StorageT finalbuf, buf1, buf2;

  const int color_per_state;

  explicit Renderer(AUT &aut, const std::string &u_sampler_name, const std::string &u_nstates_name):
    aut(aut),
    w(0), h(0),
    tw(0), th(0),
    uSampler(u_sampler_name.c_str()),
    uNstates(u_nstates_name.c_str()),
    color_per_state(UINT8_MAX / (AUT::no_states - 1))
  {}

  void init(int w_, int h_, int zoom=0, const char *filename=nullptr) {
    if(zoom==0)zoom=1;
    if(zoom > 0) {
      w_ /= zoom, h_ /= zoom;
      tw = w_, th = h_;
    } else if(zoom < 0) {
      tw = w_, th = h_;
      w_ *= -zoom, h_ *= -zoom;
    }
    w=w_,h=h_;
    printf("[%d %d] [%d %d]\n", w,h,tw,th);
    if(w!=tw||h!=th||extrabuf) {
      extrabuf = true;
      finalbuf.init(tw, th);
    }
    buf1.init(w, h);
    if constexpr(doublebuffer) {
      buf2.init(w, h);
    }
    for(int i=0;i<w*h;++i) {
      buf1.buffer[i]=0;
      if constexpr(doublebuffer) {
        buf2.buffer[i]=0;
      }
    }
    if(filename == nullptr) {
      #pragma omp parallel for
      for(int i = 0; i < w * h; ++i) {
        buf1.buffer[i] = AUT::init_state(i / buf1.w, i % buf1.w) * color_per_state;
      }
    } else {
      RLEDecoder<StorageT>::read(filename, buf1);
      /* Life106Decoder<StorageT>::read(filename, buf1); */
      #pragma omp parallel for
      for(int i=0;i<w*h;++i)buf1.buffer[i]*=color_per_state;
    }
    gl::Texture<GL_TEXTURE_2D>::init(tex);
    reinit_texture();
  }

  void update() {
    if constexpr(doublebuffer) {
      StorageT *srcbuf = &buf1, *dstbuf = &buf2;
      if(current_buf) {
        std::swap(srcbuf, dstbuf);
      }
      static_assert(AUT::update_mode == ::update_mode::ALL, "ambiguous update mode");
      if constexpr(AUT::update_mode == ::update_mode::ALL) {
        #pragma omp parallel for
        for(int i = 0; i < w*h; ++i) {
          dstbuf->buffer[i] = aut.next_state(make_grid<4>([=, this](int y, int x) mutable -> typename StorageT::value_type {
            return AccessT::access(*srcbuf, y, x) / color_per_state;
          }, w, h), i / w, i % w) * color_per_state;
        }
      }
    } else {
      StorageT *srcbuf = &buf1, *dstbuf = &buf1;
      if constexpr(AUT::update_mode == ::update_mode::CURSOR) {
        int num_updates = std::sqrt(w * h);
        //int num_updates = 1;
        for(int i = 0; i < num_updates; ++i) {
          auto [index, val] = aut.next_state(make_grid<4>([=, this](int y, int x) mutable -> typename StorageT::value_type {
            return AccessT::access(*srcbuf, y, x) / color_per_state;
          }, w, h));
          dstbuf->buffer[index] = val * color_per_state;
        }
      }
    }
    if constexpr(doublebuffer) {
      current_buf = current_buf ? 0 : 1;
    }
    reinit_texture();
  }

  void reinit_texture() {
    const StorageT *srcbuf = !current_buf ? &buf1 : &buf2;
    if(extrabuf) {
      int per_x = w / tw;
      int per_y = h / th;
      #pragma omp parallel for
      for(int i = 0; i < tw*th; ++i) {
        uint32_t sum = 0;
        int y = i / tw, x = i % tw;
        int q = 0;
        for(int iy = 0; iy < per_y; ++iy) {
          for(int ix = 0; ix < per_x; ++ix) {
            sum += srcbuf->buffer[(i / tw * per_y + iy) * w + (i % tw) * per_x + ix];
            ++q;
          }
        }
        finalbuf.buffer[i] = uint8_t(sum / q);
      }
      srcbuf = &finalbuf;
    }
    gl::Texture<GL_TEXTURE_2D>::bind(tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, tw, th, 0, GL_RED, GL_UNSIGNED_BYTE, srcbuf->data()); GLERROR
    gl::Texture<GL_TEXTURE_2D>::param(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl::Texture<GL_TEXTURE_2D>::param(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl::Texture<GL_TEXTURE_2D>::param(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl::Texture<GL_TEXTURE_2D>::param(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D); GLERROR
    gl::Texture<GL_TEXTURE_2D>::unbind();
  }

  static void set_active(int index=0) {
    glActiveTexture(GL_TEXTURE0 + index); GLERROR
  }

  static GLint get_active_texture() {
    GLint active_tex;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &active_tex); GLERROR
    return active_tex;
  }

  void bind_texture() {
    gl::Texture<GL_TEXTURE_2D>::bind(tex);
  }

  template <typename ShaderProgramT>
  void init_uniforms(const ShaderProgramT &prog) {
    uSampler.set_id(prog.id());
    uNstates.set_id(prog.id());
    uNstates.set_data(AUT::no_states);
  }

  void set_data(int index) {
    uSampler.set_data(index);
    uNstates.set_data(AUT::no_states);
  }

  static void unbind_texture() {
    gl::Texture<GL_TEXTURE_2D>::unbind();
  }

  void clear() {
    gl::Texture<GL_TEXTURE_2D>::clear(tex);
    buf1.clear();
    buf2.clear();
    if(extrabuf) {
      finalbuf.clear();
      extrabuf = false;
    }
  }
};

/* template <> */
/* struct Renderer<cellular::GameOfLife, storage_mode::Textures, access_mode::bounded> { */
/*   using AUT = cellular::GameOfLife; */
/*   int w, h; */
/*   int tw,th; */
/*   gl::ShaderProgram<gl::ComputeShader> compute; */
/*   gl::Uniform<gl::UniformType::SAMPLER2D> uSampler; */
/*   int8_t current_tex = 0; */
/*   GLuint tex1 = 0, tex2 = 0; */

/*   using ShaderProgram = decltype(compute); */

/*   Renderer(std::string s): */
/*     w(0), h(0), */
/*     uSampler(s), */
/*     compute({"shaders/conway.comp"}) */
/*   {} */

/*   void init(int w_, int h_, int zoom=0) { */
/*     w=w_,h=h_; */
/*     ShaderProgram::compile_program(compute); */
/*     if(!zoom)zoom=1; */
/*     if(zoom > 0) { */
/*       w_/=zoom,h_/=zoom; */
/*     } else { */
/*       w_*=-zoom,h_*=-zoom; */
/*     } */
/*     tw=w_,th=h_; */
/*     uint8_t *buf = new uint8_t[w*h]; */
/*     for(int i=0;i<w*h;++i)buf[i]=AUT::init_state(i/w,i%w)?~uint8_t(0):0; */
/*     for(GLuint *tex : {&tex1, &tex2}) { */
/*       gl::Texture<GL_TEXTURE_2D>::init(*tex); */
/*       gl::Texture<GL_TEXTURE_2D>::bind(*tex); */
/*       glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, tw, th, 0, GL_RED, GL_UNSIGNED_BYTE, buf); GLERROR */
/*       gl::Texture<GL_TEXTURE_2D>::param(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); */
/*       gl::Texture<GL_TEXTURE_2D>::param(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); */
/*       gl::Texture<GL_TEXTURE_2D>::param(GL_TEXTURE_MAG_FILTER, GL_NEAREST); */
/*       gl::Texture<GL_TEXTURE_2D>::param(GL_TEXTURE_MIN_FILTER, GL_NEAREST); */
/*       gl::Texture<GL_TEXTURE_2D>::unbind(); */
/*     } */
/*     delete [] buf; */
/*   } */

/*   void update() { */
/*     /1* int wg_size[3]; *1/ */
/*     /1* glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &wg_size[0]); GLERROR *1/ */
/*     /1* glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &wg_size[1]); GLERROR *1/ */
/*     /1* glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &wg_size[2]); GLERROR *1/ */
/*     /1* Logger::Info("max work group sizes: [%d %d %d]\n", wg_size[0], wg_size[1], wg_size[2]); *1/ */
/*     /1* int wg_invocations; *1/ */
/*     /1* glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &wg_invocations); GLERROR *1/ */
/*     /1* Logger::Info("max work group invocations: %d\n", wg_invocations); *1/ */
/*     /1* Logger::Info("current tex: %d\n", current_tex); *1/ */
/*     GLuint srctex = current_tex?tex1:tex2, */
/*            dsttex = current_tex?tex2:tex1; */
/*     ShaderProgram::use(compute); */
/*     glBindImageTexture(0, srctex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8); GLERROR */
/*     glBindImageTexture(1, dsttex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8); GLERROR */
/*     glUniform1i(glGetUniformLocation(compute.id(), "srcTex"), 0); */
/*     glUniform1i(glGetUniformLocation(compute.id(), "dstTex"), 1); */
/*     int lsX=1,lsY=1; */
/*     compute.dispatch(GLuint(tw)/lsX, GLuint(th)/lsY, 1); */
/*     ShaderProgram::unuse(); */
/*     current_tex=current_tex?0:1; */
/*     /1* glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT *1/ */
/*     /1*                 | GL_TEXTURE_UPDATE_BARRIER_BIT); GLERROR *1/ */
/*     glMemoryBarrier(GL_ALL_BARRIER_BITS); GLERROR */
/*     glFinish(); GLERROR */
/*   } */

/*   void set_active(int index) { */
/*     gl::Texture<GL_TEXTURE_2D>::set_active(index); */
/*   } */

/*   void set_data(int index) { */
/*     uSampler.set_data(index); */
/*   } */

/*   void bind_texture() { */
/*     gl::Texture<GL_TEXTURE_2D>::bind(current_tex?tex1:tex2); */
/*   } */

/*   void unbind_texture() { */
/*     gl::Texture<GL_TEXTURE_2D>::unbind(); */
/*   } */

/*   void clear() { */
/*     gl::Texture<GL_TEXTURE_2D>::clear(tex1); */
/*     gl::Texture<GL_TEXTURE_2D>::clear(tex2); */
/*     ShaderProgram::clear(compute); */
/*   } */
/* }; */
