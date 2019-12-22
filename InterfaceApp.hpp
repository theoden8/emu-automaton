#pragma once

#include <cstring>
#include <vector>

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
#include <Nuklear/demo/style.c>

#define MAX_VERTEX_BUFFER (512 * 1024)
#define MAX_ELEMENT_BUFFER (128 * 1024)

struct InterfaceApp {
  Window &w;
  const std::vector<int> factors = {-4, -2, 1, 2, 4, 8, 16, 32};
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
   REPLICATOR,
   FREDKIN,
   SEEDS,
   LIVEORDIE,
   FLOCK,
   MAZECTRIC,
   MAZE,
   GAMEOFLIFE,
   EIGHTLIFE,
   LONGLIFE,
   TXT,
   HIGHLIFE,
   MOVE,
   STAINS,
   DAYANDNIGHT,
   DRYLIFE,
   PEDESTRLIFE,
   NO_CELLULAR
  };

  static constexpr int LINEAR = 0, CELLULAR = 1;
  const std::string dir;

  int autType = CELLULAR;
  int autOption = Cellular::DAYANDNIGHT;
  bool finished = false;
  bool shouldQuit = false;

  InterfaceApp(Window &w, const std::string &dir):
    w(w), dir(dir)
  {}

  void run() {
    w.run(
      // setup
      [&](auto &w) mutable -> void {
        ctx = nk_glfw3_init(g_window, NK_GLFW3_INSTALL_CALLBACKS);
        /* Logger::Info("ctx %p\n", ctx); */
        {
          struct nk_font_atlas *atlas;
          nk_glfw3_font_stash_begin(&atlas);
          struct nk_font *droid = nk_font_atlas_add_from_file(atlas, (dir+"resources/DroidSans.ttf").c_str(), 24, 0);
          /*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/Roboto-Regular.ttf", 14, 0);*/
          /*struct nk_font *future = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/kenvector_future_thin.ttf", 13, 0);*/
          /*struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/ProggyClean.ttf", 12, 0);*/
          /*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/ProggyTiny.ttf", 10, 0);*/
          /*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/Cousine-Regular.ttf", 13, 0);*/
          nk_glfw3_font_stash_end();
          /* nk_style_load_all_cursors(ctx, atlas->cursors); */
          nk_style_set_font(ctx, &droid->handle);
        }
        background = nk_rgb(136,181,216);
      },
      // display
      [&](auto &w) mutable -> bool {
        nk_glfw3_new_frame();
        if (nk_begin(ctx, "Automaton Menu", nk_rect(25, 25, w.width()-50, w.height()-50),
              NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
              NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
        {
          nk_layout_row_dynamic(ctx, 30, 1);
          nk_label(ctx, "Automaton type", NK_TEXT_CENTERED);
          /* ASSERT(nk_group_begin(ctx, "Type", NK_WINDOW_BORDER)); */
          if (nk_option_label(ctx, "Linear"  , autType == LINEAR)) autType = LINEAR;
          if (nk_option_label(ctx, "Cellular", autType == CELLULAR)) autType = CELLULAR;
          /* nk_group_end(ctx); */

          /* ASSERT(nk_group_begin(ctx, "Automaton", NK_WINDOW_BORDER)); */
          nk_layout_row_dynamic(ctx, 30, 1);
          nk_label(ctx, "Automaton", NK_TEXT_CENTERED);
          if(autType == LINEAR) {
            nk_layout_row_dynamic(ctx, 30, 5);
            if (nk_option_label(ctx, "Rule 30"  , autOption == Linear::RULE30 )) autOption = Linear::RULE30;
            if (nk_option_label(ctx, "Rule 54"  , autOption == Linear::RULE54 )) autOption = Linear::RULE54;
            if (nk_option_label(ctx, "Rule 90"  , autOption == Linear::RULE90 )) autOption = Linear::RULE90;
            if (nk_option_label(ctx, "Rule 110" , autOption == Linear::RULE110)) autOption = Linear::RULE110;
            if (nk_option_label(ctx, "Rule 184" , autOption == Linear::RULE184)) autOption = Linear::RULE184;
          } else if(autType == CELLULAR) {
            nk_layout_row_dynamic(ctx, 30, 4);
            if (nk_option_label(ctx, "Replicator"  , autOption == Cellular::REPLICATOR )) autOption = Cellular::REPLICATOR;
            if (nk_option_label(ctx, "Fredkin"     , autOption == Cellular::FREDKIN    )) autOption = Cellular::FREDKIN;
            if (nk_option_label(ctx, "Seeds"       , autOption == Cellular::SEEDS      )) autOption = Cellular::SEEDS;
            if (nk_option_label(ctx, "Live Or Die" , autOption == Cellular::LIVEORDIE  )) autOption = Cellular::LIVEORDIE;
            nk_layout_row_dynamic(ctx, 30, 4);
            if (nk_option_label(ctx, "Flock"       , autOption == Cellular::FLOCK      )) autOption = Cellular::FLOCK;
            if (nk_option_label(ctx, "Mazectric"   , autOption == Cellular::MAZECTRIC  )) autOption = Cellular::MAZECTRIC;
            if (nk_option_label(ctx, "Maze"        , autOption == Cellular::MAZE       )) autOption = Cellular::MAZE;
            if (nk_option_label(ctx, "Game Of Life", autOption == Cellular::GAMEOFLIFE )) autOption = Cellular::GAMEOFLIFE;
            nk_layout_row_dynamic(ctx, 30, 4);
            if (nk_option_label(ctx, "Eight Life"  , autOption == Cellular::EIGHTLIFE  )) autOption = Cellular::EIGHTLIFE;
            if (nk_option_label(ctx, "Long Life"   , autOption == Cellular::LONGLIFE   )) autOption = Cellular::LONGLIFE;
            if (nk_option_label(ctx, "2x2"         , autOption == Cellular::TXT        )) autOption = Cellular::TXT;
            if (nk_option_label(ctx, "High Life"   , autOption == Cellular::HIGHLIFE   )) autOption = Cellular::HIGHLIFE;
            nk_layout_row_dynamic(ctx, 30, 4);
            if (nk_option_label(ctx, "Move"        , autOption == Cellular::MOVE       )) autOption = Cellular::MOVE;
            if (nk_option_label(ctx, "Stains"      , autOption == Cellular::STAINS     )) autOption = Cellular::STAINS;
            if (nk_option_label(ctx, "Day And Night",autOption == Cellular::DAYANDNIGHT)) autOption = Cellular::DAYANDNIGHT;
            if (nk_option_label(ctx, "Dry Life"    , autOption == Cellular::DRYLIFE    )) autOption = Cellular::DRYLIFE;
            if (nk_option_label(ctx, "Pedestr Life", autOption == Cellular::PEDESTRLIFE)) autOption = Cellular::PEDESTRLIFE;
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
            nk_glfw3_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
          }
          return !finished;
        } else {
          return false;
        }
      },
      // quit
      [&](auto &w) mutable -> void {
        nk_glfw3_shutdown();
      }
    );
  }
};
