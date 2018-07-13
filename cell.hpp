#pragma once

#include <cstdlib>
#include <type_traits>
#include <utility>

namespace ca {
template <typename CA>
inline uint8_t random(int y, int x) {
  return rand() % CA::no_states;
}

constexpr int DEAD = 0, LIVE = 1;

template <typename CA, typename B>
inline int count_neighbours(B &prev, int y, int x) {
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

template <int... Is>
using sequence = std::index_sequence<Is...>;

template <int... Bs, int... Ss>
struct BS<sequence<Bs...>, sequence<Ss...>> {
  using this_t = BS<sequence<Bs...>, sequence<Ss...>>;
  static constexpr int outside_state = 0;
  static constexpr int no_states = 2;
  static inline uint8_t init_state(int y, int x) {
    return random<this_t>(y, x);
  }

  template <typename B>
  static inline uint8_t next_state(B &&prev, int y, int x) {
    int count = count_neighbours<this_t>(prev, y, x);
    int result = outside_state;
    if(prev[y][x] == DEAD && one_of_cond<Bs...>::eval(count)/* || (rand() % 100 == 99 && count > 1)*/) {
      return LIVE;
    } else if(prev[y][x] == LIVE && one_of_cond<Ss...>::eval(count)) {
      return LIVE;
    }
    return DEAD;
  }
};
} // namespace ca

// http://www.conwaylife.com/wiki/List_of_Life-like_cellular_automata
namespace cellular {
  using Replicator  =  ca::BS<ca::sequence<1,3,5,7>,     ca::sequence<1,3,5,7>>;
  using Fredkin     =  ca::BS<ca::sequence<1,3,5,7>,     ca::sequence<0,2,4,6,8>>;
  using Seeds       =  ca::BS<ca::sequence<2>,           ca::sequence<>>;
  using LiveOrDie   =  ca::BS<ca::sequence<2>,           ca::sequence<0>>;
  using LifeWtDeath =  ca::BS<ca::sequence<3>,           ca::sequence<0,1,2,4,5,6,7,8>>;
  using Flock       =  ca::BS<ca::sequence<3>,           ca::sequence<1,2>>;
  using Mazectric   =  ca::BS<ca::sequence<3>,           ca::sequence<1,2,3,4>>;
  using Maze        =  ca::BS<ca::sequence<3>,           ca::sequence<1,2,3,4,5>>;
  using Conway      =  ca::BS<ca::sequence<3>,           ca::sequence<2,3>>;
  using EightLife   =  ca::BS<ca::sequence<3>,           ca::sequence<2,3,8>>;
  using LongLife    =  ca::BS<ca::sequence<3,4,5>,       ca::sequence<5>>;
  using TxT         =  ca::BS<ca::sequence<3,6>,         ca::sequence<1,2,5>>;
  using HighLife    =  ca::BS<ca::sequence<3,6>,         ca::sequence<2,3>>;
  using Move        =  ca::BS<ca::sequence<3,6,8>,       ca::sequence<2,4,5>>;
  using Stains      =  ca::BS<ca::sequence<3,6,7,8>,     ca::sequence<2,3,5,6,7,8>>;
  using DayAndNight =  ca::BS<ca::sequence<3,6,7,8>,     ca::sequence<3,4,6,7,8>>;
  using DryLife     =  ca::BS<ca::sequence<3,7>,         ca::sequence<2,3>>;
  using PedestrLife =  ca::BS<ca::sequence<3,8>,         ca::sequence<2,3>>;
} // namespace cellular
