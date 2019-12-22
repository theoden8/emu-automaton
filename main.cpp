#include <cstdint>
#include <cctype>

#include <string>

#include <Logger.hpp>
#include <Debug.hpp>
#include <File.hpp>

#include <Window.hpp>
#include <InterfaceApp.hpp>
#include <AutomatonApp.hpp>

#if !defined(_POSIX_VERSION)
#include <direct.h>
#endif

using namespace std::literals::string_literals;


int main(int argc, char *argv[]) {
  srand(time(NULL));
  Logger::Setup("app.log");
  /* Logger::MirrorLog(stderr); */

  Window w;
  w.init();

#if defined(_POSIX_VERSION)
  char buf[PATH_MAX + 1];
  getcwd(buf, PATH_MAX);
  Logger::Info("cwd: '%s'\n", buf);
#else
  char buf[MAX_PATH + 1];
  _getcwd(buf, MAX_PATH);
  Logger::Info("cwd: '%s'\n", buf);
#endif
  const std::string dir = sys::get_executable_directory(argc, argv);
  Logger::Info("dir '%s'\n", dir.c_str());

  bool shouldQuit = false;
  while(!shouldQuit) {
    InterfaceApp iface(w, dir);
    iface.run();
    shouldQuit = iface.shouldQuit;
    if(shouldQuit) {
      break;
    }
    const int factor = iface.factor;

    AutomatonApp app(w, dir);
    if(iface.autType == InterfaceApp::LINEAR) {
      switch(iface.autOption) {
        case InterfaceApp::Linear::RULE30 : app.run<linear::Rule30 >(factor); break;
        case InterfaceApp::Linear::RULE54 : app.run<linear::Rule54 >(factor); break;
        case InterfaceApp::Linear::RULE90 : app.run<linear::Rule90 >(factor); break;
        case InterfaceApp::Linear::RULE110: app.run<linear::Rule110>(factor); break;
        case InterfaceApp::Linear::RULE184: app.run<linear::Rule184>(factor); break;
      }
    } else if(iface.autType == InterfaceApp::CELLULAR) {
      switch(iface.autOption) {
        case InterfaceApp::Cellular::REPLICATOR:    app.run<cellular::Replicator >(factor);break;
        case InterfaceApp::Cellular::FREDKIN:       app.run<cellular::Fredkin    >(factor);break;
        case InterfaceApp::Cellular::SEEDS:         app.run<cellular::Seeds      >(factor);break;
        case InterfaceApp::Cellular::LIVEORDIE:     app.run<cellular::LiveOrDie  >(factor);break;
        case InterfaceApp::Cellular::FLOCK:         app.run<cellular::Flock      >(factor);break;
        case InterfaceApp::Cellular::MAZECTRIC:     app.run<cellular::Mazectric  >(factor);break;
        case InterfaceApp::Cellular::MAZE:          app.run<cellular::Maze       >(factor);break;
        case InterfaceApp::Cellular::GAMEOFLIFE:    app.run<cellular::GameOfLife >(factor);break;
        case InterfaceApp::Cellular::EIGHTLIFE:     app.run<cellular::EightLife  >(factor);break;
        case InterfaceApp::Cellular::LONGLIFE:      app.run<cellular::LongLife   >(factor);break;
        case InterfaceApp::Cellular::TXT:           app.run<cellular::TxT        >(factor);break;
        case InterfaceApp::Cellular::HIGHLIFE:      app.run<cellular::HighLife   >(factor);break;
        case InterfaceApp::Cellular::MOVE:          app.run<cellular::Move       >(factor);break;
        case InterfaceApp::Cellular::STAINS:        app.run<cellular::Stains     >(factor);break;
        case InterfaceApp::Cellular::DAYANDNIGHT:   app.run<cellular::DayAndNight>(factor);break;
        case InterfaceApp::Cellular::DRYLIFE:       app.run<cellular::DryLife    >(factor);break;
        case InterfaceApp::Cellular::PEDESTRLIFE:   app.run<cellular::PedestrLife>(factor);break;
      }
    }
    shouldQuit = true;
  }

  w.quit();

  Logger::Close();
}
