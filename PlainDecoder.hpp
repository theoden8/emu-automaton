#pragma once

#ifndef __APPLE__
#include <omp.h>
#endif

#include <Filestream.hpp>
#include <Automaton.hpp>

template <typename StorageT> struct PlainDecoder;

template <typename Mode> struct PlainDecoder<RenderStorage<Mode>> {
  using StorageT = RenderStorage<Mode>;

  static void read(const char *filename, StorageT &buf) {
    int w=buf.w,h=buf.h;

    FILE *fp = fopen(filename, "r");
    filestream fs(fp);
    int mx=0,my=0,x=0,y=0;
    char c;
    while((c = fs.getc()) != EOF) {
      switch(c) {
        case '!':fs.skip_line();break;
        case '.':buf.data[y*w+x] = 0;++x;if(x>mx)mx=x;break;
        case 'O':buf.data[y*w+x] = 1;++x;if(x>mx)mx=x;break;
        case '\n':x=0,++y,++my;break;
      }
    }
    int sx = w/2 - mx/2, sy = h/2 - my/2;
    for(int iy = my - 1; iy >= 0; --iy) {
      for(int ix = mx - 1; ix >= 0; --ix) {
        buf.data[(sy+iy)*w+(sx+ix)] = buf.data[iy*w+ix];
        buf.data[iy*w+ix]=0;
      }
    }
    fclose(fp);
  }
};
