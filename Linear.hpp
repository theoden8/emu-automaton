#pragma once

#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <type_traits>
#include <utility>

namespace la {

inline uint8_t random(int y, int x, int no_states) {
  return rand() % no_states;
}

constexpr int DEAD = 0, LIVE = 1;

struct Rule {
  using self_t = Rule;

  static constexpr int outside_state = 0;
  static constexpr int no_states = 2;
  static constexpr int dim = 4;
  static constexpr int update_mode = ::update_mode::ALL;

  static inline uint8_t init_state(int y, int x) {
    /* if(y == 0) { */
    /*   if(x < 2) { */
    /*     return LIVE; */
    /*   } */
    /*   return DEAD; */
    /* } */
    if(y == 0) {
      return random(y, x, no_states);
    }
    return outside_state;
  }

  template <typename B>
  inline uint64_t get_case(B &prev, int y, int x) {
    uint64_t result = 0x00;
    const int nn = (n - 1) / 2;
    for(int i = 0; i < n; ++i) {
      int cur = -nn + i;
      if(prev[y][x + cur] == LIVE) {
        result |= (1 << (n - i - 1));
      }
    }
    return result;
  }

  const int n;
  const uint64_t c;
  inline explicit Rule(int n, uint64_t c):
    n(n), c(c)
  {}

  template <typename B>
  inline uint8_t next_state(B &&prev, int y, int x) {
    if(y > 0) {
      return prev[y - 1][x];
    }
    if(c & (uint64_t(1) << get_case(prev, 0, x))) {
      return LIVE;
    }
    return DEAD;
  }
};

decltype(auto) rule(int N, uint64_t C) {
  assert(N <= 6 && (N & 1));
  return [=]() mutable -> Rule {
    return Rule(N, C);
  };
}

} // namespace la

namespace linear {
  decltype(auto) Rule30  = la::rule(3, 30LLU);
  decltype(auto) Rule54  = la::rule(3, 54LLU);
  decltype(auto) Rule90  = la::rule(3, 90LLU);
  decltype(auto) Rule110 = la::rule(3, 110LLU);
  decltype(auto) Rule184 = la::rule(3, 184LLU);
} // namespace linear
