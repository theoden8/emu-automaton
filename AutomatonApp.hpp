#pragma once

#include <Window.hpp>
#include <Renderer.hpp>


class AutomatonApp {
  Window w;
public:
  AutomatonApp(Window &w):
    w(w)
  {}

  template <typename AUT>
  void run(int factor) {
    Renderer<AUT, storage_mode::HostBuffer, access_mode::looped> automaton("grid"s);
    gl::VertexArray vao;
    gl::Attrib<GL_ARRAY_BUFFER, gl::AttribType::VEC2> attrVertex("vertex"s);
    gl::ShaderProgram<
      gl::VertexShader,
      gl::FragmentShader
    > prog({"shaders/aut4.vert"s, "shaders/aut4.frag"s});

    using ShaderAttrib = decltype(attrVertex);
    using ShaderProgram = decltype(prog);

    w.run(
      // setup function
      [&](auto &w) mutable -> void {
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
        automaton.init(w.width(), w.height(), factor);
        automaton.uSampler.set_id(prog.id());
        Logger::Info("init fin\n");
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
};
