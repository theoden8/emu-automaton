#pragma once

#include <string>
#include <vector>

#include <Window.hpp>
#include <Renderer.hpp>


class AutomatonApp;

namespace {

typedef enum { RUN_TWOSTATE, RUN_THREESTATE } RUNNER_TYPE;
template <typename AUT> struct use_runner_type {
  static const RUNNER_TYPE rtype = RUN_TWOSTATE;
};

} // namespace


class AutomatonApp {
  Window w;
  const std::string dir;
public:
  AutomatonApp(Window &w, const std::string &dir):
    w(w),
    dir(dir)
  {}

  template <typename AUT>
  void run(AUT aut, int factor);
};

template <typename AUT>
void AutomatonApp::run(AUT aut, int factor) {
  constexpr RUNNER_TYPE RunnerType  = ::use_runner_type<AUT>::rtype;
  AutomatonApp &app = *this;
  app.w.update_size();
  Renderer<AUT, storage_mode::HostBuffer, access_mode::looped> automaton(aut, "grid"s, "no_states"s);
  gl::VertexArray vao;
  gl::Attrib<GL_ARRAY_BUFFER, gl::AttribType::VEC2> attrVertex("vertex"s);
  gl::ShaderProgram<
    gl::VertexShader,
    gl::FragmentShader
  > prog({
    std::string(sys::Path(app.dir) / sys::Path("shaders"s) / sys::Path("aut4.vert"s)),
    std::string(sys::Path(app.dir) / sys::Path("shaders"s) / sys::Path("aut4.frag"s))
  });

  using ShaderAttrib = decltype(attrVertex);
  using ShaderProgram = decltype(prog);

  app.w.run(
    // setup function
    [&](auto &w) mutable -> void {
      Logger::Info("init\n");
      // init attribute vertex
      ShaderAttrib::init(attrVertex);
      std::vector<float> points = {
        1,1, -1,1, -1,-1,
        -1,-1, 1,1, 1,-1,
      };
      attrVertex.allocate<GL_STATIC_DRAW>(points.size() / 2, points.data());
      // add the attribute to the vertex array
      gl::VertexArray::init(vao);
      gl::VertexArray::bind(vao);
      vao.enable(attrVertex);
      vao.set_access(attrVertex, 0, 0);
      gl::VertexArray::unbind();
      // init shader program
      ShaderProgram::init(prog, vao, {"attrVertex"});
      // init texture
      automaton.init(w.width(), w.height(), factor);
      automaton.init_uniforms(prog);
      Logger::Info("init fin\n");
      assert(prog.is_valid());
      prog.print_all();
    },
    // display function
    [&](auto &w) mutable -> bool {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); GLERROR
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
      return true;
    },
    // cleanup function
    [&](auto &w) mutable -> void {
      Logger::Info("clear\n");
      automaton.clear();
      ShaderAttrib::clear(attrVertex);
      gl::VertexArray::clear(vao);
      ShaderProgram::clear(prog);
    }
  );
}
