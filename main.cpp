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
    switch(iface.autType) {
      case InterfaceApp::AutomataType::LINEAR:
      switch(iface.autOption) {
        case InterfaceApp::Linear::RULE30 : app.run(linear::Rule30(), factor); break;
        case InterfaceApp::Linear::RULE54 : app.run(linear::Rule54(), factor); break;
        case InterfaceApp::Linear::RULE90 : app.run(linear::Rule90(), factor); break;
        case InterfaceApp::Linear::RULE110: app.run(linear::Rule110(), factor); break;
        case InterfaceApp::Linear::RULE184: app.run(linear::Rule184(), factor); break;
      }
      break;
      case InterfaceApp::AutomataType::CELLULAR:
      switch(iface.autOption) {
        // 2 states
        case InterfaceApp::Cellular::REPLICATOR:      app.run(cellular::Replicator()     ,factor);break;
        case InterfaceApp::Cellular::FREDKIN:         app.run(cellular::Fredkin()        ,factor);break;
        case InterfaceApp::Cellular::SEEDS:           app.run(cellular::Seeds()          ,factor);break;
        case InterfaceApp::Cellular::LIVEORDIE:       app.run(cellular::LiveOrDie()      ,factor);break;
        case InterfaceApp::Cellular::FLOCK:           app.run(cellular::Flock()          ,factor);break;
        case InterfaceApp::Cellular::MAZECTRIC:       app.run(cellular::Mazectric()      ,factor);break;
        case InterfaceApp::Cellular::MAZE:            app.run(cellular::Maze()           ,factor);break;
        case InterfaceApp::Cellular::MAZECTRICMICE:   app.run(cellular::MazectricMice()  ,factor);break;
        case InterfaceApp::Cellular::MAZEMICE:        app.run(cellular::MazeMice()       ,factor);break;
        case InterfaceApp::Cellular::GAMEOFLIFE:      app.run(cellular::GameOfLife()     ,factor);break;
        case InterfaceApp::Cellular::EIGHTLIFE:       app.run(cellular::EightLife()      ,factor);break;
        case InterfaceApp::Cellular::LONGLIFE:        app.run(cellular::LongLife()       ,factor);break;
        case InterfaceApp::Cellular::TXT:             app.run(cellular::TxT()            ,factor);break;
        case InterfaceApp::Cellular::HIGHLIFE:        app.run(cellular::HighLife()       ,factor);break;
        case InterfaceApp::Cellular::MOVE:            app.run(cellular::Move()           ,factor);break;
        case InterfaceApp::Cellular::STAINS:          app.run(cellular::Stains()         ,factor);break;
        case InterfaceApp::Cellular::DAYANDNIGHT:     app.run(cellular::DayAndNight()    ,factor);break;
        case InterfaceApp::Cellular::ANNEAL:          app.run(cellular::Anneal()         ,factor);break;
        case InterfaceApp::Cellular::DRYLIFE:         app.run(cellular::DryLife()        ,factor);break;
        case InterfaceApp::Cellular::PEDESTRLIFE:     app.run(cellular::PedestrLife()    ,factor);break;
        case InterfaceApp::Cellular::AMOEBA:          app.run(cellular::Amoeba()         ,factor);break;
        case InterfaceApp::Cellular::DIAMOEBA:        app.run(cellular::Diamoeba()       ,factor);break;
        case InterfaceApp::Cellular::LANGTONSANT:     app.run(cellular::LangtonsAnt()    ,factor);break;
        // 3 states
        case InterfaceApp::Cellular::BRIANSBRAIN:     app.run(cellular::BriansBrain()    ,factor);break;
        case InterfaceApp::Cellular::BRAIN6:          app.run(cellular::Brain6()         ,factor);break;
        case InterfaceApp::Cellular::FROGS:           app.run(cellular::Frogs()          ,factor);break;
        case InterfaceApp::Cellular::LINES:           app.run(cellular::Lines()          ,factor);break;
        // 4 states
        case InterfaceApp::Cellular::CATERPILLARS:    app.run(cellular::Caterpillars()   ,factor);break;
        case InterfaceApp::Cellular::ORTHOGO:         app.run(cellular::OrthoGo()        ,factor);break;
        case InterfaceApp::Cellular::SEDIMENTAL:      app.run(cellular::SediMental()     ,factor);break;
        case InterfaceApp::Cellular::STARWARS:        app.run(cellular::StarWars()       ,factor);break;
        case InterfaceApp::Cellular::WIREWORLD:       app.run(cellular::Wireworld()      ,factor);break;
        // 5 states
        case InterfaceApp::Cellular::BANNERS:         app.run(cellular::Banners()        ,factor);break;
        case InterfaceApp::Cellular::GLISSERGY:       app.run(cellular::Glissergy()      ,factor);break;
        case InterfaceApp::Cellular::SPIRALS:         app.run(cellular::Spirals()        ,factor);break;
        case InterfaceApp::Cellular::TRANSERS:        app.run(cellular::Transers()       ,factor);break;
        case InterfaceApp::Cellular::WANDERERS:       app.run(cellular::Wanderers()      ,factor);break;
        // 6 states
        case InterfaceApp::Cellular::CHENILLE:        app.run(cellular::Chenille()       ,factor);break;
        case InterfaceApp::Cellular::FROZENSPIRALS:   app.run(cellular::FrozenSpirals()  ,factor);break;
        case InterfaceApp::Cellular::LIVINGONTHEEDGE: app.run(cellular::LivingOnTheEdge(),factor);break;
        case InterfaceApp::Cellular::PRAIRIEONFIRE:   app.run(cellular::PrairieOnFire()  ,factor);break;
        case InterfaceApp::Cellular::RAKE:            app.run(cellular::Rake()           ,factor);break;
        case InterfaceApp::Cellular::SNAKE:           app.run(cellular::Snake()          ,factor);break;
        case InterfaceApp::Cellular::SOFTFREEZE:      app.run(cellular::SoftFreeze()     ,factor);break;
        case InterfaceApp::Cellular::STICKS:          app.run(cellular::Sticks()         ,factor);break;
        case InterfaceApp::Cellular::WORMS:           app.run(cellular::Worms()          ,factor);break;
        // 7 states
        case InterfaceApp::Cellular::GLISSERATI:      app.run(cellular::Glisserati()     ,factor);break;
        // 8 states
        case InterfaceApp::Cellular::BELZHAB:         app.run(cellular::BelZhab()        ,factor);break;
        case InterfaceApp::Cellular::CIRCUITGENESIS:  app.run(cellular::CircuitGenesis() ,factor);break;
        case InterfaceApp::Cellular::COOTIES:         app.run(cellular::Cooties()        ,factor);break;
        case InterfaceApp::Cellular::FLAMINGSTARBOWS: app.run(cellular::FlamingStarbows(),factor);break;
        case InterfaceApp::Cellular::LAVA:            app.run(cellular::Lava()           ,factor);break;
        case InterfaceApp::Cellular::METEORGUNS:      app.run(cellular::MeteorGuns()     ,factor);break;
        case InterfaceApp::Cellular::SWIRL:           app.run(cellular::Swirl()          ,factor);break;
        // 9 states
        case InterfaceApp::Cellular::BURST:           app.run(cellular::Burst()          ,factor);break;
        case InterfaceApp::Cellular::BURST2:          app.run(cellular::Burst2()         ,factor);break;
        // 16 states
        case InterfaceApp::Cellular::XTASY:           app.run(cellular::Xtasy()          ,factor);break;
        // 18 states
        case InterfaceApp::Cellular::EBBANDFLOW:      app.run(cellular::EbbAndFlow()     ,factor);break;
        case InterfaceApp::Cellular::EBBANDFLOW2:     app.run(cellular::EbbAndFlow2()    ,factor);break;
        // 21 states
        case InterfaceApp::Cellular::FIREWORKS:       app.run(cellular::Fireworks()      ,factor);break;
        // 24 states
        case InterfaceApp::Cellular::BLOOMERANG:      app.run(cellular::Bloomerang()     ,factor);break;
        // 25 states
        case InterfaceApp::Cellular::FADERS:          app.run(cellular::Faders()         ,factor);break;
        case InterfaceApp::Cellular::NOVA:            app.run(cellular::Nova()           ,factor);break;
        case InterfaceApp::Cellular::BOMBERS:         app.run(cellular::Bombers()        ,factor);break;
        // 48 states
        case InterfaceApp::Cellular::THRILLGRILL:     app.run(cellular::Faders()         ,factor);break;
      }
      break;
      case InterfaceApp::AutomataType::PROBABILISTIC:
      switch (iface.autOption) {
        case InterfaceApp::Probabilistic::ISING: app.run(probabilistic::Ising(iface.isingBeta), factor);break;
      }
      break;
    }
    shouldQuit = true;
  }

  w.quit();

  Logger::Close();
}
