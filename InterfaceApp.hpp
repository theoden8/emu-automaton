#pragma once

#include <cstring>
#include <vector>

#include <String.hpp>
#include <Window.hpp>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#include <Nuklear/nuklear.h>
#include <Nuklear/demo/glfw_opengl3/nuklear_glfw_gl3.h>
#include <Nuklear/demo/common/style.c>

#define MAX_VERTEX_BUFFER (512 * 1024)
#define MAX_ELEMENT_BUFFER (128 * 1024)

struct InterfaceApp {
  Window &w;
  struct nk_glfw nkglfw = {0};
  const std::vector<int> factors = {-4, -3, -2, 1, 2, 4, 8, 16, 32};
  int factor = 2;

  struct nk_context *ctx = nullptr;
  struct nk_color background;

  enum Linear : int {
    RULE30,
    RULE54,
    RULE90,
    RULE110,
    RULE184,
    NO_LINEAR
  };

  enum Cellular : int {
    // 2 states
    REPLICATOR,
    FREDKIN,
    SEEDS,
    LIVEORDIE,
    FLOCK,
    MAZECTRIC,
    MAZE,
    MAZECTRICMICE,
    MAZEMICE,
    GAMEOFLIFE,
    EIGHTLIFE,
    LONGLIFE,
    TXT,
    HIGHLIFE,
    MOVE,
    STAINS,
    DAYANDNIGHT,
    ANNEAL,
    DRYLIFE,
    PEDESTRLIFE,
    AMOEBA,
    DIAMOEBA,
    LANGTONSANT,
    // 3 states
    BRIANSBRAIN,
    BRAIN6,
    FROGS,
    LINES,
    // 4 states
    CATERPILLARS,
    ORTHOGO,
    SEDIMENTAL,
    STARWARS,
    WIREWORLD,
    // 5 states
    BANNERS,
    GLISSERGY,
    SPIRALS,
    TRANSERS,
    WANDERERS,
    // 6 states
    CHENILLE,
    FROZENSPIRALS,
    LIVINGONTHEEDGE,
    PRAIRIEONFIRE,
    RAKE,
    SNAKE,
    SOFTFREEZE,
    STICKS,
    WORMS,
    // 7 states
    GLISSERATI,
    // 8 states
    BELZHAB,
    CIRCUITGENESIS,
    COOTIES,
    FLAMINGSTARBOWS,
    LAVA,
    METEORGUNS,
    SWIRL,
    // 9 states
    BURST,
    BURST2,
    // 16 states
    XTASY,
    // 18 states
    EBBANDFLOW,
    EBBANDFLOW2,
    // 21 states
    FIREWORKS,
    // 24 states
    BLOOMERANG,
    // 25 states
    FADERS,
    NOVA,
    BOMBERS,
    // 48 states
    THRILLGRILL,
    //
    NO_CELLULAR
  };

  enum Probabilistic : int {
    // 2 states
    ISING,
    NO_PROBABILISTIC
  };

  enum AutomataType : int {
    LINEAR, CELLULAR, PROBABILISTIC, NO_AUTOMATA_TYPES
  };
  const sys::Path root_path;

  int autType = CELLULAR;
  int autStates = 2;
  int autOption = Cellular::DAYANDNIGHT;
  float isingBeta = .5;
  bool finished = false;
  bool shouldQuit = false;

  InterfaceApp(Window &w, const std::string &dir):
    w(w), root_path(dir)
  {}

  void run() {
    w.update_size();
    Logger::Info("interface app\n");
    w.run(
      // setup
      [&](auto &w) mutable -> void {
        ctx = nk_glfw3_init(&nkglfw, g_window, NK_GLFW3_INSTALL_CALLBACKS);
        /* Logger::Info("ctx %p\n", ctx); */
        {
          struct nk_font_atlas *atlas;
          nk_glfw3_font_stash_begin(&nkglfw, &atlas);
          const sys::Path font_path = root_path / sys::Path("resources"s) / sys::Path("DroidSans.ttf"s);
          Logger::Info("loading font from %s\n", std::string(font_path).c_str());
          struct nk_font *droid = nk_font_atlas_add_from_file(atlas, std::string(font_path).c_str(), 24, 0);
          /*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/Roboto-Regular.ttf", 14, 0);*/
          /*struct nk_font *future = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/kenvector_future_thin.ttf", 13, 0);*/
          /*struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/ProggyClean.ttf", 12, 0);*/
          /*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/ProggyTiny.ttf", 10, 0);*/
          /*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/Cousine-Regular.ttf", 13, 0);*/
          nk_glfw3_font_stash_end(&nkglfw);
          /* nk_style_load_all_cursors(ctx, atlas->cursors); */
          nk_style_set_font(ctx, &droid->handle);
        }
        background = nk_rgb(136,181,216);
      },
      // display
      [&](auto &w) mutable -> bool {
        nk_glfw3_new_frame(&nkglfw);
        if (nk_begin(ctx, "Automaton Menu", nk_rect(25, 25, w.width()-50, w.height()-50),
              NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
              NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
        {
          nk_layout_row_dynamic(ctx, 30, 1);
          nk_label(ctx, "Automaton type", NK_TEXT_CENTERED);
          nk_layout_row_dynamic(ctx, 30, NO_AUTOMATA_TYPES);
          /* ASSERT(nk_group_begin(ctx, "Type", NK_WINDOW_BORDER)); */
          if(nk_option_label(ctx, "Linear", autType == AutomataType::LINEAR)) {
            if(autType != AutomataType::LINEAR) {
              autOption = 0;
            }
            autType = AutomataType::LINEAR;
          }
          if(nk_option_label(ctx, "Cellular", autType == AutomataType::CELLULAR)) {
            if(autType != AutomataType::CELLULAR) {
              autOption = 0;
            }
            autType = AutomataType::CELLULAR;
          }
          if(nk_option_label(ctx, "Probabilistic", autType == AutomataType::PROBABILISTIC)) {
            if(autType != AutomataType::PROBABILISTIC) {
              autOption = 0;
            }
            autType = AutomataType::PROBABILISTIC;
          }
          /* nk_group_end(ctx); */
          {
            char aut_states_s[256];
            snprintf(aut_states_s, sizeof(aut_states_s), "States %d", autStates);
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_label(ctx, aut_states_s, NK_TEXT_LEFT);
            nk_slider_int(ctx, 2, &autStates, 48, 1);
          }

          /* ASSERT(nk_group_begin(ctx, "Automaton", NK_WINDOW_BORDER)); */
          nk_layout_row_dynamic(ctx, 30, 1);
          nk_label(ctx, "Automaton", NK_TEXT_CENTERED);
          if(autType == AutomataType::LINEAR) {
            if(autStates == 2) {
              nk_layout_row_dynamic(ctx, 30, 5);
              if (nk_option_label(ctx, "Rule 30"  ,autOption == Linear::RULE30 )) autOption = Linear::RULE30;
              if (nk_option_label(ctx, "Rule 54"  ,autOption == Linear::RULE54 )) autOption = Linear::RULE54;
              if (nk_option_label(ctx, "Rule 90"  ,autOption == Linear::RULE90 )) autOption = Linear::RULE90;
              if (nk_option_label(ctx, "Rule 110" ,autOption == Linear::RULE110)) autOption = Linear::RULE110;
              if (nk_option_label(ctx, "Rule 184" ,autOption == Linear::RULE184)) autOption = Linear::RULE184;
            }
          } else if(autType == AutomataType::CELLULAR) {
            switch(autStates) {
              case 2:
                nk_layout_row_dynamic(ctx, 30, 4);
                if (nk_option_label(ctx, "Replicator"     ,autOption == Cellular::REPLICATOR     )) autOption = Cellular::REPLICATOR;
                if (nk_option_label(ctx, "Fredkin"        ,autOption == Cellular::FREDKIN        )) autOption = Cellular::FREDKIN;
                if (nk_option_label(ctx, "Seeds"          ,autOption == Cellular::SEEDS          )) autOption = Cellular::SEEDS;
                if (nk_option_label(ctx, "Live Or Die"    ,autOption == Cellular::LIVEORDIE      )) autOption = Cellular::LIVEORDIE;
                nk_layout_row_dynamic(ctx, 30, 4);
                if (nk_option_label(ctx, "Flock"          ,autOption == Cellular::FLOCK          )) autOption = Cellular::FLOCK;
                if (nk_option_label(ctx, "Game Of Life"   ,autOption == Cellular::GAMEOFLIFE     )) autOption = Cellular::GAMEOFLIFE;
                if (nk_option_label(ctx, "Mazectric"      ,autOption == Cellular::MAZECTRIC      )) autOption = Cellular::MAZECTRIC;
                if (nk_option_label(ctx, "Maze"           ,autOption == Cellular::MAZE           )) autOption = Cellular::MAZE;
                nk_layout_row_dynamic(ctx, 30, 4);
                if (nk_option_label(ctx, "MazectricMice"  ,autOption == Cellular::MAZECTRICMICE  )) autOption = Cellular::MAZECTRICMICE;
                if (nk_option_label(ctx, "MazeMice"       ,autOption == Cellular::MAZEMICE       )) autOption = Cellular::MAZEMICE;
                if (nk_option_label(ctx, "Eight Life"     ,autOption == Cellular::EIGHTLIFE      )) autOption = Cellular::EIGHTLIFE;
                if (nk_option_label(ctx, "Long Life"      ,autOption == Cellular::LONGLIFE       )) autOption = Cellular::LONGLIFE;
                nk_layout_row_dynamic(ctx, 30, 4);
                if (nk_option_label(ctx, "2x2"            ,autOption == Cellular::TXT            )) autOption = Cellular::TXT;
                if (nk_option_label(ctx, "High Life"      ,autOption == Cellular::HIGHLIFE       )) autOption = Cellular::HIGHLIFE;
                if (nk_option_label(ctx, "Move"           ,autOption == Cellular::MOVE           )) autOption = Cellular::MOVE;
                if (nk_option_label(ctx, "Stains"         ,autOption == Cellular::STAINS         )) autOption = Cellular::STAINS;
                nk_layout_row_dynamic(ctx, 30, 4);
                if (nk_option_label(ctx, "Day And Night"  ,autOption == Cellular::DAYANDNIGHT    )) autOption = Cellular::DAYANDNIGHT;
                if (nk_option_label(ctx, "Anneal"         ,autOption == Cellular::ANNEAL         )) autOption = Cellular::ANNEAL;
                if (nk_option_label(ctx, "Dry Life"       ,autOption == Cellular::DRYLIFE        )) autOption = Cellular::DRYLIFE;
                if (nk_option_label(ctx, "Pedestr Life"   ,autOption == Cellular::PEDESTRLIFE    )) autOption = Cellular::PEDESTRLIFE;
                nk_layout_row_dynamic(ctx, 30, 4);
                if (nk_option_label(ctx, "Amoeba"         ,autOption == Cellular::AMOEBA         )) autOption = Cellular::AMOEBA;
                if (nk_option_label(ctx, "Diamoeba"       ,autOption == Cellular::DIAMOEBA       )) autOption = Cellular::DIAMOEBA;
                nk_layout_row_dynamic(ctx, 30, 4);
                if (nk_option_label(ctx, "Langton's Ant"  ,autOption == Cellular::LANGTONSANT    )) autOption = Cellular::LANGTONSANT;
              break;
              case 3:
                nk_layout_row_dynamic(ctx, 30, 4);
                if (nk_option_label(ctx, "Brian's Brain"  ,autOption == Cellular::BRIANSBRAIN    )) autOption = Cellular::BRIANSBRAIN;
                if (nk_option_label(ctx, "Brain6"         ,autOption == Cellular::BRAIN6         )) autOption = Cellular::BRAIN6;
                if (nk_option_label(ctx, "Frogs"          ,autOption == Cellular::FROGS          )) autOption = Cellular::FROGS;
                if (nk_option_label(ctx, "Lines"          ,autOption == Cellular::LINES          )) autOption = Cellular::LINES;
              break;
              case 4:
                nk_layout_row_dynamic(ctx, 30, 4);
                if (nk_option_label(ctx, "Caterpillars"   ,autOption == Cellular::CATERPILLARS   ))autOption = Cellular::CATERPILLARS;
                if (nk_option_label(ctx, "OrthoGo"        ,autOption == Cellular::ORTHOGO        )) autOption = Cellular::ORTHOGO;
                if (nk_option_label(ctx, "SediMental"     ,autOption == Cellular::SEDIMENTAL     )) autOption = Cellular::SEDIMENTAL;
                if (nk_option_label(ctx, "StarWars"       ,autOption == Cellular::STARWARS       )) autOption = Cellular::STARWARS;
                nk_layout_row_dynamic(ctx, 30, 4);
                if (nk_option_label(ctx, "Wireworld"      ,autOption == Cellular::WIREWORLD      )) autOption = Cellular::WIREWORLD;
              break;
              case 5:
                nk_layout_row_dynamic(ctx, 30, 4);
                if (nk_option_label(ctx, "Banners"        ,autOption == Cellular::BANNERS        )) autOption = Cellular::BANNERS;
                if (nk_option_label(ctx, "Glissergy"      ,autOption == Cellular::GLISSERGY      )) autOption = Cellular::GLISSERGY;
                if (nk_option_label(ctx, "Spirals"        ,autOption == Cellular::SPIRALS        )) autOption = Cellular::SPIRALS;
                if (nk_option_label(ctx, "Transers"       ,autOption == Cellular::TRANSERS       )) autOption = Cellular::TRANSERS;
                nk_layout_row_dynamic(ctx, 30, 4);
                if (nk_option_label(ctx, "Wanderers"      ,autOption == Cellular::WANDERERS      )) autOption = Cellular::WANDERERS;
              break;
              case 6:
                nk_layout_row_dynamic(ctx, 30, 4);
                if (nk_option_label(ctx, "Chenille"       ,autOption == Cellular::CHENILLE       )) autOption = Cellular::CHENILLE;
                if (nk_option_label(ctx, "FrozenSpirals"  ,autOption == Cellular::FROZENSPIRALS  )) autOption = Cellular::FROZENSPIRALS;
                if (nk_option_label(ctx, "LivingOnTheEdge",autOption == Cellular::LIVINGONTHEEDGE)) autOption = Cellular::LIVINGONTHEEDGE;
                if (nk_option_label(ctx, "PrairieOnFire"  ,autOption == Cellular::PRAIRIEONFIRE  )) autOption = Cellular::PRAIRIEONFIRE;
                nk_layout_row_dynamic(ctx, 30, 4);
                if (nk_option_label(ctx, "Rake"           ,autOption == Cellular::RAKE           )) autOption = Cellular::RAKE;
                if (nk_option_label(ctx, "Snake"          ,autOption == Cellular::SNAKE          )) autOption = Cellular::SNAKE;
                if (nk_option_label(ctx, "SoftFreeze"     ,autOption == Cellular::SOFTFREEZE     )) autOption = Cellular::SOFTFREEZE;
                if (nk_option_label(ctx, "Sticks"         ,autOption == Cellular::STICKS         )) autOption = Cellular::STICKS;
                nk_layout_row_dynamic(ctx, 30, 4);
                if (nk_option_label(ctx, "Worms"          ,autOption == Cellular::WORMS          )) autOption = Cellular::WORMS;
              break;
              case 7:
                nk_layout_row_dynamic(ctx, 30, 4);
                if (nk_option_label(ctx, "Glisserati"     ,autOption == Cellular::GLISSERATI     )) autOption = Cellular::GLISSERATI;
              break;
              case 8:
                nk_layout_row_dynamic(ctx, 30, 4);
                if (nk_option_label(ctx, "BelZhab"        ,autOption == Cellular::BELZHAB        )) autOption = Cellular::BELZHAB;
                if (nk_option_label(ctx, "CircuitGenesis" ,autOption == Cellular::CIRCUITGENESIS )) autOption = Cellular::CIRCUITGENESIS;
                if (nk_option_label(ctx, "Cooties"        ,autOption == Cellular::COOTIES        )) autOption = Cellular::COOTIES;
                if (nk_option_label(ctx, "FlamingStarbows",autOption == Cellular::FLAMINGSTARBOWS)) autOption = Cellular::FLAMINGSTARBOWS;
                nk_layout_row_dynamic(ctx, 30, 4);
                if (nk_option_label(ctx, "Lava"           ,autOption == Cellular::LAVA           )) autOption = Cellular::LAVA;
                if (nk_option_label(ctx, "MeteorGuns"     ,autOption == Cellular::METEORGUNS     )) autOption = Cellular::METEORGUNS;
                if (nk_option_label(ctx, "Swirl"          ,autOption == Cellular::SWIRL          )) autOption = Cellular::SWIRL;
              break;
              case 9:
                nk_layout_row_dynamic(ctx, 30, 4);
                if (nk_option_label(ctx, "Burst"          ,autOption == Cellular::BURST          )) autOption = Cellular::BURST;
                if (nk_option_label(ctx, "Burst2"         ,autOption == Cellular::BURST2         )) autOption = Cellular::BURST2;
              break;
              case 16:
                nk_layout_row_dynamic(ctx, 30, 4);
                if (nk_option_label(ctx, "Xtasy"          ,autOption == Cellular::XTASY          )) autOption = Cellular::XTASY;
              break;
              case 18:
                nk_layout_row_dynamic(ctx, 30, 4);
                if (nk_option_label(ctx, "EbbAndFlow"     ,autOption == Cellular::EBBANDFLOW     )) autOption = Cellular::EBBANDFLOW;
                if (nk_option_label(ctx, "EbbAndFlow2"    ,autOption == Cellular::EBBANDFLOW2    )) autOption = Cellular::EBBANDFLOW2;
              break;
              case 21:
                nk_layout_row_dynamic(ctx, 30, 4);
                if (nk_option_label(ctx, "Fireworks"      ,autOption == Cellular::FIREWORKS      )) autOption = Cellular::FIREWORKS;
              break;
              case 24:
                nk_layout_row_dynamic(ctx, 30, 4);
                if (nk_option_label(ctx, "Bloomerang"     ,autOption == Cellular::BLOOMERANG     )) autOption = Cellular::BLOOMERANG;
              break;
              case 25:
                nk_layout_row_dynamic(ctx, 30, 4);
                if (nk_option_label(ctx, "Faders"         ,autOption == Cellular::FADERS         )) autOption = Cellular::FADERS;
                if (nk_option_label(ctx, "Nova"           ,autOption == Cellular::NOVA           )) autOption = Cellular::NOVA;
                if (nk_option_label(ctx, "Bombers"        ,autOption == Cellular::BOMBERS        )) autOption = Cellular::BOMBERS;
              break;
              case 48:
                nk_layout_row_dynamic(ctx, 30, 4);
                if (nk_option_label(ctx, "ThrillGrill"    ,autOption == Cellular::THRILLGRILL    )) autOption = Cellular::THRILLGRILL;
              break;
            }
          } else if(autType == AutomataType::PROBABILISTIC) {
            if(autStates == 2) {
              nk_layout_row_dynamic(ctx, 30, 1);
              if (nk_option_label(ctx, "Ising", autOption == Probabilistic::ISING)) autOption = Probabilistic::ISING;
              if(autOption == Probabilistic::ISING) {
                char beta_s[256];
                snprintf(beta_s, sizeof(beta_s), "beta %.3f", isingBeta);
                nk_layout_row_dynamic(ctx, 30, 2);
                nk_label(ctx, beta_s, NK_TEXT_LEFT);
                nk_slider_float(ctx, .010, &isingBeta, 10., .010);
              }
            }
          }
          /* nk_group_end(ctx); */

          /* ASSERT(nk_group_begin(ctx, "Scale", NK_WINDOW_BORDER)); */
          nk_layout_row_dynamic(ctx, 30, 1);
          nk_label(ctx, "Scale", NK_TEXT_CENTERED);
          nk_layout_row_dynamic(ctx, 30, factors.size());
          for(const int f : factors) {
            if(nk_option_label(ctx, std::to_string(f).c_str(), factor == f)) factor = f;
          }
          /* nk_group_end(ctx); */


          nk_layout_row_dynamic(ctx, 30, 2);
          if(nk_button_label(ctx, "Simulate")) {
            finished = true;
          }
          if(nk_button_label(ctx, "Quit")) {
            finished = true;
            shouldQuit = true;
          }

          nk_end(ctx);
          set_style(ctx, THEME_RED);
          {
            float bg[4];
            nk_color_fv(bg, background);
            glViewport(0, 0, w.width(), w.height());
            glClear(GL_COLOR_BUFFER_BIT);
            glClearColor(bg[0], bg[1], bg[2], bg[3]);
            /* IMPORTANT: `nk_glfw_render` modifies some global OpenGL state
             * with blending, scissor, face culling, depth test and viewport and
             * defaults everything back into a default state.
             * Make sure to either a.) save and restore or b.) reset your own state after
             * rendering the UI. */
            nk_glfw3_render(&nkglfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
          }
          return !finished;
        } else {
          return false;
        }
      },
      // quit
      [&](auto &w) mutable -> void {
        nk_glfw3_shutdown(&nkglfw);
      }
    );
  }
};
