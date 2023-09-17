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

inline uint8_t random(int y, int x, int no_states) {
  return rand() % no_states;
}

template <typename CA, typename BufT>
int count_moore_neighborhood(BufT &prev, int y, int x, int on_state) {
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
struct BSC {
  using self_t = BSC;
  static constexpr int outside_state = 0;
  static constexpr int dim = 4;
  static constexpr int update_mode = ::update_mode::ALL;

  uint8_t init_state(int y, int x) {
    return random(y, x, no_states);
  }

  std::bitset<8> bs_bitmask, ss_bitmask;
  int no_states;
  const int DEAD, LIVE;

  explicit BSC(const std::vector<uint8_t> &bs, const std::vector<uint8_t> &ss, int c):
    bs_bitmask(0), ss_bitmask(0), no_states(c),
    DEAD(0), LIVE(no_states - 1)
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
  uint8_t next_state(B &&prev, int y, int x) {
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

decltype(auto) bsc(std::vector<uint8_t> bs, std::vector<uint8_t> ss, int c=2) {
  return [=]() mutable -> BSC {
    return BSC(bs, ss, c);
  };
}

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

  static uint8_t init_state(int y, int x) {
    return DEAD;
    //return random(y, x, no_states);
  }

  template <typename B>
  std::pair<size_t, uint8_t> next_state(B &&prev) {
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

  static uint8_t init_state(int y, int x) {
    return random(y, x, no_states);
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
  static uint8_t next_state(B &&prev, int y, int x) {
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
  decltype(auto) Replicator     = ca::bsc({1,3,5,7},    {1,3,5,7});
  decltype(auto) Fredkin        = ca::bsc({1,3,5,7},    {0,2,4,6,8});
  decltype(auto) Seeds          = ca::bsc({2},          {});
  decltype(auto) LiveOrDie      = ca::bsc({2},          {0});
  decltype(auto) Flock          = ca::bsc({3},          {1,2});
  decltype(auto) Mazectric      = ca::bsc({3},          {1,2,3,4});
  decltype(auto) Maze           = ca::bsc({3},          {1,2,3,4,5});
  decltype(auto) MazectricMice  = ca::bsc({3,7},        {1,2,3,4});
  decltype(auto) MazeMice       = ca::bsc({3,7},        {1,2,3,4,5});
  decltype(auto) GameOfLife     = ca::bsc({3},          {2,3});
  decltype(auto) EightLife      = ca::bsc({3},          {2,3,8});
  decltype(auto) LongLife       = ca::bsc({3,4,5},      {5});
  decltype(auto) TxT            = ca::bsc({3,6},        {1,2,5});
  decltype(auto) HighLife       = ca::bsc({3,6},        {2,3});
  decltype(auto) Move           = ca::bsc({3,6,8},      {2,4,5});
  decltype(auto) Stains         = ca::bsc({3,6,7,8},    {2,3,5,6,7,8});
  decltype(auto) DayAndNight    = ca::bsc({3,6,7,8},    {3,4,6,7,8});
  decltype(auto) Anneal         = ca::bsc({4,6,7,8},    {3,5,6,7,8});
  decltype(auto) DryLife        = ca::bsc({3,7},        {2,3});
  decltype(auto) PedestrLife    = ca::bsc({3,8},        {2,3});
  decltype(auto) Amoeba         = ca::bsc({3,5,7},      {1,3,5,8});
  decltype(auto) Diamoeba       = ca::bsc({3,5,6,7,8},  {5,6,7,8});

  using LangtonsAnt  = ca::LangtonsAnt;

  // 3 states
  decltype(auto) BriansBrain    = ca::bsc({2},          {},                3 );
  decltype(auto) Brain6         = ca::bsc({2,4,6},      {6},               3 );
  decltype(auto) Frogs          = ca::bsc({3,4},        {1,2},             3 );
  decltype(auto) Lines          = ca::bsc({4,5,8},      {0,1,2,3,4,5},     3 );

  // 4 states
  decltype(auto) Caterpillars   = ca::bsc({3,7,8},      {1,2,4,5,6,7},     4 );
  decltype(auto) OrthoGo        = ca::bsc({2},          {3},               4 );
  decltype(auto) SediMental     = ca::bsc({2,5,6,7,8},  {4,5,6,7,8},       4 );
  decltype(auto) StarWars       = ca::bsc({2},          {3,4,5},           4 );

  using Wireworld    = ca::Wireworld;

  // 5 states
  decltype(auto) Banners        = ca::bsc({3,4,5,7},    {2,3,6,7},         5 );
  decltype(auto) Glissergy      = ca::bsc({2,4,5,6,7,8},{0,3,5,6,7,8},     5 );
  decltype(auto) Spirals        = ca::bsc({2,3,4},      {2},               5 );
  decltype(auto) Transers       = ca::bsc({2,6},        {3,4,5},           5 );
  decltype(auto) Wanderers      = ca::bsc({3,4,6,7,8},  {3,4,5},           5 );

  // 6 states
  decltype(auto) Chenille       = ca::bsc({2,4,5,6,7},  {0,5,6,7,8},       6 );
  decltype(auto) FrozenSpirals  = ca::bsc({2,3},        {3,5,6},           6 );
  decltype(auto) LivingOnTheEdge= ca::bsc({3},          {3,4,5},           6 );
  decltype(auto) PrairieOnFire  = ca::bsc({3,4},        {3,4,5},           6 );
  decltype(auto) Rake           = ca::bsc({2,6,7,8},    {3,4,6,7},         6 );
  decltype(auto) Snake          = ca::bsc({2,5},        {0,3,4,6,7},       6 );
  decltype(auto) SoftFreeze     = ca::bsc({3,8},        {1,3,4,5,8},       6 );
  decltype(auto) Sticks         = ca::bsc({2},          {3,4,5,6},         6 );
  decltype(auto) Worms          = ca::bsc({2,5},        {3,4,6,7},         6 );

  // 7 states
  decltype(auto) Glisserati     = ca::bsc({2,4,5,6,7,8},{0,3,5,6,7,8},     7 );

  // 8 states
  decltype(auto) BelZhab        = ca::bsc({2,3},        {1,4,5,6,7,8},     8 );
  decltype(auto) CircuitGenesis = ca::bsc({1,2,3,4},    {2,3,4,5},         8 );
  decltype(auto) Cooties        = ca::bsc({2},          {2,3},             8 );
  decltype(auto) FlamingStarbows= ca::bsc({2,3},        {3,4,7},           8 );
  decltype(auto) Lava           = ca::bsc({4,5,6,7,8},  {1,2,3,4,5},       8 );
  decltype(auto) MeteorGuns     = ca::bsc({3},          {0,1,2,4,5,6,7,8}, 8 );
  decltype(auto) Swirl          = ca::bsc({3,4},        {2,3},             8 );

  // 9 states
  decltype(auto) Burst          = ca::bsc({3,4,6,8},    {0,2,3,5,6,7,8},   9 );
  decltype(auto) Burst2         = ca::bsc({3,4,6,8},    {2,3,5,6,7,8},     9 );

  // 16 states
  decltype(auto) Xtasy          = ca::bsc({2,3,5,6},    {1,4,5,6},         16);

  // 18 states
  decltype(auto) EbbAndFlow     = ca::bsc({3,6},        {0,1,2,4,7,8},     18);
  decltype(auto) EbbAndFlow2    = ca::bsc({3,7},        {0,1,2,4,6,8},     18);

  // 21 states
  decltype(auto) Fireworks      = ca::bsc({1,3},        {2},               21);

  // 24 states
  decltype(auto) Bloomerang     = ca::bsc({3,4,6,7,8},  {2,3,4},           24);

  // 25 states
  decltype(auto) Bombers        = ca::bsc({2,4},        {3,4,5},           25);
  decltype(auto) Nova           = ca::bsc({2,4,7,8},    {4,5,6,7,8},       25);
  decltype(auto) Faders         = ca::bsc({2},          {2},               25);

  // 48 states
  decltype(auto) ThrillGrill    = ca::bsc({3,4},        {1,2,3,4},         48);
} // namespace cellular
