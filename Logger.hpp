#pragma once

#include <cstdarg>
#include <cstdlib>

#include <string>

#ifdef FLAG_THREADS
#include <mutex>
#endif

#include <File.hpp>
#include <Debug.hpp>

class Logger {
  std::string filename;
  FILE *file = nullptr;

  static char *log_file;
  static FILE *log_file_ptr;
#ifdef FLAG_THREADS
  std::mutex mtx;
#endif

  explicit Logger(const char *filename):
    filename(filename)
  {
    #ifndef NDEBUG
      sys::File::truncate(this->filename);
      file = fopen(this->filename.c_str(), "w");
      ASSERT(file != nullptr);
    #else
      file = stdout;
    #endif
  }
  ~Logger() {
    if(file != nullptr && file != stdout && file != stderr) {
      fclose(file);
    }
  }
  void Write(const char *fmt, va_list args) {
#ifdef FLAT_THREADS
    std::lock_guard<std::mutex> guard(mtx);
#endif
    if(file == nullptr)return;
    ASSERT(file != nullptr);
    vfprintf(file, fmt, args);
    fflush(file);
  }
  void WriteFmt(const char *prefix, const char *fmt, va_list args) {
#ifdef FLAT_THREADS
    std::lock_guard<std::mutex> guard(mtx);
#endif
    if(file == nullptr)return;
    ASSERT(file != nullptr);
    vfprintf(file, (std::string() + prefix + fmt).c_str(), args);
    fflush(file);
  }
  static Logger *instance;
public:
  static void Setup(const char *filename) {
    if(instance == nullptr) {
      instance = new Logger(filename);
    }
    Logger::Info("Started log %s\n", filename);
  }
  static void Say(const char *fmt, ...) {
    ASSERT(instance != nullptr);
    va_list argptr;
    va_start(argptr, fmt);
    instance->Write(fmt, argptr);
    va_end(argptr);
  }
  static void Debug(const char *fmt, ...) {
    #ifndef NDEBUG
    ASSERT(instance != nullptr);
    va_list argptr;
    va_start(argptr, fmt);
    instance->WriteFmt("DEBG: ", fmt, argptr);
    va_end(argptr);
    #endif
  }
  static void Info(const char *fmt, ...) {
    ASSERT(instance != nullptr);
    va_list argptr;
    va_start(argptr, fmt);
    instance->WriteFmt("INFO: ", fmt, argptr);
    va_end(argptr);
  }
  static void Warning(const char *fmt, ...) {
    ASSERT(instance != nullptr);
    va_list argptr;
    va_start(argptr, fmt);
    instance->WriteFmt("WARN: ", fmt, argptr);
    va_end(argptr);
  }
  static void Error(const char *fmt, ...) {
    ASSERT(instance != nullptr);
    va_list argptr;
    va_start(argptr, fmt);
    instance->WriteFmt("ERROR: ", fmt, argptr);
    va_end(argptr);
  }
  static void MirrorLog(FILE *redir) {
    #ifdef __unix__
    ASSERT(instance != nullptr);
    if(instance->file == nullptr) {
      return;
    }
    ASSERT(instance->file != nullptr);
    dup2(fileno(redir), fileno(instance->file));
    if(errno) {
      perror("error");
      errno=0;
    }
    #endif
  }
  static void Close() {
    Logger::Info("Closing log %s", instance->filename.c_str());
    ASSERT(instance != nullptr);
    delete instance;
    instance = nullptr;
  }
};

char *Logger::log_file  = nullptr;
FILE *Logger::log_file_ptr  = nullptr;
Logger *Logger::instance = nullptr;
