#include <string>
#include <cstdint>
#include <cctype>

#include <incgraphics.h>

#include <Logger.hpp>
#include <Debug.hpp>

#include <ShaderProgram.hpp>
#include <ShaderAttrib.hpp>
#include <ShaderUniform.hpp>
#include <Texture.hpp>

using namespace std::literals::string_literals;

GLFWwindow *g_window = nullptr;

/* GLvoid debug_callback(GLenum source, GLenum type, GLuint id, */
/*                               GLuint severity, GLsizei length, */
/*                               const GLchar *message, const GLvoid *userParam) */
/* { */
/*   Logger::Info("%s\n", message); */
/* } */

class Window {
protected:
  const GLFWvidmode *vidmode = nullptr;
  size_t width_, height_;
  void init_glfw() {
    int rc = glfwInit();
    ASSERT(rc == 1);

    vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    ASSERT(vidmode != nullptr);
    width_ = vidmode->width;
    height_ = vidmode->height;
    width_ = 1000, height_ = 1000;
    /* width_ = height_ = std::min(width_, height_); */

    /* glfwWindowHint(GLFW_SAMPLES, 4); */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL

    g_window = window = glfwCreateWindow(width(), height(), "automata", nullptr, nullptr);
    ASSERT(window != nullptr);
    glfwMakeContextCurrent(window); GLERROR
    Logger::Info("initialized glfw\n");
  }
  void init_glew() {
    // Initialize GLEW
    glewExperimental = GL_TRUE; // Needed for core profile
    auto res = glewInit(); GLERROR
    // on some systems, returns invalid value even if succeeds
    if(res != GLEW_OK) {
      Logger::Error("glew error: %s\n", "there was some error initializing glew");
      /* Logger::Error("glew error: %s\n", glewGetErrorString()); */
    }
    Logger::Info("initialized glew\n");
  }
  void init_controls() {
    // ensure we can capture the escape key
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE); GLERROR
    /* glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); GLERROR */
    Logger::Info("initialized controls\n");
  }
public:
  GLFWwindow *window = nullptr;
  Window():
    width_(0),
    height_(0)
  {}
  inline size_t width() const { return width_; }
  inline size_t height() const { return height_; }
  template <typename SF, typename DF, typename CF>
  void run(SF &&setupfunc, DF &&dispfunc, CF &&cleanupfunc) {
    init_glfw();
    init_glew();
    init_controls();
    /* glDebugMessageCallbackARB(&debug_callback, nullptr); GLERROR */
    setupfunc(*this);
    glfwSwapInterval(1); GLERROR
    while(!glfwWindowShouldClose(window)) {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); GLERROR
      dispfunc(*this);
      glfwPollEvents(); GLERROR
      glfwSwapBuffers(window); GLERROR
    }
    cleanupfunc(*this);
    glfwDestroyWindow(window); GLERROR
    glfwTerminate(); GLERROR
  }
  ~Window() {
    g_window = nullptr;
  }
};

class App {
  Window w;
public:
  App():
    w()
  {}

  template <typename... Ts>
  void run(Ts... args) {
    Logger::Setup("app.log");
    Logger::MirrorLog(stderr);
    w.run(std::forward<Ts>(args)...);
    Logger::Close();
  }
};

#include <Automaton.hpp>
#include <RLEDecoder.hpp>
#include <PlainDecoder.hpp>
#include <Life106Decoder.hpp>

template <typename AUT, typename StorageMode, typename AccessMode> struct Renderer;

template <typename AUT, typename AccessMode>
struct Renderer<AUT, storage_mode::HostBuffer, AccessMode> {
  using StorageT = Storage<AUT::dim, storage_mode::HostBuffer>;
  using AccessT = Access<AUT, StorageT, AccessMode>;

  int w, h;
  int tw, th;

  int color_per_state;

  GLuint tex = 0;
  gl::Uniform<gl::UniformType::SAMPLER2D> uSampler;

  int8_t current_buf = 0;
  bool extrabuf = false;
  StorageT finalbuf, buf1, buf2;

  Renderer(std::string uniform_name):
    uSampler(uniform_name.c_str()),
    w(0), h(0)
  {
    color_per_state = (AUT::no_states == 0) ? 1 : UINT8_MAX / (AUT::no_states - 1);
  }

  void init(int w_, int h_, int zoom=0, const char *filename=nullptr) {
    if(zoom==0)zoom=1;
    if(zoom >= 0) {
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
    buf2.init(w, h);
    for(int i=0;i<w*h;++i) {
      buf1.data[i]=0;
      buf2.data[i]=0;
    }
    if(filename == nullptr) {
      #pragma omp parallel for
      for(int i = 0; i < w * h; ++i) {
        buf1.data[i] = AUT::init_state(i / buf1.w, i % buf1.w) * color_per_state;
      }
    } else {
      RLEDecoder<StorageT>::read(filename, buf1);
      /* Life106Decoder<StorageT>::read(filename, buf1); */
      #pragma omp parallel for
      for(int i=0;i<w*h;++i)buf1.data[i]*=color_per_state;
    }
    gl::Texture<GL_TEXTURE_2D>::init(tex);
    reinit_texture();
  }

  void update() {
    StorageT srcbuf = buf1, dstbuf = buf2;
    if(current_buf) {
      std::swap(srcbuf, dstbuf);
    }
    #pragma omp parallel for
    for(int i = 0; i < w*h; ++i) {
      dstbuf.data[i] = AUT::next_state(make_grid<4>([=](int y, int x) mutable -> typename StorageT::value_type {
        return AccessT::access(srcbuf, y, x) / color_per_state;
      }, w, h), i / w, i % w) * color_per_state;
    }
    current_buf = current_buf ? 0 : 1;
    reinit_texture();
  }

  void reinit_texture() {
    StorageT srcbuf = !current_buf ? buf1 : buf2;
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
            sum += srcbuf.data[(i / tw * per_y + iy) * w + (i % tw) * per_x + ix];
            ++q;
          }
        }
        finalbuf.data[i] = uint8_t(sum / q);
      }
      srcbuf = finalbuf;
    }
    gl::Texture<GL_TEXTURE_2D>::bind(tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, tw, th, 0, GL_RED, GL_UNSIGNED_BYTE, srcbuf.data); GLERROR
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

  void set_data(int index) {
    uSampler.set_data(index);
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

template <>
struct Renderer<cellular::GameOfLife, storage_mode::Textures, access_mode::bounded> {
  using AUT = cellular::GameOfLife;
  int w, h;
  int tw,th;
  gl::ShaderProgram<gl::ComputeShader> compute;
  gl::Uniform<gl::UniformType::SAMPLER2D> uSampler;
  int8_t current_tex = 0;
  GLuint tex1 = 0, tex2 = 0;

  using ShaderProgram = decltype(compute);

  Renderer(std::string s):
    w(0), h(0),
    uSampler(s),
    compute({"shaders/conway.comp"})
  {}

  void init(int w_, int h_, int zoom=0) {
    w=w_,h=h_;
    ShaderProgram::compile_program(compute);
    if(!zoom)zoom=1;
    if(zoom > 0) {
      w_/=zoom,h_/=zoom;
    } else {
      w_*=-zoom,h_*=-zoom;
    }
    tw=w_,th=h_;
    uint8_t *buf = new uint8_t[w*h];
    for(int i=0;i<w*h;++i)buf[i]=AUT::init_state(i/w,i%w)?~uint8_t(0):0;
    for(GLuint *tex : {&tex1, &tex2}) {
      gl::Texture<GL_TEXTURE_2D>::init(*tex);
      gl::Texture<GL_TEXTURE_2D>::bind(*tex);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, tw, th, 0, GL_RED, GL_UNSIGNED_BYTE, buf); GLERROR
      gl::Texture<GL_TEXTURE_2D>::param(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      gl::Texture<GL_TEXTURE_2D>::param(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      gl::Texture<GL_TEXTURE_2D>::param(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      gl::Texture<GL_TEXTURE_2D>::param(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      gl::Texture<GL_TEXTURE_2D>::unbind();
    }
    delete [] buf;
  }

  void update() {
    /* int wg_size[3]; */
    /* glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &wg_size[0]); GLERROR */
    /* glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &wg_size[1]); GLERROR */
    /* glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &wg_size[2]); GLERROR */
    /* Logger::Info("max work group sizes: [%d %d %d]\n", wg_size[0], wg_size[1], wg_size[2]); */
    /* int wg_invocations; */
    /* glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &wg_invocations); GLERROR */
    /* Logger::Info("max work group invocations: %d\n", wg_invocations); */
    /* Logger::Info("current tex: %d\n", current_tex); */
    GLuint srctex = current_tex?tex1:tex2,
           dsttex = current_tex?tex2:tex1;
    ShaderProgram::use(compute);
    glBindImageTexture(0, srctex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8); GLERROR
    glBindImageTexture(1, dsttex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8); GLERROR
    glUniform1i(glGetUniformLocation(compute.id(), "srcTex"), 0);
    glUniform1i(glGetUniformLocation(compute.id(), "dstTex"), 1);
    int lsX=1,lsY=1;
    compute.dispatch(GLuint(tw)/lsX, GLuint(th)/lsY, 1);
    ShaderProgram::unuse();
    current_tex=current_tex?0:1;
    /* glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT */
    /*                 | GL_TEXTURE_UPDATE_BARRIER_BIT); GLERROR */
    glMemoryBarrier(GL_ALL_BARRIER_BITS); GLERROR
    glFinish(); GLERROR
  }

  void set_active(int index) {
    gl::Texture<GL_TEXTURE_2D>::set_active(index);
  }

  void set_data(int index) {
    uSampler.set_data(index);
  }

  void bind_texture() {
    gl::Texture<GL_TEXTURE_2D>::bind(current_tex?tex1:tex2);
  }

  void unbind_texture() {
    gl::Texture<GL_TEXTURE_2D>::unbind();
  }

  void clear() {
    gl::Texture<GL_TEXTURE_2D>::clear(tex1);
    gl::Texture<GL_TEXTURE_2D>::clear(tex2);
    ShaderProgram::clear(compute);
  }
};

int main(int argc, char *argv[]) {
  srand(time(NULL));

  App app;

  Renderer<cellular::DayAndNight, storage_mode::HostBuffer, access_mode::looped> automaton("grid"s);
  gl::VertexArray vao;
  gl::Attrib<GL_ARRAY_BUFFER, gl::AttribType::VEC2> attrVertex("vertex"s);
  gl::ShaderProgram<
    gl::VertexShader,
    gl::FragmentShader
  > prog({"shaders/aut4.vert"s, "shaders/aut4.frag"s});

  using ShaderAttrib = decltype(attrVertex);
  using ShaderProgram = decltype(prog);

  app.run(
    // setup function
    [&](auto &w) mutable {
      Logger::Info("init\n");
      // init attribute vertex
      ShaderAttrib::init(attrVertex);
      attrVertex.allocate<GL_STATIC_DRAW>(6, (float[]){
        1,1, -1,1, -1,-1,
        -1,-1, 1,1, 1,-1,
      });
      // add the attribute to the vertex array
      gl::VertexArray::init(vao);
      gl::VertexArray::bind(vao);
      vao.enable(attrVertex);
      vao.set_access(attrVertex, 0, 0);
      gl::VertexArray::unbind();
      // init shader program
      ShaderProgram::init(prog, vao, {"attrVertex"});
      // init texture
      const int factor = 1;
      automaton.init(w.width(), w.height(), factor);
      automaton.uSampler.set_id(prog.id());
      Logger::Info("init fin\n");
    },
    // display function
    [&](auto &w) mutable {
      // use program
      /* Logger::Info("draw\n"); */
      automaton.update();
      /* usleep(1e4*10); */
      // display
      ShaderProgram::use(prog);
      automaton.set_active(0);
      automaton.bind_texture();
      automaton.set_data(0);
      gl::VertexArray::bind(vao);
      glDrawArrays(GL_TRIANGLES, 0, 6); GLERROR
      gl::VertexArray::unbind();
      // unuse
      automaton.unbind_texture();
      ShaderProgram::unuse();
    },
    // cleanup function
    [&](auto &w) mutable {
      Logger::Info("clear\n");
      automaton.clear();
      ShaderAttrib::clear(attrVertex);
      gl::VertexArray::clear(vao);
      ShaderProgram::clear(prog);
    }
  );
}
