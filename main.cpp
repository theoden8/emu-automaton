#include <cstdint>
#include <cctype>

#include <string>

#include <Logger.hpp>
#include <Debug.hpp>
#include <File.hpp>

#include <Window.hpp>
#include <InterfaceApp.hpp>
#include <AutomatonApp.hpp>

using namespace std::literals::string_literals;


int main(int argc, char *argv[]) {
  srand(time(NULL));
  Logger::Setup("app.log");
  /* Logger::MirrorLog(stderr); */

  Window w;
  w.init();

  const std::string cwd = sys::get_cwd();
  Logger::Info("cwd: '%s'\n", cwd.c_str());
  const std::string dir = sys::get_executable_directory(argc, argv);
  Logger::Info("dir '%s'\n", dir.c_str());

  bool shouldQuit = false;
  while(!shouldQuit) {
    InterfaceApp iface(w, dir);
    iface.run();
    AutOptions opts = (AutOptions){
      .factor=iface.factor,
      .force_cpu=bool(iface.force_cpu),
    };
    shouldQuit = iface.shouldQuit;
    if(shouldQuit) {
      break;
    }
    const int factor = iface.factor;

    AutomatonApp app(w, dir);
    switch(iface.autType) {
      case InterfaceApp::AutomataType::LINEAR:
      switch(iface.autOption) {
        case InterfaceApp::Linear::RULE30 : app.run(linear::Rule30(),  opts); break;
        case InterfaceApp::Linear::RULE54 : app.run(linear::Rule54(),  opts); break;
        case InterfaceApp::Linear::RULE90 : app.run(linear::Rule90(),  opts); break;
        case InterfaceApp::Linear::RULE110: app.run(linear::Rule110(), opts); break;
        case InterfaceApp::Linear::RULE184: app.run(linear::Rule184(), opts); break;
      }
      break;
      case InterfaceApp::AutomataType::CELLULAR:
      switch(iface.autOption) {
        // 2 states
        case InterfaceApp::Cellular::REPLICATOR:      app.run(cellular::Replicator()     ,opts);break;
        case InterfaceApp::Cellular::FREDKIN:         app.run(cellular::Fredkin()        ,opts);break;
        case InterfaceApp::Cellular::SEEDS:           app.run(cellular::Seeds()          ,opts);break;
        case InterfaceApp::Cellular::LIVEORDIE:       app.run(cellular::LiveOrDie()      ,opts);break;
        case InterfaceApp::Cellular::FLOCK:           app.run(cellular::Flock()          ,opts);break;
        case InterfaceApp::Cellular::MAZECTRIC:       app.run(cellular::Mazectric()      ,opts);break;
        case InterfaceApp::Cellular::MAZE:            app.run(cellular::Maze()           ,opts);break;
        case InterfaceApp::Cellular::MAZECTRICMICE:   app.run(cellular::MazectricMice()  ,opts);break;
        case InterfaceApp::Cellular::MAZEMICE:        app.run(cellular::MazeMice()       ,opts);break;
        case InterfaceApp::Cellular::GAMEOFLIFE:      app.run(cellular::GameOfLife()     ,opts);break;
        case InterfaceApp::Cellular::EIGHTLIFE:       app.run(cellular::EightLife()      ,opts);break;
        case InterfaceApp::Cellular::LONGLIFE:        app.run(cellular::LongLife()       ,opts);break;
        case InterfaceApp::Cellular::TXT:             app.run(cellular::TxT()            ,opts);break;
        case InterfaceApp::Cellular::HIGHLIFE:        app.run(cellular::HighLife()       ,opts);break;
        case InterfaceApp::Cellular::MOVE:            app.run(cellular::Move()           ,opts);break;
        case InterfaceApp::Cellular::STAINS:          app.run(cellular::Stains()         ,opts);break;
        case InterfaceApp::Cellular::DAYANDNIGHT:     app.run(cellular::DayAndNight()    ,opts);break;
        case InterfaceApp::Cellular::ANNEAL:          app.run(cellular::Anneal()         ,opts);break;
        case InterfaceApp::Cellular::DRYLIFE:         app.run(cellular::DryLife()        ,opts);break;
        case InterfaceApp::Cellular::PEDESTRLIFE:     app.run(cellular::PedestrLife()    ,opts);break;
        case InterfaceApp::Cellular::AMOEBA:          app.run(cellular::Amoeba()         ,opts);break;
        case InterfaceApp::Cellular::DIAMOEBA:        app.run(cellular::Diamoeba()       ,opts);break;
        case InterfaceApp::Cellular::LANGTONSANT:     app.run(cellular::LangtonsAnt()    ,opts);break;
        // 3 states
        case InterfaceApp::Cellular::BRIANSBRAIN:     app.run(cellular::BriansBrain()    ,opts);break;
        case InterfaceApp::Cellular::BRAIN6:          app.run(cellular::Brain6()         ,opts);break;
        case InterfaceApp::Cellular::FROGS:           app.run(cellular::Frogs()          ,opts);break;
        case InterfaceApp::Cellular::LINES:           app.run(cellular::Lines()          ,opts);break;
        // 4 states
        case InterfaceApp::Cellular::CATERPILLARS:    app.run(cellular::Caterpillars()   ,opts);break;
        case InterfaceApp::Cellular::ORTHOGO:         app.run(cellular::OrthoGo()        ,opts);break;
        case InterfaceApp::Cellular::SEDIMENTAL:      app.run(cellular::SediMental()     ,opts);break;
        case InterfaceApp::Cellular::STARWARS:        app.run(cellular::StarWars()       ,opts);break;
        case InterfaceApp::Cellular::WIREWORLD:       app.run(cellular::Wireworld()      ,opts);break;
        // 5 states
        case InterfaceApp::Cellular::BANNERS:         app.run(cellular::Banners()        ,opts);break;
        case InterfaceApp::Cellular::GLISSERGY:       app.run(cellular::Glissergy()      ,opts);break;
        case InterfaceApp::Cellular::SPIRALS:         app.run(cellular::Spirals()        ,opts);break;
        case InterfaceApp::Cellular::TRANSERS:        app.run(cellular::Transers()       ,opts);break;
        case InterfaceApp::Cellular::WANDERERS:       app.run(cellular::Wanderers()      ,opts);break;
        // 6 states
        case InterfaceApp::Cellular::CHENILLE:        app.run(cellular::Chenille()       ,opts);break;
        case InterfaceApp::Cellular::FROZENSPIRALS:   app.run(cellular::FrozenSpirals()  ,opts);break;
        case InterfaceApp::Cellular::LIVINGONTHEEDGE: app.run(cellular::LivingOnTheEdge(),opts);break;
        case InterfaceApp::Cellular::PRAIRIEONFIRE:   app.run(cellular::PrairieOnFire()  ,opts);break;
        case InterfaceApp::Cellular::RAKE:            app.run(cellular::Rake()           ,opts);break;
        case InterfaceApp::Cellular::SNAKE:           app.run(cellular::Snake()          ,opts);break;
        case InterfaceApp::Cellular::SOFTFREEZE:      app.run(cellular::SoftFreeze()     ,opts);break;
        case InterfaceApp::Cellular::STICKS:          app.run(cellular::Sticks()         ,opts);break;
        case InterfaceApp::Cellular::WORMS:           app.run(cellular::Worms()          ,opts);break;
        // 7 states
        case InterfaceApp::Cellular::GLISSERATI:      app.run(cellular::Glisserati()     ,opts);break;
        // 8 states
        case InterfaceApp::Cellular::BELZHAB:         app.run(cellular::BelZhab()        ,opts);break;
        case InterfaceApp::Cellular::CIRCUITGENESIS:  app.run(cellular::CircuitGenesis() ,opts);break;
        case InterfaceApp::Cellular::COOTIES:         app.run(cellular::Cooties()        ,opts);break;
        case InterfaceApp::Cellular::FLAMINGSTARBOWS: app.run(cellular::FlamingStarbows(),opts);break;
        case InterfaceApp::Cellular::LAVA:            app.run(cellular::Lava()           ,opts);break;
        case InterfaceApp::Cellular::METEORGUNS:      app.run(cellular::MeteorGuns()     ,opts);break;
        case InterfaceApp::Cellular::SWIRL:           app.run(cellular::Swirl()          ,opts);break;
        // 9 states
        case InterfaceApp::Cellular::BURST:           app.run(cellular::Burst()          ,opts);break;
        case InterfaceApp::Cellular::BURST2:          app.run(cellular::Burst2()         ,opts);break;
        // 16 states
        case InterfaceApp::Cellular::XTASY:           app.run(cellular::Xtasy()          ,opts);break;
        // 18 states
        case InterfaceApp::Cellular::EBBANDFLOW:      app.run(cellular::EbbAndFlow()     ,opts);break;
        case InterfaceApp::Cellular::EBBANDFLOW2:     app.run(cellular::EbbAndFlow2()    ,opts);break;
        // 21 states
        case InterfaceApp::Cellular::FIREWORKS:       app.run(cellular::Fireworks()      ,opts);break;
        // 24 states
        case InterfaceApp::Cellular::BLOOMERANG:      app.run(cellular::Bloomerang()     ,opts);break;
        // 25 states
        case InterfaceApp::Cellular::FADERS:          app.run(cellular::Faders()         ,opts);break;
        case InterfaceApp::Cellular::NOVA:            app.run(cellular::Nova()           ,opts);break;
        case InterfaceApp::Cellular::BOMBERS:         app.run(cellular::Bombers()        ,opts);break;
        // 48 states
        case InterfaceApp::Cellular::THRILLGRILL:     app.run(cellular::Faders()         ,opts);break;
      }
      break;
      case InterfaceApp::AutomataType::PROBABILISTIC:
      switch (iface.autOption) {
        case InterfaceApp::Probabilistic::ISING: app.run(probabilistic::Ising(iface.isingBeta), opts);break;
      }
      break;
    }
    shouldQuit = true;
  }

  w.quit();

  Logger::Close();
}
