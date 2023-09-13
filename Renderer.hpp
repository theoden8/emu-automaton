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

template <typename AUT>
struct GenericRenderer {
  AUT &aut;
  int w, h;

  gl::Uniform<gl::UniformType::SAMPLER2D> uSampler;
  gl::Uniform<gl::UniformType::UINTEGER> uNstates;
  gl::Uniform<gl::UniformType::INTEGER> uColorscheme;

  virtual storage_mode get_storage_mode() = 0;

  explicit GenericRenderer(AUT &aut):
    aut(aut),
    w(0), h(0),
    uSampler("grid"s),
    uNstates("no_states"s),
    uColorscheme("colorscheme"s)
  {}

  virtual void init(int w_, int h_, int zoom=0, const char *filename=nullptr) = 0;
  virtual void update() = 0;

  template <typename ShaderProgramRenderT>
  void init_uniforms(const ShaderProgramRenderT &prog) {
    init_uniforms_renderer(prog, 0);
  }

  template <typename ShaderProgramRenderT>
  void init_uniforms_renderer(const ShaderProgramRenderT &prog, int colorscheme) {
    uSampler.set_id(prog.id());
    uNstates.set_id(prog.id());
    uNstates.set_data(AUT::no_states);
    uColorscheme.set_id(prog.id());
    uColorscheme.set_data(colorscheme);
  }

  void set_data_renderer(int index) {
    // TEXTURES: maybe
    // uSampler.set_datindex(current_tex ? 0 : 1);
    uSampler.set_data(index);
    uNstates.set_data(AUT::no_states);
    uColorscheme.set_data(0);
  }

  virtual void set_active(int index=0) = 0;

  GLint get_active_texture() {
    GLint active_tex;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &active_tex); GLERROR
    return active_tex;
  }

  virtual void bind_texture() = 0;

  static void unbind_texture() {
    gl::Texture<GL_TEXTURE_2D>::unbind();
  }

  virtual void clear() = 0;
};

template <typename AUT, storage_mode StorageMode, access_mode AccessMode> struct Renderer;

template <typename AUT, access_mode AccessMode>
struct Renderer<AUT, storage_mode::HOSTBUFFER, AccessMode> : public GenericRenderer<AUT> {
  using parent_t = GenericRenderer<AUT>;
  using StorageT = RenderStorage<storage_mode::HOSTBUFFER>;
  using AccessT = Access<AUT, StorageT, AccessMode>;

  using parent_t::aut;
  using parent_t::w;
  using parent_t::h;

  int tw, th;

  GLuint tex = 0;

  int8_t current_buf = 0;
  static constexpr bool doublebuffer = (AUT::update_mode == ::update_mode::ALL);
  bool extrabuf = false;
  StorageT finalbuf, buf1, buf2;

  storage_mode get_storage_mode() override {
    return storage_mode::HOSTBUFFER;
  }

  explicit Renderer(AUT &aut, const std::string &dir):
    parent_t(aut),
    tw(0), th(0)
  {}

  void init(int w_, int h_, int zoom=0, const char *filename=nullptr) override {
    if(zoom==0)zoom=1;
    if(zoom > 0) {
      w_ /= zoom, h_ /= zoom;
      tw=w_, th=h_;
    } else if(zoom < 0) {
      tw=w_, th=h_;
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
        buf1.buffer[i] = AUT::init_state(i / buf1.w, i % buf1.w);
      }
    } else {
      RLEDecoder<StorageT>::read(filename, buf1);
      /* Life106Decoder<StorageT>::read(filename, buf1); */
    }
    gl::Texture<GL_TEXTURE_2D>::init(tex);
    reinit_texture();
  }

  void update() override {
    if constexpr(doublebuffer) {
      StorageT *srcbuf = &buf1, *dstbuf = &buf2;
      if(current_buf) {
        std::swap(srcbuf, dstbuf);
      }
      static_assert(AUT::update_mode == ::update_mode::ALL, "ambiguous update mode");
      if constexpr(AUT::update_mode == ::update_mode::ALL) {
        #pragma omp parallel for
        for(int i = 0; i < w*h; ++i) {
          dstbuf->buffer[i] = aut.next_state(make_grid<4>([=](int y, int x) mutable -> typename StorageT::value_type {
            return AccessT::access(*srcbuf, y, x);
          }, w, h), i / w, i % w);
        }
      }
    } else {
      StorageT *srcbuf = &buf1, *dstbuf = &buf1;
      if constexpr(AUT::update_mode == ::update_mode::CURSOR) {
        int num_updates = std::sqrt(w * h) * std::log2(w * h);
        //int num_updates = 1;
        for(int i = 0; i < num_updates; ++i) {
          auto [index, val] = aut.next_state(make_grid<4>([=](int y, int x) mutable -> typename StorageT::value_type {
            return AccessT::access(*srcbuf, y, x);
          }, w, h));
          dstbuf->buffer[index] = val;
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
      const int q = per_x * per_y;
      #pragma omp parallel for
      for(int i = 0; i < tw*th; ++i) {
        uint32_t sum = 0;
        int y = i / tw, x = i % tw;
        for(int iy = 0; iy < per_y; ++iy) {
          for(int ix = 0; ix < per_x; ++ix) {
            sum += srcbuf->buffer[(i / tw * per_y + iy) * w + (i % tw) * per_x + ix];
          }
        }
        finalbuf.buffer[i] = uint8_t(sum / q);
      }
      srcbuf = &finalbuf;
    }
    gl::Texture<GL_TEXTURE_2D>::bind(tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, tw, th, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, srcbuf->data()); GLERROR
    gl::Texture<GL_TEXTURE_2D>::param(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl::Texture<GL_TEXTURE_2D>::param(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl::Texture<GL_TEXTURE_2D>::param(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl::Texture<GL_TEXTURE_2D>::param(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D); GLERROR
    gl::Texture<GL_TEXTURE_2D>::unbind();
  }

  void set_active(int index=0) override {
    glActiveTexture(GL_TEXTURE0 + index); GLERROR
  }

  void bind_texture() override {
    gl::Texture<GL_TEXTURE_2D>::bind(tex);
  }

  template <typename ShaderProgramRenderT>
  void init_uniforms(const ShaderProgramRenderT &prog) {
    parent_t::init_uniforms_renderer(prog, extrabuf ? 1 : 0);
  }

  void clear() override {
    gl::Texture<GL_TEXTURE_2D>::clear(tex);
    buf1.clear();
    buf2.clear();
    if(extrabuf) {
      finalbuf.clear();
      extrabuf = false;
    }
  }
};

template <size_t C, access_mode AccessMode>
struct Renderer<ca::BSC<C>, storage_mode::TEXTURES, AccessMode> : public GenericRenderer<ca::BSC<C>> {
  using AUT = ca::BSC<C>;
  using parent_t = GenericRenderer<AUT>;
  using StorageT = RenderStorage<storage_mode::HOSTBUFFER>;

  using parent_t::aut;
  using parent_t::w;
  using parent_t::h;

  int8_t current_tex = 0;
  GLuint tex1 = 0, tex2 = 0;
  int wgsize = 1;
  bool largetexture = false;

  gl::Uniform<gl::UniformType::SAMPLER2D> uSrcTex, uDstTex;
  gl::Uniform<gl::UniformType::UINTEGER> uBs, uSs, uC;
  gl::Uniform<gl::UniformType::INTEGER> uW, uH, uWgsize;
  gl::Uniform<gl::UniformType::UINTEGER> uAccessMode;
  gl::ShaderProgram<gl::ComputeShader> compute;

  using ShaderProgramCompute = decltype(compute);

  storage_mode get_storage_mode() override {
    return storage_mode::TEXTURES;
  }

  explicit Renderer(AUT &aut, const std::string &dir):
    parent_t(aut),
    uSrcTex("srcbuf"s), uDstTex("dstbuf"s),
    uBs("bs"s), uSs("ss"s), uC("c"s),
    uW("w"s), uH("h"s), uWgsize("wgsize"),
    uAccessMode("access_mode"s),
    compute({std::string(sys::Path(dir) / sys::Path("shaders"s) / sys::Path("bsc.comp"s))})
  {}

  void init(int w_, int h_, int zoom=0, const char *filename=nullptr) override {
    w=w_,h=h_;
    if(!zoom)zoom=1;
    if(zoom > 0) {
      w/=zoom,h/=zoom;
      wgsize = 2;
    } else {
      w*=-zoom,h*=-zoom;
      wgsize = -zoom + 1;
    }
    printf("w %d, h %d\n", w, h);
    largetexture = (zoom < 0);
    std::vector<uint8_t> buf(w * h, 0);
    #pragma omp parallel for
    for(int i = 0; i < w * h; ++i) {
      buf[i] = AUT::init_state(i/w, i%w);
    }
    ShaderProgramCompute::compile_program(compute);
    for(GLuint *tex_ptr : {&tex1, &tex2}) {
      GLuint &tex = *tex_ptr;
      gl::Texture<GL_TEXTURE_2D>::init(tex);
      gl::Texture<GL_TEXTURE_2D>::bind(tex);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, w, h, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, buf.data()); GLERROR
      gl::Texture<GL_TEXTURE_2D>::param(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      gl::Texture<GL_TEXTURE_2D>::param(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      gl::Texture<GL_TEXTURE_2D>::param(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      gl::Texture<GL_TEXTURE_2D>::param(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      gl::Texture<GL_TEXTURE_2D>::unbind();
    }
  }

  void init_uniforms_compute() {
    uSrcTex.set_id(compute.id());
    uDstTex.set_id(compute.id());
    uBs.set_id(compute.id());
    uSs.set_id(compute.id());
    uC.set_id(compute.id());
    uW.set_id(compute.id());
    uH.set_id(compute.id());
    uWgsize.set_id(compute.id());
    uAccessMode.set_id(compute.id());
  }

  template <typename ShaderProgramRenderT>
  void init_uniforms(const ShaderProgramRenderT &prog) {
    init_uniforms_compute();
    this->init_uniforms_renderer(prog, largetexture ? 1 : 0);
  }

  void update() override {
    set_data_compute();
    /* int wg_size[3]; */
    /* glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &wg_size[0]); GLERROR */
    /* glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &wg_size[1]); GLERROR */
    /* glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &wg_size[2]); GLERROR */
    /* Logger::Info("max work group sizes: [%d %d %d]\n", wg_size[0], wg_size[1], wg_size[2]); */
    /* int wg_invocations; */
    /* glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &wg_invocations); GLERROR */
    /* Logger::Info("max work group invocations: %d\n", wg_invocations); */
    /* Logger::Info("current tex: %d\n", current_tex); */
    ShaderProgramCompute::use(compute);
    set_data_compute();
    GLuint srctex = current_tex?tex1:tex2,
           dsttex = current_tex?tex2:tex1;
    glBindImageTexture(0, srctex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8UI); GLERROR
    glBindImageTexture(1, dsttex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8UI); GLERROR
    compute.dispatch(GLuint(w + wgsize - 1) / wgsize, GLuint(h + wgsize - 1) / wgsize, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT); GLERROR
    //glMemoryBarrier(GL_ALL_BARRIER_BITS); GLERROR
    //glFinish(); GLERROR
    ShaderProgramCompute::unuse();
    current_tex = current_tex ? 0 : 1;
  }

  void set_active(int index=0) override {
    gl::Texture<GL_TEXTURE_2D>::set_active(GL_TEXTURE0 + index + current_tex);
  }

  void set_data_compute() {
    uSrcTex.set_data(current_tex ? 0 : 1);
    uDstTex.set_data(current_tex ? 1 : 0);
    int bs = int(aut.bs_bitmask.to_ulong());;
    uBs.set_data(bs);
    int ss = int(aut.ss_bitmask.to_ulong());;
    uSs.set_data(ss);
    uC.set_data(AUT::no_states);
    uW.set_data(w);
    uH.set_data(h);
    uWgsize.set_data(wgsize);
    uAccessMode.set_data(AccessMode);
  }

  void bind_texture() override {
    gl::Texture<GL_TEXTURE_2D>::bind(current_tex?tex1:tex2);
  }

  void clear() override {
    gl::Texture<GL_TEXTURE_2D>::clear(tex1);
    gl::Texture<GL_TEXTURE_2D>::clear(tex2);
    ShaderProgramCompute::clear(compute);
  }
};
