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

constexpr int DEAD = 0, LIVE = 1;

template <typename CA, typename BufT>
inline int count_moore_neighborhood(BufT &prev, int y, int x) {
  int counter = 0;
  for(int iy : {-1, 0, 1}) {
    for(int ix : {-1, 0, 1}) {
      if(!iy&&!ix)continue;
      if(prev[y + iy][x + ix] == LIVE) {
        ++counter;
      }
    }
  }
  return counter;
}

template <class B, class S> struct BS;

template <int... Cs> struct one_of_cond;
template <int C, int... Cs> struct one_of_cond<C, Cs...> {
  static inline bool eval(int val) { return C == val || one_of_cond<Cs...>::eval(val); }
};
template <> struct one_of_cond<> {
  static inline bool eval(int){return false;}
};

template <int... Bs, int... Ss>
struct BS<std::index_sequence<Bs...>, std::index_sequence<Ss...>> {
  using self_t = BS<std::index_sequence<Bs...>, std::index_sequence<Ss...>>;
  static constexpr int outside_state = 0;
  static constexpr int no_states = 2;
  static constexpr int dim = 4;
  static constexpr int update_mode = ::update_mode::ALL;
  static inline uint8_t init_state(int y, int x) {
    return random<self_t>(y, x);
  }

  template <typename B>
  static inline uint8_t next_state(B &&prev, int y, int x) {
    int count = count_moore_neighborhood<self_t>(prev, y, x);
    if(prev[y][x] == DEAD && one_of_cond<Bs...>::eval(count)/* || (rand() % 100 == 99 && count > 1)*/) {
      return LIVE;
    } else if(prev[y][x] == LIVE && one_of_cond<Ss...>::eval(count)) {
      return LIVE;
    }
    return DEAD;
  }
};

struct LangtonsAnt {
  using self_t = LangtonsAnt;
  static constexpr int outside_state = 0;
  static constexpr int no_states = 2;
  static constexpr int dim = 4;
  static constexpr int update_mode = ::update_mode::CURSOR;

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

struct BriansBrain {
  using self_t = BriansBrain;
  static constexpr int outside_state = 0;
  static constexpr int no_states = 3;
  static constexpr int dim = 4;
  static constexpr int update_mode = ::update_mode::ALL;

  static constexpr int DEAD = 0, DYING = 1, LIVE = 2;

  static inline uint8_t init_state(int y, int x) {
    return random<self_t>(y, x);
  }

  template <typename B>
  static int count_moore_neighborhood_live(B &prev, int y, int x) {
    int count = 0;
    for(int ix = -1; ix <= 1; ++ix) {
      for(int iy = -1; iy <= 1; ++iy) {
        if(!ix&&!iy)continue;
        if(prev[y+iy][x+ix]==LIVE)++count;
      }
    }
    return count;
  }

  template <typename B>
  static inline uint8_t next_state(B &&prev, int y, int x) {
    const int state = prev[y][x];
    if(state == DEAD) {
      const int count_on = count_moore_neighborhood_live(prev, y, x);
      if(count_on == 2) {
        return LIVE;
      }
      return DEAD;
    } else if(state == LIVE) {
      return DYING;
    } else if(state == DYING) {
      return DEAD;
    }
    return DEAD;
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
  using Replicator  =  ca::BS<sequence<1,3,5,7>,   sequence<1,3,5,7>>;
  using Fredkin     =  ca::BS<sequence<1,3,5,7>,   sequence<0,2,4,6,8>>;
  using Seeds       =  ca::BS<sequence<2>,         sequence<>>;
  using LiveOrDie   =  ca::BS<sequence<2>,         sequence<0>>;

  using Flock       =  ca::BS<sequence<3>,         sequence<1,2>>;
  using Mazectric   =  ca::BS<sequence<3>,         sequence<1,2,3,4>>;
  using Maze        =  ca::BS<sequence<3>,         sequence<1,2,3,4,5>>;
  using GameOfLife  =  ca::BS<sequence<3>,         sequence<2,3>>;
  using EightLife   =  ca::BS<sequence<3>,         sequence<2,3,8>>;
  using LongLife    =  ca::BS<sequence<3,4,5>,     sequence<5>>;
  using TxT         =  ca::BS<sequence<3,6>,       sequence<1,2,5>>;
  using HighLife    =  ca::BS<sequence<3,6>,       sequence<2,3>>;
  using Move        =  ca::BS<sequence<3,6,8>,     sequence<2,4,5>>;
  using Stains      =  ca::BS<sequence<3,6,7,8>,   sequence<2,3,5,6,7,8>>;
  using DayAndNight =  ca::BS<sequence<3,6,7,8>,   sequence<3,4,6,7,8>>;
  using Anneal      =  ca::BS<sequence<4,6,7,8>,   sequence<3,5,6,7,8>>;
  using DryLife     =  ca::BS<sequence<3,7>,       sequence<2,3>>;
  using PedestrLife =  ca::BS<sequence<3,8>,       sequence<2,3>>;

  using Amoeba      =  ca::BS<sequence<3,5,7>,     sequence<1,3,5,8>>;
  using Diamoeba    =  ca::BS<sequence<3,5,6,7,8>, sequence<5,6,7,8>>;

  using LangtonsAnt = ca::LangtonsAnt;

  // 3 states
  using BriansBrain = ca::BriansBrain;

  // 4 states
  using Wireworld = ca::Wireworld;
} // namespace cellular
