#pragma once

#include <Linear.hpp>
#include <Cellular.hpp>

template <int D, typename F> struct Grid;

template <typename F>
struct Grid<4, F> {
  F func;

  const int width;
  const int height;

  inline Grid(F &&func, int w, int h):
    func(func),
    width(w), height(h)
  {}

  struct Row {
    F func;

    const int y;
    const int width;

    inline Row(F &&func, int y, int w):
      func(func), y(y), width(w)
    {}

    inline decltype(auto) operator[](int x) {
      return func(y, x);
    }
  };

  inline Row operator[](int y) {
    return Row(std::forward<F>(func), y, width);
  }
};

template <int D, typename F, typename... As>
decltype(auto) make_grid(F &&func, As... args) {
  return Grid<4, F>(std::forward<F>(func), args...);
}

template <int D, class Mode> struct Storage;

namespace storage_mode {
  class Textures;
  class HostBuffer;
} // namespace storage

template <>
struct Storage<4, storage_mode::HostBuffer> {
  int w=0, h=0;

  static constexpr int dim = 4;
  using value_type = uint8_t;
  value_type *data = nullptr;

  Storage()
  {}

  void init(int ww, int hh) {
    w=ww,h=hh;
    data = new value_type[w*h];
  }

  void clear() {
    delete [] data;
    data = nullptr;
  }

  ~Storage()
  {}
};

namespace access_mode {
  class bounded;
  class looped;
  class compute_shader;
} // namespace access

template <typename AUT, typename StorageT, typename Mode> struct Access;

template <typename AUT>
struct Access<AUT, Storage<4, storage_mode::HostBuffer>, access_mode::bounded> {
  using StorageT = Storage<4, storage_mode::HostBuffer>;
  static typename StorageT::value_type access(const StorageT &s, int i) {
    return access(s, i / s.w, i % s.w);
  }
  static typename StorageT::value_type access(const StorageT &s, int y, int x) {
    int w=s.w,h=s.h;
    if(y < 0 || y > h || x < 0 || x > w) {
      return AUT::outside_state;
    }
    return s.data[y * w + x];
  }
};

template <typename AUT>
struct Access<AUT, Storage<4, storage_mode::HostBuffer>, access_mode::looped> {
  using StorageT = Storage<4, storage_mode::HostBuffer>;
  static typename StorageT::value_type access(const StorageT &s, int i) {
    return access(s, i / s.w, i % s.w);
  }
  static typename StorageT::value_type access(const StorageT &s, int y, int x) {
    int w=s.w,h=s.h;
    if(y < 0 || y >= h || x < 0 || x >= w) {
      y = (y < 0) ? y + h : y % h;
      x = (x < 0) ? x + w : x % w;
    }
    return s.data[y * w + x];
  }
};
