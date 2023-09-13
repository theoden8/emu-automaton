#pragma once

#include <vector>

namespace {
  enum update_mode : int { ALL, CURSOR };
}

#include <Linear.hpp>
#include <Cellular.hpp>
#include <Probabilistic.hpp>


// grid: a macro-topology of the automaton.
// takes a function, which can read and/or write to it
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

enum storage_mode {
  // on the device, as a texture
  TEXTURES,
  // on the host, as a 1D buffer
  HOSTBUFFER,
  NO_STORAGE_MODES
};

// storage: underlying data representation of a topological representation
template <int D, storage_mode StorageMode, class T> struct Storage;

template <storage_mode StorageMode> using RenderStorage = Storage<4, StorageMode, uint8_t>;

template <typename T>
struct Storage<4, storage_mode::HOSTBUFFER, T> {
  int w=0, h=0;

  static constexpr int dim = 4;
  using value_type = T;
  std::vector<value_type> buffer;

  Storage()
  {}

  // create 1d buffer
  void init(int ww, int hh) {
    w=ww,h=hh;
    buffer.resize(w*h);
    buffer.shrink_to_fit();
  }

  value_type *data() {
    return buffer.data();
  }

  const value_type *data() const {
    return buffer.data();
  }

  // delete data
  void clear() {
    buffer.clear();
  }

  bool empty() {
    return buffer.empty();
  }

  ~Storage()
  {}
};

// ways to access the storage (differential topology)
// sometimes this is cleaner than using macro-topology
enum access_mode {
  // as arectangle
  bounded, looped
};

template <typename AUT, typename StorageT, access_mode AccessMode> struct Access;

template <typename AUT, typename T>
struct Access<AUT, Storage<4, storage_mode::HOSTBUFFER, T>, access_mode::bounded> {
  using StorageT = Storage<4, storage_mode::HOSTBUFFER, T>;

  static typename StorageT::value_type access(const StorageT &s, int i) {
    return access(s, i / s.w, i % s.w);
  }

  static typename StorageT::value_type access(const StorageT &s, int y, int x) {
    int w=s.w,h=s.h;
    if(y < 0 || y >= h || x < 0 || x >= w) {
      return AUT::outside_state;
    }
    return s.buffer[y * w + x];
  }
};

template <typename AUT, typename T>
struct Access<AUT, Storage<4, storage_mode::HOSTBUFFER, T>, access_mode::looped> {
  using StorageT = Storage<4, storage_mode::HOSTBUFFER, T>;

  static typename StorageT::value_type access(const StorageT &s, int i) {
    return access(s, i / s.w, i % s.w);
  }

  static typename StorageT::value_type access(const StorageT &s, int y, int x) {
    int w=s.w,h=s.h;
    if(y < 0 || y >= h || x < 0 || x >= w) {
      y = (y < 0) ? y + h : y % h;
      x = (x < 0) ? x + w : x % w;
    }
    return s.buffer[y * w + x];
  }
};
