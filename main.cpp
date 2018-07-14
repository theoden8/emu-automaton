#include <string>
#include <cstdint>

#include "incgraphics.h"

#include "Logger.hpp"
#include "Debug.hpp"

#include "ShaderProgram.hpp"
#include "ShaderAttrib.hpp"
#include "ShaderUniform.hpp"

using namespace std::literals::string_literals;

GLFWwindow *g_window = nullptr;

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
    /* width_ = height_ = std::min(width_, height_); */

    /* glfwWindowHint(GLFW_SAMPLES, 4); */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL

    g_window = window = glfwCreateWindow(width(), height(), "automata", nullptr, nullptr);
    ASSERT(window != nullptr);
    glfwMakeContextCurrent(window); GLERROR
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
  }
  void init_controls() {
    // ensure we can capture the escape key
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE); GLERROR
    /* glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); GLERROR */
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

template <typename F>
struct Grid {
  F func;

  const int width;
  const int height;

  inline Grid(F &&func, int w, int h):
    func(func),
    width(w), height(h)
  {}

  struct Row {
    F func;

    const int y;
    const int width;

    inline Row(F &&func, int y, int w):
      func(func), y(y), width(w)
    {}

    inline decltype(auto) operator[](int x) {
      return func(y, x);
    }
  };

  inline Row operator[](int y) {
    return Row(std::forward<F>(func), y, width);
  }
};

template <typename AUT>
struct Board {
  size_t w, h;
  size_t tw, th;

  int color_per_state;

  GLuint tex = 0;
  gl::Uniform<gl::UniformType::SAMPLER2D> uSampler;

  int8_t current_buf = 0;
  bool extrabuf = false;
  uint8_t *finalbuf=nullptr, *buf1=nullptr, *buf2=nullptr;

  Board(std::string uniform_name):
    uSampler(uniform_name.c_str()),
    w(0), h(0)
  {
    color_per_state = (AUT::no_states == 0) ? 1 : UINT8_MAX / (AUT::no_states - 1);
  }

  void init(int w_, int h_, int zoom) {
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
      finalbuf = new uint8_t[tw * th];
    }
    buf1 = new uint8_t[w * h];
    buf2 = new uint8_t[w * h];
    #pragma omp parallel for
    for(int i = 0; i < w * h; ++i) {
      buf1[i] = AUT::init_state(i / w, i % w) * color_per_state;
      buf2[i] = 0;
    }
    glGenTextures(1, &tex); GLERROR
    reinit_texture();
  }

  void update() {
    uint8_t *srcbuf = buf1, *dstbuf = buf2;
    if(current_buf) {
      std::swap(srcbuf, dstbuf);
    }
    /* #pragma omp parallel for */
    for(int i = 0; i < w*h; ++i) {
      dstbuf[i] = AUT::next_state(Grid([=](int y, int x) -> uint8_t {
        if(y < 0 || y > h || x < 0 || x > w) {
          return AUT::outside_state;
        }
        return srcbuf[y * w + x] / color_per_state;
      }, w, h), i / w, i % w) * color_per_state;
    }
    current_buf = current_buf ? 0 : 1;
  }

  void reinit_texture() {
    uint8_t *srcbuf = !current_buf ? buf1 : buf2;
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
            sum += srcbuf[(i / tw * per_y + iy) * w + (i % tw) * per_x + ix];
            ++q;
          }
        }
        finalbuf[i] = uint8_t(sum / q);
      }
      srcbuf = finalbuf;
    }
    Board::bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, tw, th, 0, GL_RED, GL_UNSIGNED_BYTE, srcbuf); GLERROR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); GLERROR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); GLERROR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); GLERROR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); GLERROR
    glGenerateMipmap(GL_TEXTURE_2D); GLERROR
    Board::unbind();
  }

  static void set_active(int index=0) {
    glActiveTexture(GL_TEXTURE0 + index); GLERROR
  }

  static GLint get_active_texture() {
    GLint active_tex;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &active_tex); GLERROR
    return active_tex;
  }

  static void bind(GLuint texId) {
    glBindTexture(GL_TEXTURE_2D, texId); GLERROR
  }

  static void bind(Board &board) {
    board.bind();
  }

  void bind() {
    bind(tex);
  }

  void set_data(int index) {
    uSampler.set_data(index);
  }

  static void unbind() {
    glBindTexture(GL_TEXTURE_2D, 0); GLERROR
  }

  void clear() {
    glDeleteTextures(1, &tex); GLERROR
    delete [] buf1;
    delete [] buf2;
    if(extrabuf) {
      delete [] finalbuf;
      extrabuf = false;
      finalbuf = nullptr;
    }
    buf1 = buf2 = nullptr;
  }
};

#include "Cellular.hpp"
#include "Linear.hpp"

int main(int argc, char *argv[]) {
  srand(time(NULL));

  App app;

  Board<cellular::DayAndNight> board("uBoard"s);
  gl::VertexArray vao;
  gl::Attrib<GL_ARRAY_BUFFER, gl::AttribType::VEC2> attrVertex("vertex"s);
  gl::ShaderProgram<
    gl::VertexShader,
    gl::FragmentShader
  > prog({"shaders/automaton.vert"s, "shaders/automaton.frag"s});

  using ShaderAttrib = decltype(attrVertex);
  using ShaderProgram = decltype(prog);

  app.run(
    // setup function
    [&](auto &w) mutable {
      Logger::Info("init\n");
      // init attribute vertex
      ShaderAttrib::init(attrVertex);
      attrVertex.allocate<GL_STREAM_DRAW>(6, (float[]){
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
      const int factor = -4;
      board.init(w.width(), w.height(), factor);
      board.uSampler.set_id(prog.id());
    },
    // display function
    [&](auto &w) mutable {
      Logger::Info("draw\n");
      // use program
      ShaderProgram::use(prog);
      board.update();
      board.reinit_texture();
      board.set_active(0);
      board.bind();
      board.set_data(0);
      gl::VertexArray::bind(vao);
      glDrawArrays(GL_TRIANGLES, 0, 6); GLERROR
      gl::VertexArray::unbind();
      // unuse
      board.unbind();
      ShaderProgram::unuse();
    },
    // cleanup function
    [&](auto &w) mutable {
      Logger::Info("clear\n");
      board.clear();
      ShaderAttrib::clear(attrVertex);
      gl::VertexArray::clear(vao);
      ShaderProgram::clear(prog);
    }
  );
}
