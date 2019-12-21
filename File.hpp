#pragma once

#include "Debug.hpp"
#include "Logger.hpp"

#include <cstdio>
#include <cassert>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/file.h>

namespace sys {
/* namespace HACK { */
/*   void rename_file(const char *a, const char *b) { */
/*     int err = rename(a, b); */
/*     if(err)perror( "Error renaming file"); */
/*   } */

/*   static void swap_files(std::string a, std::string b) { */
/*     ASSERT(a != b); */
/*     const char *TMP = "dahfsjkgdhjsfgshjkgfdhjgfwfghjfhdgjsvfh"; */
/*     rename_file(a.c_str(), TMP); */
/*     rename_file(b.c_str(), a.c_str()); */
/*     rename_file(TMP, b.c_str()); */
/*   } */
/* } */


#ifdef __linux__
static std::string get_executable_path() {
  std::vector<char> buf(1000);
  readlink("/proc/self/exe", buf.data(), 1000);
  return std::string(buf.begin(), buf.end());
}
#endif

std::string get_executable_directory(int argc, char *argv[]) {
#ifdef __linux__
  const std::string exec = get_executable_path();
#else
  const std::string exec = argv[0];
#endif
  std::string::size_type n = exec.rfind('/');
  std::string folder;
  if(n == std::string::npos) {
    folder = "./";
  } else {
    folder = exec.substr(0, n + 1);
  }
  return folder;
}

class File {
  std::string filename;

public:
  class Lock {
    int fd;
    bool dropped = true;
  public:
    Lock(FILE *file):
      fd(fileno(file))
    {
      ASSERT(File::is_open(fd));
      if(flock(fd, LOCK_EX) < 0) {
        perror("flock[ex]:");
        TERMINATE("unable to lock file\n");
      }
      dropped = false;
    }

    void drop() noexcept {
      if(flock(fd, LOCK_UN) < 0) {
        perror("flock[un]:");
        /* TERMINATE("unable to unlock file\n"); */
        abort();
      }
      dropped = true;
    }

    ~Lock() {
      if(!dropped) {
        drop();
      }
    }
  };
public:
  File(const char *filename):
    filename(filename)
  {}

  size_t length() {
    struct stat st;
    stat(filename.c_str(), &st);
    return st.st_size;
  }

  const std::string &name() const {
    return filename;
  }

  std::string name() {
    return filename;
  }

  static bool is_open(FILE *file) {
    return sys::File::is_open(fileno(file));
  }

  static bool is_open(int fd) {
    return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
  }

  bool is_ext(const std::string &&ext) {
    if(ext.length() > filename.length())
      return false;
    size_t f=filename.length(),e=ext.length();
    return filename.substr(f-e, e) == ext;
  }

  bool exists() {
    return access(filename.c_str(), F_OK) != -1;
  }

  std::string load_text() {
    size_t size = length() + 1;
    std::string text;
    text.resize(size);

    FILE *file = fopen(filename.c_str(), "r");
    if(file == nullptr) {
      TERMINATE("unable to open file '%s' for reading\n", filename.c_str());
    }

    ASSERT(sys::File::is_open(file));
    sys::File::Lock fl(file);

    int i = 0;
    while((text[i] = fgetc(file)) != EOF)
      ++i;
    text[i] = '\0';

    fl.drop();

    fclose(file);
    return text;
  }
};

}
