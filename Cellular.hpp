#pragma once

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <initializer_list>
#include <type_traits>
#include <utility>
#include <bitset>
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

// https://conwaylife.com/wiki/List_of_Generations_rules
// http://www.mirekw.com/ca/rullex_gene.html
template <size_t C>
struct BSC {
  using self_t = BSC<C>;
  static constexpr int outside_state = 0;
  static constexpr int no_states = C;
  static constexpr int dim = 4;
  static constexpr int update_mode = ::update_mode::ALL;
  static inline uint8_t init_state(int y, int x) {
    return random<self_t>(y, x);
  }

  std::bitset<8> bs_bitmask, ss_bitmask;

  static constexpr int DEAD = 0, LIVE = self_t::no_states - 1;

  inline explicit BSC(std::initializer_list<uint8_t> bs, std::initializer_list<uint8_t> ss):
    bs_bitmask(0), ss_bitmask(0)
  {
    for(uint8_t b : bs) {
      bs_bitmask[b] = 1;
    }
    for(uint8_t s : ss) {
      ss_bitmask[s] = 1;
    }
  }

  // 0 dead
  // N - 1 alive
  // N - 2 age 1
  // N - 3 age 2
  // ...

  template <typename B>
  inline uint8_t next_state(B &&prev, int y, int x) {
    const int count = count_moore_neighborhood<self_t>(prev, y, x, LIVE);
    const int state = prev[y][x];
    if(
        (state == DEAD && bs_bitmask[count])
        || (state == LIVE && ss_bitmask[count])
    )
    {
      return LIVE;
    }
    return (state == 0) ? 0 : state - 1;
  }
};

template <class B, class S, size_t C> struct bsc_constructor;
template <size_t... Bs, size_t... Ss, size_t C>
struct bsc_constructor<std::index_sequence<Bs...>, std::index_sequence<Ss...>, C> {
  inline static decltype(auto) make() {
    return BSC<C>({Bs...}, {Ss...});
  }
};

template <class B, class S, size_t C> constexpr auto bsc = bsc_constructor<B, S, C>::make;
template <class B, class S> constexpr auto bs = bsc<B, S, 2>;

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
    if(x<0)x+=w;else if(x>=w)x-=w;
    if(y<0)y+=h;else if(y>=h)y-=h;
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
  constexpr auto Replicator     =  ca::bs<sequence<1,3,5,7>,    sequence<1,3,5,7>>;
  constexpr auto Fredkin        =  ca::bs<sequence<1,3,5,7>,    sequence<0,2,4,6,8>>;
  constexpr auto Seeds          =  ca::bs<sequence<2>,          sequence<>>;
  constexpr auto LiveOrDie      =  ca::bs<sequence<2>,          sequence<0>>;

  constexpr auto Flock          =  ca::bs<sequence<3>,          sequence<1,2>>;
  constexpr auto Mazectric      =  ca::bs<sequence<3>,          sequence<1,2,3,4>>;
  constexpr auto Maze           =  ca::bs<sequence<3>,          sequence<1,2,3,4,5>>;
  constexpr auto MazectricMice  =  ca::bs<sequence<3,7>,        sequence<1,2,3,4>>;
  constexpr auto MazeMice       =  ca::bs<sequence<3,7>,        sequence<1,2,3,4,5>>;
  constexpr auto GameOfLife     =  ca::bs<sequence<3>,          sequence<2,3>>;
  constexpr auto EightLife      =  ca::bs<sequence<3>,          sequence<2,3,8>>;
  constexpr auto LongLife       =  ca::bs<sequence<3,4,5>,      sequence<5>>;
  constexpr auto TxT            =  ca::bs<sequence<3,6>,        sequence<1,2,5>>;
  constexpr auto HighLife       =  ca::bs<sequence<3,6>,        sequence<2,3>>;
  constexpr auto Move           =  ca::bs<sequence<3,6,8>,      sequence<2,4,5>>;
  constexpr auto Stains         =  ca::bs<sequence<3,6,7,8>,    sequence<2,3,5,6,7,8>>;
  constexpr auto DayAndNight    =  ca::bs<sequence<3,6,7,8>,    sequence<3,4,6,7,8>>;
  constexpr auto Anneal         =  ca::bs<sequence<4,6,7,8>,    sequence<3,5,6,7,8>>;
  constexpr auto DryLife        =  ca::bs<sequence<3,7>,        sequence<2,3>>;
  constexpr auto PedestrLife    =  ca::bs<sequence<3,8>,        sequence<2,3>>;

  constexpr auto Amoeba         =  ca::bs<sequence<3,5,7>,      sequence<1,3,5,8>>;
  constexpr auto Diamoeba       =  ca::bs<sequence<3,5,6,7,8>,  sequence<5,6,7,8>>;

  using LangtonsAnt  = ca::LangtonsAnt;

  // 3 states
  constexpr auto BriansBrain    = ca::bsc<sequence<2>,          sequence<>,             3 >;
  constexpr auto Brain6         = ca::bsc<sequence<2,4,6>,      sequence<6>,            3 >;
  constexpr auto Frogs          = ca::bsc<sequence<3,4>,        sequence<1,2>,          3 >;
  constexpr auto Lines          = ca::bsc<sequence<4,5,8>,      sequence<0,1,2,3,4,5>,  3 >;

  // 4 states
  constexpr auto Caterpillars   = ca::bsc<sequence<3,7,8>,      sequence<1,2,4,5,6,7>,  4 >;
  constexpr auto OrthoGo        = ca::bsc<sequence<2>,          sequence<3>,            4 >;
  constexpr auto SediMental     = ca::bsc<sequence<2,5,6,7,8>,  sequence<4,5,6,7,8>,    4 >;
  constexpr auto StarWars       = ca::bsc<sequence<2>,          sequence<3,4,5>,        4 >;

  using Wireworld    = ca::Wireworld;

  // 5 states
  constexpr auto Banners        = ca::bsc<sequence<3,4,5,7>,    sequence<2,3,6,7>,      5 >;
  constexpr auto Glissergy      = ca::bsc<sequence<2,4,5,6,7,8>,sequence<0,3,5,6,7,8>,  5 >;
  constexpr auto Spirals        = ca::bsc<sequence<2,3,4>,      sequence<2>,            5 >;
  constexpr auto Transers       = ca::bsc<sequence<2,6>,        sequence<3,4,5>,        5 >;
  constexpr auto Wanderers      = ca::bsc<sequence<3,4,6,7,8>,  sequence<3,4,5>,        5 >;

  // 6 states
  constexpr auto Chenille       = ca::bsc<sequence<2,4,5,6,7>,  sequence<0,5,6,7,8>,    6 >;
  constexpr auto FrozenSpirals  = ca::bsc<sequence<2,3>,        sequence<3,5,6>,        6 >;
  constexpr auto LivingOnTheEdge= ca::bsc<sequence<3>,          sequence<3,4,5>,        6 >;
  constexpr auto PrairieOnFire  = ca::bsc<sequence<3,4>,        sequence<3,4,5>,        6 >;
  constexpr auto Rake           = ca::bsc<sequence<2,6,7,8>,    sequence<3,4,6,7>,      6 >;
  constexpr auto Snake          = ca::bsc<sequence<2,5>,        sequence<0,3,4,6,7>,    6 >;
  constexpr auto SoftFreeze     = ca::bsc<sequence<3,8>,        sequence<1,3,4,5,8>,    6 >;
  constexpr auto Sticks         = ca::bsc<sequence<2>,          sequence<3,4,5,6>,      6 >;
  constexpr auto Worms          = ca::bsc<sequence<2,5>,        sequence<3,4,6,7>,      6 >;

  // 7 states
  constexpr auto Glisserati     = ca::bsc<sequence<2,4,5,6,7,8>,sequence<0,3,5,6,7,8>,  7 >;

  // 8 states
  constexpr auto BelZhab        = ca::bsc<sequence<2,3>,        sequence<1,4,5,6,7,8>,  8 >;
  constexpr auto CircuitGenesis = ca::bsc<sequence<1,2,3,4>,    sequence<2,3,4,5>,      8 >;
  constexpr auto Cooties        = ca::bsc<sequence<2>,          sequence<2,3>,          8 >;
  constexpr auto FlamingStarbows= ca::bsc<sequence<2,3>,        sequence<3,4,7>,        8 >;
  constexpr auto Lava           = ca::bsc<sequence<4,5,6,7,8>,  sequence<1,2,3,4,5>,    8 >;
  constexpr auto MeteorGuns     = ca::bsc<sequence<3>,       sequence<0,1,2,4,5,6,7,8>, 8 >;
  constexpr auto Swirl          = ca::bsc<sequence<3,4>,        sequence<2,3>,          8 >;

  // 9 states
  constexpr auto Burst          = ca::bsc<sequence<3,4,6,8>,    sequence<0,2,3,5,6,7,8>,9 >;
  constexpr auto Burst2         = ca::bsc<sequence<3,4,6,8>,    sequence<2,3,5,6,7,8>,  9 >;

  // 16 states
  constexpr auto Xtasy          = ca::bsc<sequence<2,3,5,6>,    sequence<1,4,5,6>,      16>;

  // 18 states
  constexpr auto EbbAndFlow     = ca::bsc<sequence<3,6>,        sequence<0,1,2,4,7,8>,  18>;
  constexpr auto EbbAndFlow2    = ca::bsc<sequence<3,7>,        sequence<0,1,2,4,6,8>,  18>;

  // 21 states
  constexpr auto Fireworks      = ca::bsc<sequence<1,3>,        sequence<2>,            21>;

  // 24 states
  constexpr auto Bloomerang     = ca::bsc<sequence<3,4,6,7,8>,  sequence<2,3,4>,        24>;

  // 25 states
  constexpr auto Bombers        = ca::bsc<sequence<2,4>,        sequence<3,4,5>,        25>;
  constexpr auto Nova           = ca::bsc<sequence<2,4,7,8>,    sequence<4,5,6,7,8>,    25>;
  constexpr auto Faders         = ca::bsc<sequence<2>,          sequence<2>,            25>;

  // 48 states
  constexpr auto ThrillGrill    = ca::bsc<sequence<3,4>,        sequence<1,2,3,4>,      48>;
} // namespace cellular
