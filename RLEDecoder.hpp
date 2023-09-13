#pragma once

#include <cstdio>
#include <vector>
#include <string>

#include <Filestream.hpp>
#include <Automaton.hpp>

template <typename StorageT> struct RLEDecoder;

// not working yet
template <storage_mode StorageMode>
struct RLEDecoder<RenderStorage<StorageMode>> {
  using StorageT = RenderStorage<StorageMode>;

  static void read(const char *filename, StorageT &buf) {
    FILE *fp = fopen(filename, "r");
    ASSERT(fp == nullptr);
    filestream fs(fp);
    int my=0,mx=0;
    fscanf(fp, "x = %d, y = %d", &mx, &my);
    fs.skip_line();
    int y=0,x=0;
    bool new_line=1;
    bool after_dollar=0;
    char c;
    while((c = fs.getc()) != EOF) {
      if(c=='\n'){new_line=true;continue;}
      else if(isspace(c))continue;
      if(new_line && (c = fs.getc()) == '#') {
        switch(fs.probe()) {
          case 'c': // comment
          case 'C': // comment
            fs.skip_line(),new_line=1;
          break;
          case 'P': // top-left corner
          case 'R': // top-left corner
            /* fs.getc(); */
            /* sx = fs.get_int(); */
            /* sy = fs.get_int(); */
            fs.skip_line(),new_line=1;
          break;
          case 'N': // name
            fs.skip_line(),new_line=1;
          break;
          case 'O': // author
            fs.skip_line(),new_line=1;
          break;
          case 'r': // rules
            fs.skip_line(),new_line=1;
          break;
        }
        continue;
      }
      fs.ungetc(c);
    }
    fclose(fp);
    /* int shift_x=(w>>1)-(mx>>1); */
    /* int shift_y=(h>>1)-(my>>1); */
    /* for(int iy = shift_y; iy < shift_y + my; ++iy) { */
    /*   for(int ix = shift_x; ix < shift_x + mx; ++ix) { */
    /*     buf1.data[iy*w+ix]=buf2.data[y*w+x]; */
    /*   } */
    /* } */
  }
};
