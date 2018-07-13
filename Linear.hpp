#pragma once

#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace la {
template <typename LA>
inline uint8_t random(int y, int x) {
  return rand() % LA::no_states;
}

constexpr int DEAD = 0, LIVE = 1;

template <typename LA, typename B>
inline uint64_t get_case(B &prev, int y, int x) {
  uint64_t result = 0x00;
  constexpr int N = LA::no_neighbours;
  constexpr int nn = (N - 1) / 2;
  for(int i = 0; i < N; ++i) {
    int cur = -nn + i;
    if(prev[y][x + cur] == LIVE) {
      result |= (1 << (N - i - 1));
    }
  }
  return result;
}

template <int N, uint64_t C> struct Rule {
  using this_t = Rule<N, C>;
  static constexpr int outside_state = 0;
  static constexpr int no_states = 2;
  static constexpr int no_neighbours = N;
  static void check() { static_assert(N <= 6 && (N & 1)); }
  static inline uint8_t init_state(int y, int x) {
    /* if(y == 0) { */
    /*   if(x < 2) { */
    /*     return LIVE; */
    /*   } */
    /*   return DEAD; */
    /* } */
    if(y == 0) {
      return random<this_t>(y, x);
    }
    return outside_state;
  }
  template <typename B>
  static inline uint8_t next_state(B &&prev, int y, int x) {
    if(y > 0) {
      return prev[y - 1][x];
    }
    if(C & (uint64_t(1) << get_case<this_t>(prev, 0, x))) {
      return LIVE;
    }
    return DEAD;
  }
};
} // namespace la

namespace linear {
  using Rule30 = la::Rule<3, 30LLU>;
  using Rule54 = la::Rule<3, 54LLU>;
  using Rule90 = la::Rule<3, 90LLU>;
  using Rule110 = la::Rule<3, 110LLU>;
  using Rule184 = la::Rule<3, 184LLU>;
} // namespace linear
