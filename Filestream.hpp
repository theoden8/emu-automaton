#pragma once

#include <cstdio>
#include <vector>
#include <string>

#include <Debug.hpp>
#include <Logger.hpp>

struct filestream {
  FILE *fp = stdin;

  std::vector<char> buf;

  filestream(FILE *fp=stdin):
    fp(fp)
  {}

  void set_stream(FILE *new_fp) {
    fp = new_fp;
    buf.clear();
  }

  char getc() {
    if(buf.empty()) {
      return fgetc(fp);
    }
    char c = buf[buf.size() - 1];
    buf.pop_back();
    return c;
  }

  void ungetc(char c) {
    buf.push_back(c);
  }

  char probe() {
    char c = getc();
    ungetc(c);
    return c;
  }

  int get_int() {
    int d;
    fscanf(fp, " %d", &d);
    return d;
  }

  std::string probe(int len) {
    std::string s;
    s.reserve(len);
    for(int i = 0; i < len; ++i) {
      s += getc();
    }
    for(int i = 0; i < len; ++i) {
      ungetc(s[s.length() - i - 1]);
    }
    return s;
  }

  void skip_space() {
    while(isspace(probe()))getc();
  }

  void skip_line() {
    char c;
    while((c = getc()) != '\n') {
      if(c == EOF) {
        ungetc(c);
        break;
      }
    }
  }
};

