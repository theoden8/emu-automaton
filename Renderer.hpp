#pragma once

#include <Logger.hpp>
#include <Debug.hpp>

#include <ShaderProgram.hpp>
#include <ShaderUniform.hpp>
#include <Texture.hpp>
#include <Window.hpp>

#include <Automaton.hpp>
#include <RLEDecoder.hpp>
#include <PlainDecoder.hpp>
#include <Life106Decoder.hpp>

using namespace std::literals::string_literals;

struct TexturedGridRenderer {
  int no_states;
  int w, h;

  gl::Buffer<GL_ARRAY_BUFFER, gl::BufferElementType::VEC2>
    bufVertex;
  gl::Attrib<decltype(bufVertex)>
    attrVertex; //("vertex"s);
  gl::VertexArray<decltype(attrVertex)>
    vao;
  gl::ShaderProgram<
    gl::VertexShader,
    gl::FragmentShader
  > prog;
  gl::Uniform<gl::UniformType::SAMPLER2D>
    uSampler;
  gl::Uniform<gl::UniformType::UINTEGER>
    uNstates;
  gl::Uniform<gl::UniformType::INTEGER>
    uColorscheme;

  using ShaderProgram = decltype(prog);

  int colorscheme = 0;

  virtual storage_mode get_storage_mode() = 0;

  explicit TexturedGridRenderer(int no_states, const std::string &dir):
    no_states(no_states),
    w(0), h(0),
    bufVertex(), attrVertex("vertex"s, bufVertex), vao(attrVertex),
    prog({
      std::string(sys::Path(dir) / sys::Path("shaders"s) / sys::Path("aut4.vert"s)),
      std::string(sys::Path(dir) / sys::Path("shaders"s) / sys::Path("aut4.frag"s))
    }),
    uSampler("grid"s),
    uNstates("no_states"s),
    uColorscheme("colorscheme"s)
  {}

  void init_renderer(Window &w, int factor) {
    // init attribute vertex
    bufVertex.init();
    std::vector<float> points = {
      1,1, -1,1, -1,-1,
      -1,-1, 1,1, 1,-1,
    };
    bufVertex.allocate<GL_STATIC_DRAW>(points);
    // add the attribute to the vertex array
    vao.init();
    vao.enable(attrVertex);
    vao.set_access(attrVertex, 0);
    // init shader program
    ShaderProgram::init(prog, vao);
    prog.assign_uniforms(uSampler, uNstates, uColorscheme);
    set_grid_size(w.width(), w.height(), factor);
    init_textures();
  }

  virtual void set_grid_size(int w_, int h_, int zoom) = 0;
  virtual void init_textures(const char *filename=nullptr) = 0;
  virtual void update_state() = 0;
  virtual GLuint get_current_texture_id() = 0;

  void render(int global_texture_index) {
    // display
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); GLERROR
    // use program
    ShaderProgram::use(prog);
    // set uniform data
    uSampler.set_data(global_texture_index);
    uNstates.set_data(no_states);
    uColorscheme.set_data(colorscheme);
    // choose texture
    gl::Texture<GL_TEXTURE_2D>::set_active(global_texture_index);
    gl::Texture<GL_TEXTURE_2D>::bind(get_current_texture_id());
    // draw triangles
    vao.draw<GL_TRIANGLES>();
    // unuse
    gl::Texture<GL_TEXTURE_2D>::unbind();
    ShaderProgram::unuse();
  }

  virtual void clear() {
    attrVertex.clear();
    bufVertex.clear();
    vao.clear();
    ShaderProgram::clear(prog);
    ShaderProgram::unassign_uniforms(uSampler, uNstates, uColorscheme);
  }
};

template <typename AUT, storage_mode StorageMode, access_mode AccessMode> struct Renderer;

template <typename AUT, access_mode AccessMode>
struct Renderer<AUT, storage_mode::HOSTBUFFER, AccessMode> : public TexturedGridRenderer {
  using parent_t = TexturedGridRenderer;
  using StorageT = RenderStorage<storage_mode::HOSTBUFFER>;
  using AccessT = Access<AUT, StorageT, AccessMode>;

  AUT &aut;
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

  explicit Renderer(AUT &_aut, const std::string &dir):
    parent_t(_aut.no_states, dir),
    aut(_aut),
    tw(0), th(0)
  {}

  void set_grid_size(int w_, int h_, int zoom) override {
    if(zoom==0)zoom=1;
    if(zoom > 0) {
      w_ /= zoom, h_ /= zoom;
      tw=w_, th=h_;
    } else if(zoom < 0) {
      tw=w_, th=h_;
      w_ *= -zoom, h_ *= -zoom;
    }
    w=w_,h=h_;
    Logger::Info("[%d %d] [%d %d]\n", w,h,tw,th);
    if(zoom < 0) {
      parent_t::colorscheme = 1;
      no_states = std::min<int>(256, (w/tw) * (h/th) * (aut.no_states - 1) + 1);
    }
  }

  void init_textures(const char *filename=nullptr) override {
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
        buf1.buffer[i] = aut.init_state(i / buf1.w, i % buf1.w);
      }
    } else {
      RLEDecoder<StorageT>::read(filename, buf1);
      /* Life106Decoder<StorageT>::read(filename, buf1); */
    }
    gl::Texture<GL_TEXTURE_2D>::init(tex);
    reinit_texture();
  }

  void update_state() override {
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
      const int area = per_x * per_y;
      const float scale_states = fmax(1, float(area * (aut.no_states - 1) + 1) / no_states);
      #pragma omp parallel for
      for(int i = 0; i < tw*th; ++i) {
        uint16_t sum = 0;
        int y = i / tw, x = i % tw;
        for(int iy = 0; iy < per_y; ++iy) {
          for(int ix = 0; ix < per_x; ++ix) {
            sum += srcbuf->buffer[(y*per_y+iy)*w + x*per_x+ix];
          }
        }
        finalbuf.buffer[i] = std::round<uint8_t>(float(sum) / scale_states - .01);
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

  GLuint get_current_texture_id() override {
    return tex;
  }

  void clear() override {
    gl::Texture<GL_TEXTURE_2D>::clear(tex);
    buf1.clear();
    buf2.clear();
    if(extrabuf) {
      finalbuf.clear();
      extrabuf = false;
    }
    parent_t::clear();
  }
};

template <access_mode AccessMode>
struct Renderer<ca::BSC, storage_mode::TEXTURES, AccessMode> : public TexturedGridRenderer {
  using AUT = ca::BSC;
  using parent_t = TexturedGridRenderer;
  using StorageT = RenderStorage<storage_mode::HOSTBUFFER>;

  AUT &aut;
  using parent_t::w;
  using parent_t::h;

  int8_t current_tex = 0;
  GLuint tex1 = 0, tex2 = 0;
  glm::ivec2 wg_per_cell = glm::ivec2(1, 1);
  bool largetexture = false;

  gl::Uniform<gl::UniformType::SAMPLER2D> uInitTex;
  gl::Uniform<gl::UniformType::UINTEGER> uNStates, uSeed;
  gl::ShaderProgram<gl::ComputeShader> computeInitSoup;
  gl::Uniform<gl::UniformType::SAMPLER2D> uSrcTex, uDstTex;
  gl::Uniform<gl::UniformType::UINTEGER> uBs, uSs, uC;
  gl::Uniform<gl::UniformType::IVEC2> uSize, uWgPerCell;
  gl::Uniform<gl::UniformType::UINTEGER> uAccessMode;
  gl::ShaderProgram<gl::ComputeShader> computeUpdate;
  static constexpr int local_size = 8;
  const int max_wg_invocations;
  glm::ivec2 wg_size = glm::ivec2(0, 0);

  using ShaderProgramCompute = decltype(computeUpdate);

  storage_mode get_storage_mode() override {
    return storage_mode::TEXTURES;
  }

  explicit Renderer(AUT &_aut, const std::string &dir):
    parent_t(_aut.no_states, dir),
    aut(_aut),
    uInitTex("initTex"s), uNStates("n_states"), uSeed("seed"),
    computeInitSoup({std::string(sys::Path(dir) / sys::Path("shaders"s) / sys::Path("soup.comp"s))}),
    uSrcTex("srcTex"s), uDstTex("dstTex"s),
    uBs("bs"s), uSs("ss"s), uC("c"s),
    uSize("size"s), uWgPerCell("wg_per_cell"),
    uAccessMode("access_mode"s),
    computeUpdate({std::string(sys::Path(dir) / sys::Path("shaders"s) / sys::Path("bsc.comp"s))}),
    max_wg_invocations(ShaderProgramCompute::get_max_wg_invocations())
  {}

  void set_grid_size(int w_, int h_, int zoom) override {
    w=w_,h=h_;
    if(!zoom)zoom=1;
    if(zoom > 0) {
      w/=zoom,h/=zoom;
    } else {
      w*=-zoom,h*=-zoom;
    }
    Logger::Info("[w %d, h %d]\n", w, h);
    largetexture = (zoom < 0);
    if(zoom < 0) {
      parent_t::colorscheme = 1;
    }
    set_work_group_sizes();
  }

  void set_work_group_sizes() {
    const glm::ivec2 max_wg_size = ShaderProgramCompute::get_max_wgsize();
    glm::ivec2 max_invocations = max_wg_size * local_size;
    wg_per_cell = (glm::ivec2(w, h) + max_invocations - 1) / max_invocations + 1;
//    Logger::Info("[max_invocations/w = %d/%d = %.2f wg_per_cell_x %d]\n", max_invocations, w, float(max_invocations) / float(w), wg_per_cell.x);
//    Logger::Info("[max_invocations/h = %d/%d = %.2f wg_per_cell_y %d]\n", max_invocations, w, float(max_invocations) / float(h), wg_per_cell.y);
    glm::ivec2 per_cell = wg_per_cell * local_size;
    wg_size = (glm::ivec2(w, h) + per_cell - 1) / per_cell;
    Logger::Info("[wg %dx%dx%d %dx%dx%d]\n", wg_size.x, local_size, wg_per_cell.x, wg_size.y, local_size, wg_per_cell.y);
  }

  #define COMPUTE_INIT_SOUP
  void init_textures(const char *filename=nullptr) override {
    for(GLuint *tex_ptr : {&tex1, &tex2}) {
      GLuint &tex = *tex_ptr;
      gl::Texture<GL_TEXTURE_2D>::init(tex);
      gl::Texture<GL_TEXTURE_2D>::bind(tex);
      if(tex_ptr == &tex1 && filename != nullptr) {
        RenderStorage<storage_mode::HOSTBUFFER> buf;
        buf.init(w, h);
//        #pragma omp parallel for
//        for(int i = 0; i < w * h; ++i) {
//          buf.buffer[i] = aut.init_state(i/w, i%w);
//        }
        RLEDecoder<RenderStorage<storage_mode::HOSTBUFFER>>::read(filename, buf);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, w, h, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, buf.buffer.data()); GLERROR
      } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, w, h, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr); GLERROR
      }
      gl::Texture<GL_TEXTURE_2D>::param(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      gl::Texture<GL_TEXTURE_2D>::param(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      gl::Texture<GL_TEXTURE_2D>::param(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      gl::Texture<GL_TEXTURE_2D>::param(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      gl::Texture<GL_TEXTURE_2D>::unbind();
    }
    //#ifdef COMPUTE_INIT_SOUP
    if(filename == nullptr) {
      ShaderProgramCompute::compile_program(computeInitSoup);
      computeInitSoup.assign_uniforms(
        uInitTex, uNStates,
        uSize, uWgPerCell, uSeed
      );
      init_state_soup();
      ShaderProgramCompute::clear(computeInitSoup);
      ShaderProgramCompute::unassign_uniforms(
        uInitTex, uNStates,
        uSize, uWgPerCell, uSeed
      );
    }
    //#endif
    ShaderProgramCompute::compile_program(computeUpdate);
    computeUpdate.assign_uniforms(
      uSrcTex, uDstTex,
      uBs, uSs, uC,
      uSize, uWgPerCell,
      uAccessMode
    );
    ShaderProgramCompute::print_compute_capabilities();
  }

  void set_data_compute_init_soup() {
    uInitTex.set_data(0);
    uNStates.set_data(aut.no_states);
    glm::ivec2 val_size(w, h);
    uSize.set_data(val_size);
    uWgPerCell.set_data(wg_per_cell);
    uSeed.set_data(rand());
  }

  void init_state_soup() {
    ShaderProgramCompute::use(computeInitSoup);
    set_data_compute_init_soup();
    GLuint inittex = tex1;
    glBindImageTexture(0, inittex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8UI); GLERROR
    ShaderProgramCompute::dispatch(wg_size.x, wg_size.y, 1);
    ShaderProgramCompute::barrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
    ShaderProgramCompute::unuse();
  }

  void set_data_compute_update() {
    uSrcTex.set_data(0);
    uDstTex.set_data(1);
    int bs = int(aut.bs_bitmask.to_ulong());;
    uBs.set_data(bs);
    int ss = int(aut.ss_bitmask.to_ulong());;
    uSs.set_data(ss);
    uC.set_data(aut.no_states);
    glm::ivec2 val_size(w, h);
    uSize.set_data(val_size);
    uWgPerCell.set_data(wg_per_cell);
    uAccessMode.set_data(AccessMode);
  }

  void update_state() override {
    ShaderProgramCompute::use(computeUpdate);
    set_data_compute_update();
    GLuint srctex = current_tex?tex2:tex1,
           dsttex = current_tex?tex1:tex2;
    glBindImageTexture(0, srctex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8UI); GLERROR
    glBindImageTexture(1, dsttex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8UI); GLERROR
    ShaderProgramCompute::dispatch(wg_size.x, wg_size.y, 1);
    ShaderProgramCompute::barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);
    //computeUpdate.barrier(GL_ALL_BARRIER_BITS); GLERROR
    //glFinish(); GLERROR
    ShaderProgramCompute::unuse();
    current_tex = current_tex ? 0 : 1;
    gl::Texture<GL_TEXTURE_2D>::bind(get_current_texture_id());
    glGenerateMipmap(GL_TEXTURE_2D); GLERROR
    gl::Texture<GL_TEXTURE_2D>::unbind();
  }

  GLuint get_current_texture_id() override {
    return current_tex ? tex1 : tex2;
  }

  void clear() override {
    gl::Texture<GL_TEXTURE_2D>::clear(tex1);
    gl::Texture<GL_TEXTURE_2D>::clear(tex2);
    ShaderProgramCompute::clear(computeUpdate);
    ShaderProgramCompute::unassign_uniforms(
      uSrcTex, uDstTex,
      uBs, uSs, uC,
      uSize, uWgPerCell,
      uAccessMode
    );
    parent_t::clear();
  }
};
