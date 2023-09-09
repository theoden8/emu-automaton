#pragma once


#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <utility>
#include <random>


namespace sca {

template <typename SCA>
inline uint8_t random(int y, int x) {
  return rand() % SCA::no_states;
}

struct ising_model {
  using self_t = ising_model;
  static constexpr int outside_state = 0;
  static constexpr int no_states = 2;
  static constexpr int dim = 4;
  static constexpr int update_mode = ::update_mode::CURSOR;

  static constexpr int DEAD = 0, LIVE = 1;

  float beta, h;
  std::random_device rd;
  std::mt19937 rng;
  std::uniform_real_distribution<> uniform_dist;
  explicit ising_model(float beta=1., float h=.0):
    beta(beta), h(h),
    rd(), rng(rd()), uniform_dist(.0, 1.)
  {}

  static inline uint8_t init_state(int y, int x) {
    return random<self_t>(y, x);
  }

  template <typename B>
  inline std::pair<size_t, uint8_t> next_state(B &&prev) {
    const int w = prev.width, h = prev.height;
    const int cursor = rand() % (w * h);
    const int y = cursor / w, x = cursor % w;
    static_assert(DEAD == 0 && LIVE == 1, "wrong index assumptions");
    static constexpr int vals_lut[] = {-1, 1};
    int S = vals_lut[prev[y][x]];
    int nb = 0;
    for(int ix : {-1, 1}) {
      for(int iy : {-1, 1}) {
        nb += vals_lut[prev[y + iy][x + ix]];
      }
    }
    int dE = 2 * S * nb;
    const float r = uniform_dist(rng);
    if(dE < 0 || std::log(r + 1e-5) < -dE * beta) {
      return std::make_pair(cursor, (prev[y][x] == DEAD) ? LIVE : DEAD);
    }
    return std::make_pair(cursor, prev[y][x]);
  }
};

} // namespace sca

namespace probabilistic {
  using Ising = sca::ising_model;
} // namespace probabilistic
