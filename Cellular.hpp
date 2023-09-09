#pragma once

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <utility>
#include <iostream>

namespace ca {

template <typename CA>
inline uint8_t random(int y, int x) {
  return rand() % CA::no_states;
}

template <typename CA, typename BufT>
inline int count_moore_neighborhood(BufT &prev, int y, int x, int on_state) {
  int counter = 0;
  for(int iy : {-1, 0, 1}) {
    for(int ix : {-1, 0, 1}) {
      if(!iy&&!ix)continue;
      if(prev[y + iy][x + ix] == on_state) {
        ++counter;
      }
    }
  }
  return counter;
}

template <class B, class S, size_t C> struct BSC;

template <int... Cs> struct one_of_cond;
template <int C, int... Cs> struct one_of_cond<C, Cs...> {
  static inline bool eval(int val) { return C == val || one_of_cond<Cs...>::eval(val); }
};
template <> struct one_of_cond<> {
  static inline bool eval(int){return false;}
};

// https://conwaylife.com/wiki/List_of_Generations_rules
template <int... Bs, int... Ss, size_t C>
struct BSC<std::index_sequence<Bs...>, std::index_sequence<Ss...>, C> {
  using self_t = BSC<std::index_sequence<Bs...>, std::index_sequence<Ss...>, C>;
  static constexpr int outside_state = 0;
  static constexpr int no_states = C;
  static constexpr int dim = 4;
  static constexpr int update_mode = ::update_mode::ALL;
  static inline uint8_t init_state(int y, int x) {
    return random<self_t>(y, x);
  }

  static constexpr int DEAD = 0, LIVE = self_t::no_states - 1;

  // 0 dead
  // N - 1 alive
  // N - 2 age 1
  // N - 3 age 2
  // ...

  template <typename B>
  static inline uint8_t next_state(B &&prev, int y, int x) {
    const int count = count_moore_neighborhood<self_t>(prev, y, x, LIVE);
    const int state = prev[y][x];
    switch(state) {
      case DEAD:
        if(one_of_cond<Bs...>::eval(count)/* || (rand() % 100 == 99 && count > 1)*/) {
          return LIVE;
        }
      break;
      case LIVE:
        if(one_of_cond<Ss...>::eval(count)) {
          return LIVE;
        }
        // no break
      default:
        return state - 1;
      break;
    }
    return DEAD;
  }
};

template <class B, class S> using BS = BSC<B, S, 2>;

struct LangtonsAnt {
  using self_t = LangtonsAnt;
  static constexpr int outside_state = 0;
  static constexpr int no_states = 2;
  static constexpr int dim = 4;
  static constexpr int update_mode = ::update_mode::CURSOR;

  static constexpr int DEAD = 0, LIVE = 1;

  enum direction : int {
    LEFT, UP, RIGHT, DOWN, NO_DIRS
  };

  static constexpr int dir_cw(int d) {
    if(d + 1 == direction::NO_DIRS) {
      return 0;
    }
    return d + 1;
  }

  static constexpr int dir_ccw(int d) {
    if(d == 0) {
      return direction::NO_DIRS - 1;
    }
    return d - 1;
  }

  int cursor = -1;
  int dir = -1;
  explicit LangtonsAnt()
  {}

  static inline uint8_t init_state(int y, int x) {
    return DEAD;
    //return random<self_t>(y, x);
  }

  template <typename B>
  inline std::pair<size_t, uint8_t> next_state(B &&prev) {
    const int w = prev.width, h = prev.height;
    if(cursor == -1) {
      cursor = rand() % (w * h);
    }
    if(dir == -1) {
      dir = rand() % NO_DIRS;
    }

    int y = cursor / w, x = cursor % w;
//    const int old_dir = dir;
    dir = (prev[y][x] == DEAD ? dir_ccw : dir_cw)(dir);
    const uint8_t new_val = (prev[y][x] == DEAD) ? LIVE : DEAD;
    switch(dir) {
      case direction::RIGHT:++x; if(x>=w)x-=w; break;
      case direction::LEFT: --x; if(x<0) x+=w; break;
      case direction::UP:   --y; if(y>=h)y-=h; break;
      case direction::DOWN: ++y; if(y<0) y+=h; break;
      default: abort();
    }
//    std::cout << ((prev[y][x] == DEAD) ? "ccw" : "cw")
//              << "[" << old_dir << " " << dir << "]"
//              << " "
//              << ((old_dir == LEFT) ? "left" : (old_dir == RIGHT ? "right" : (old_dir == UP ? "up" : "down")))
//              << " "
//              << "(" << x << " " << y << ")"
//              << std::endl;
    assert(dir >= 0 && dir < direction::NO_DIRS);
    if(x<0)x+=w;if(x>=w)x-=w;
    if(y<0)y+=h;if(y>=h)y-=h;
    const int old_cursor = cursor;
    cursor = y * w + x;
    return std::make_pair(old_cursor, new_val);
  }
};

struct Wireworld {
  using self_t = Wireworld;
  static constexpr int outside_state = 0;
  static constexpr int no_states = 4;
  static constexpr int dim = 4;
  static constexpr int update_mode = ::update_mode::ALL;

  static constexpr int EMPTY = 0, ELECTRON_HEAD = 1, ELECTRON_TAIL = 2, CONDUCTOR = 3;

  static inline uint8_t init_state(int y, int x) {
    return random<self_t>(y, x);
  }

  template <typename B>
  static int count_moore_neighborhood_electron_heads(B &prev, int y, int x) {
    int count = 0;
    for(int ix = -1; ix <= 1; ++ix) {
      for(int iy = -1; iy <= 1; ++iy) {
        if(!ix&&!iy)continue;
        if(prev[y+iy][x+ix]==ELECTRON_HEAD)++count;
      }
    }
    return count;
  }

  template <typename B>
  static inline uint8_t next_state(B &&prev, int y, int x) {
    const int state = prev[y][x];
    switch(state) {
      case EMPTY:return EMPTY;
      case ELECTRON_HEAD:return ELECTRON_TAIL;
      case ELECTRON_TAIL:return CONDUCTOR;
      case CONDUCTOR:;
        const int count_eheads = count_moore_neighborhood_electron_heads(prev, y, x);
      return (count_eheads == 2) ? ELECTRON_HEAD : CONDUCTOR;
    }
    return EMPTY;
  }
};

} // namespace ca

// http://www.conwaylife.com/wiki/List_of_Life-like_cellular_automata
namespace cellular {
  template <size_t... Is> using sequence = std::index_sequence<Is...>;

  // 2 states
  using Replicator   =  ca::BS<sequence<1,3,5,7>,    sequence<1,3,5,7>>;
  using Fredkin      =  ca::BS<sequence<1,3,5,7>,    sequence<0,2,4,6,8>>;
  using Seeds        =  ca::BS<sequence<2>,          sequence<>>;
  using LiveOrDie    =  ca::BS<sequence<2>,          sequence<0>>;

  using Flock        =  ca::BS<sequence<3>,          sequence<1,2>>;
  using Mazectric    =  ca::BS<sequence<3>,          sequence<1,2,3,4>>;
  using Maze         =  ca::BS<sequence<3>,          sequence<1,2,3,4,5>>;
  using MazectricMice=  ca::BS<sequence<3,7>,        sequence<1,2,3,4>>;
  using MazeMice     =  ca::BS<sequence<3,7>,        sequence<1,2,3,4,5>>;
  using GameOfLife   =  ca::BS<sequence<3>,          sequence<2,3>>;
  using EightLife    =  ca::BS<sequence<3>,          sequence<2,3,8>>;
  using LongLife     =  ca::BS<sequence<3,4,5>,      sequence<5>>;
  using TxT          =  ca::BS<sequence<3,6>,        sequence<1,2,5>>;
  using HighLife     =  ca::BS<sequence<3,6>,        sequence<2,3>>;
  using Move         =  ca::BS<sequence<3,6,8>,      sequence<2,4,5>>;
  using Stains       =  ca::BS<sequence<3,6,7,8>,    sequence<2,3,5,6,7,8>>;
  using DayAndNight  =  ca::BS<sequence<3,6,7,8>,    sequence<3,4,6,7,8>>;
  using Anneal       =  ca::BS<sequence<4,6,7,8>,    sequence<3,5,6,7,8>>;
  using DryLife      =  ca::BS<sequence<3,7>,        sequence<2,3>>;
  using PedestrLife  =  ca::BS<sequence<3,8>,        sequence<2,3>>;

  using Amoeba       =  ca::BS<sequence<3,5,7>,      sequence<1,3,5,8>>;
  using Diamoeba     =  ca::BS<sequence<3,5,6,7,8>,  sequence<5,6,7,8>>;

  using LangtonsAnt  = ca::LangtonsAnt;

  // 3 states
  using BriansBrain  = ca::BSC<sequence<2>,          sequence<>,            3>;
  using Brain6       = ca::BSC<sequence<2,4,6>,      sequence<6>,           3>;
  using Frogs        = ca::BSC<sequence<3,4>,        sequence<1,2>,         3>;
  using Lines        = ca::BSC<sequence<4,5,8>,      sequence<0,1,2,3,4,5>, 3>;

  // 4 states
  using Caterpillars = ca::BSC<sequence<3,7,8>,      sequence<1,2,4,5,6,7>,  4>;
  using OrthoGo      = ca::BSC<sequence<2>,          sequence<3>,            4>;
  using SediMental   = ca::BSC<sequence<2,5,6,7,8>,  sequence<4,5,6,7,8>,    4>;
  using StarWars     = ca::BSC<sequence<2>,          sequence<3,4,5>,        4>;

  using Wireworld    = ca::Wireworld;

  // 5 states
  using Banners      = ca::BSC<sequence<3,4,5,7>,    sequence<2,3,6,7>,      5>;
  using Glissergy    = ca::BSC<sequence<2,4,5,6,7,8>,sequence<0,3,5,6,7,8>,  5>;
  using Spirals      = ca::BSC<sequence<2,3,4>,      sequence<2>,            5>;
  using Transers     = ca::BSC<sequence<2,6>,        sequence<3,4,5>,        5>;
  using Wanderers    = ca::BSC<sequence<3,4,6,7,8>,  sequence<3,4,5>,        5>;

  // 6 states
  using FrozenSpirals= ca::BSC<sequence<2,3>,       sequence<3,5,6>,        6>;
  using LiveOnTheEdge= ca::BSC<sequence<3>,         sequence<3,4,5>,        6>;
  using PrairieOnFire= ca::BSC<sequence<3,4>,       sequence<3,4,5>,        6>;
  using Rake         = ca::BSC<sequence<2,6,7,8>,   sequence<3,4,6,7>,      6>;
  using Snake        = ca::BSC<sequence<2,5>,       sequence<0,3,4,6,7>,    6>;
  using SoftFreeze   = ca::BSC<sequence<3,8>,       sequence<1,3,4,5,8>,    6>;
  using Sticks       = ca::BSC<sequence<2>,         sequence<3,4,5,6>,      6>;
  using Worms        = ca::BSC<sequence<2,5>,       sequence<3,4,6,7>,      6>;
} // namespace cellular
