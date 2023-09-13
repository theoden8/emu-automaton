#pragma once


#include <incgraphics.h>

#include <Logger.hpp>
#include <Debug.hpp>


GLFWwindow *g_window = nullptr;

/* GLvoid debug_callback(GLenum source, GLenum type, GLuint id, */
/*                               GLuint severity, GLsizei length, */
/*                               const GLchar *message, const GLvoid *userParam) */
/* { */
/*   Logger::Info("%s\n", message); */
/* } */

class Window;

Window *g_current_window = nullptr;
void keypress_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

class Window {
protected:
  const GLFWvidmode *vidmode = nullptr;
  size_t width_, height_;

  bool try_create_window(int gl_major, int gl_minor) {
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gl_major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gl_minor);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL

    g_window = window = glfwCreateWindow(width(), height(), "automata", nullptr, nullptr);
    return (g_window != NULL);
  }

  void init_glfw() {
    int rc = glfwInit();
    ASSERT(rc == 1);

    vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    ASSERT(vidmode != nullptr);
    width_ = vidmode->width;
    height_ = vidmode->height;
    width_ = 800, height_ = 800;
    /* width_ = height_ = std::min(width_, height_); */

    bool has_4_3 = try_create_window(4, 3);
    gl_support_compute_shaders = has_4_3;
    if(!has_4_3) {
      Logger::Info("version 4.3 not supported\n");
      bool has_3_3 = try_create_window(3, 3);
      ASSERT(has_3_3);
    }

    glfwMakeContextCurrent(window); GLERROR
    glfwSetKeyCallback(window, keypress_callback); GLERROR

    int glfw_major, glfw_minor, glfw_rev;
    glfwGetVersion(&glfw_major, &glfw_minor, &glfw_rev);
    int gl_major, gl_minor;
    glGetIntegerv(GL_MAJOR_VERSION, &gl_major);
    glGetIntegerv(GL_MINOR_VERSION, &gl_minor);
    Logger::Info("glfw version %d.%d.%d\n", glfw_major, glfw_minor, glfw_rev);
    Logger::Info("gl version %d.%d\n", gl_major, gl_minor);
    Logger::Info("initialized glfw\n");
  }
  void init_controls() {
    // ensure we can capture the escape key
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE); GLERROR
    /* glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); GLERROR */
    Logger::Info("initialized controls\n");
  }
public:
  GLFWwindow *window = nullptr;
  bool gl_support_compute_shaders = false;
  Window():
    width_(0),
    height_(0)
  {}
  inline size_t width() const { return width_; }
  inline size_t height() const { return height_; }
  bool esc_triggered = false;
  void keyboard_event(int key, int scancode, int action, int mods) {
    if(action == GLFW_PRESS) {
      if(key == GLFW_KEY_ESCAPE && !esc_triggered) {
        Logger::Info("registered escape press\n");
        esc_triggered = true;
      }
    }
  }
  void init() {
    init_glfw();
    init_controls();
    /* glDebugMessageCallbackARB(&debug_callback, nullptr); GLERROR */
  }
  void update_size() {
    int w, h;
    glfwGetWindowSize(window, &w, &h);
    width_=w, height_=h;
  }
  template <typename SF, typename DF, typename CF>
  void run(SF &&setupfunc, DF &&dispfunc, CF &&cleanupfunc) {
    setupfunc(*this);
    glfwSwapInterval(1); GLERROR
    bool shouldClose = false;
    while(!glfwWindowShouldClose(window) && !shouldClose && !esc_triggered) {
      g_current_window = this;
      shouldClose = !dispfunc(*this);
      glfwPollEvents(); GLERROR
      glfwSwapBuffers(window); GLERROR
    }
    esc_triggered = false;
    cleanupfunc(*this);
  }
  void quit() {
    g_current_window = nullptr;
    glfwDestroyWindow(window); GLERROR
    glfwTerminate(); GLERROR
  }
  ~Window() {
    g_window = nullptr;
  }
};

void keypress_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if(g_current_window != nullptr) {
    g_current_window->keyboard_event(key, scancode, action, mods);
  }
}
