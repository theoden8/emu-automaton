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

template <size_t N>
struct Rule {
  using self_t = Rule;
  static constexpr int outside_state = 0;
  static constexpr int no_states = 2;
  static constexpr int no_neighbours = N;
  static constexpr int dim = 4;
  static constexpr int update_mode = ::update_mode::ALL;
  static void check() { static_assert(N <= 6 && (N & 1)); }
  static inline uint8_t init_state(int y, int x) {
    /* if(y == 0) { */
    /*   if(x < 2) { */
    /*     return LIVE; */
    /*   } */
    /*   return DEAD; */
    /* } */
    if(y == 0) {
      return random<self_t>(y, x);
    }
    return outside_state;
  }

  uint64_t c;
  inline explicit Rule(uint64_t c):
    c(c)
  {}

  template <typename B>
  inline uint8_t next_state(B &&prev, int y, int x) {
    if(y > 0) {
      return prev[y - 1][x];
    }
    if(c & (uint64_t(1) << get_case<self_t>(prev, 0, x))) {
      return LIVE;
    }
    return DEAD;
  }
};

template <int N, uint64_t C>
decltype(auto) rule() {
  return Rule<N>(C);
}

} // namespace la

namespace linear {
  constexpr auto Rule30  = la::rule<3, 30LLU>;
  constexpr auto Rule54  = la::rule<3, 54LLU>;
  constexpr auto Rule90  = la::rule<3, 90LLU>;
  constexpr auto Rule110 = la::rule<3, 110LLU>;
  constexpr auto Rule184 = la::rule<3, 184LLU>;
} // namespace linear
