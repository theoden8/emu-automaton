#pragma once

#ifndef __APPLE__
#include <omp.h>
#endif

#include <Filestream.hpp>
#include <Automaton.hpp>

template <typename StorageT> struct Life106Decoder;

template <storage_mode StorageMode> struct Life106Decoder<RenderStorage<StorageMode>> {
  using StorageT = RenderStorage<StorageMode>;
  static void read(const char *filename, StorageT &buf) {
    FILE *fp = fopen(filename, "r");
    filestream fs(fp);
    int w=buf.w,h=buf.h;
    char c;
    std::vector<std::pair<int, int>> points;
    int cx,cy;
    int mnx=INT_MAX,mny=INT_MAX,mxx=INT_MIN,mxy=INT_MIN;
    bool skip=true;
    while((c=fs.getc()) != EOF) {
      if(c == '#')skip=true;
      if(skip && c != '\n')continue;
      if(c == '\n') {
        cx=0,cy=0;
        cx=fs.get_int();
        cy=fs.get_int();
        points.push_back({cx,cy});
        if(cx>mxx){mxx=cx;}if(cx<mnx){mnx=cx;}
        if(cy>mxy){mxy=cy;}if(cy<mny){mny=cy;}
      }
    }
    if(!points.empty()) {
      mxx-=mnx,mxy-=mny;
      int sx=w/2-mxx/2,sy=h/2-mxy/2;
      for(auto &[x, y] : points) {
        x=x-mnx+sx,y=y-mny+sy;
        buf.data[y*w+x]=1;
      }
    }
    fclose(fp);
  }
};
