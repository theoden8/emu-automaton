#pragma once

#include <string>
#include <vector>

#include <Window.hpp>
#include <Renderer.hpp>


class AutomatonApp;

namespace {

template <typename AUT> struct use_storage_mode {
  static constexpr storage_mode smode = storage_mode::HOSTBUFFER;
};

template <>
struct use_storage_mode<ca::BSC> {
  static constexpr storage_mode smode = storage_mode::TEXTURES;
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

  template <storage_mode StorageMode, typename AUT>
  void run_with_storage_mode(AUT &&aut, const AutOptions &opts);

  template <typename AUT>
  void run(AUT &&aut, const AutOptions &opts) {
    AutomatonApp &app = (*this);
    constexpr storage_mode storage_mode_recommended = ::use_storage_mode<AUT>::smode;
    if constexpr(storage_mode_recommended == storage_mode::HOSTBUFFER) {
      run_with_storage_mode<storage_mode::HOSTBUFFER>(std::forward<AUT>(aut), opts);
    } else {
      if(app.w.gl_support_compute_shaders && !opts.force_cpu) {
        run_with_storage_mode<storage_mode_recommended>(std::forward<AUT>(aut), opts);
      } else {
        run_with_storage_mode<storage_mode::HOSTBUFFER>(std::forward<AUT>(aut), opts);
      }
    }
  }
};

template <storage_mode StorageMode, typename AUT>
void AutomatonApp::run_with_storage_mode(AUT &&aut, const AutOptions &opts) {
  AutomatonApp &app = (*this);
  app.w.update_size();
  Logger::Info("automaton app\n");
  Renderer<AUT, StorageMode, access_mode::looped> automaton(aut, app.dir);
  Logger::Info("using storage mode %s\n", (automaton.get_storage_mode() == storage_mode::HOSTBUFFER) ? "host" : "textures");

  bool w_ret = app.w.run(
    // setup function
    [&](auto &w) mutable -> void {
      Logger::Info("init\n");
      automaton.init_renderer(w, opts.factor);
      Logger::Info("init fin\n");
    },
    // display function
    [&](auto &w) mutable -> bool {
      automaton.update_state();
//      constexpr int ms = 1e4;
//      usleep(50*ms);
      automaton.render(0);
      return true;
    },
    // cleanup function
    [&](auto &w) mutable -> void {
      Logger::Info("clear\n");
      automaton.clear();
    }
  );
}
