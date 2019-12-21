cmake_minimum_required(VERSION 3.14)
project(automaton)

message("C++ Compiler ${CMAKE_CXX_COMPILER}")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.0)
        message(FATAL_ERROR "GCC version must be at least 4.8 to support OpenMP")
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.8)
        message(FATAL_ERROR "Clang version must be at least 3.8 to support OpenMP.")
    endif()
else()
    message(WARNING "You are using unsupported compiler, and will probably have to change the source code.")
endif()

if(UNIX AND NOT APPLE)
  set(CMAKE_CXX_FLAGS "-std=c++17 -fopenmp -Wall")
else()
  set(CMAKE_CXX_FLAGS "-std=c++17 -Wall")
endif()

set(exec automaton)
add_executable(${exec} ./main.cpp)
include_directories($(CMAKE_CURRENT_SOURCE_DIR))

file(COPY shaders DESTINATION ${CMAKE_BINARY_DIR})
file(COPY resources DESTINATION ${CMAKE_BINARY_DIR})

find_package(PkgConfig REQUIRED)

find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  target_compile_options(${exec} PUBLIC "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(${exec} "${CMAKE_THREAD_LIBS_INIT}")
endif()

pkg_check_modules(EPOXY REQUIRED IMPORTED_TARGET epoxy)
pkg_check_modules(GLFW3 REQUIRED IMPORTED_TARGET glfw3)
pkg_check_modules(GLM REQUIRED IMPORTED_TARGET glm)
target_link_libraries(${exec}
  PkgConfig::EPOXY
  PkgConfig::GLFW3
  PkgConfig::GLM
)

include(ExternalProject)
ExternalProject_Add(Nuklear
    GIT_REPOSITORY https://github.com/Immediate-Mode-UI/Nuklear
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    )
include_directories(${CMAKE_BINARY_DIR}/Nuklear-prefix/src)
add_dependencies(${exec} Nuklear)

if(UNIX)
  install(DIRECTORY resources DESTINATION "${CMAKE_INSTALL_PREFIX}/..")
  install(TARGETS ${exec} DESTINATION "${CMAKE_INSTALL_PREFIX}")
  install(DIRECTORY shaders DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
  install(DIRECTORY resources DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
endif(UNIX)

# find_package(EPOXY REQUIRED)
# pkg_search_module(EPOXY REQUIRED epoxy)
# pkg_check_modules(EPOXY epoxy REQUIRED)
# if(EPOXY_FOUND)
#   message("using ${EPOXY_LIBRARIES} ${EPOXY_INCLUDE_DIRS}")
#   include_directories(${EPOXY_INCLUDE_DIRS})
#   target_link_libraries(${exec} ${EPOXY_LIBRARIES})
# else()
#   message(FATAL_ERROR "EPOXY not found")
# endif(EPOXY_FOUND)

##find_package(GLFW3 REQUIRED)
#pkg_search_module(GLFW3 REQUIRED glfw3)
#pkg_check_modules(GLFW3 glfw3 REQUIRED)
#if(GLFW3_FOUND)
#  message("using ${GLFW3_STATIC_LIBRARIES} ${GLFW3_INCLUDE_DIRS}")
#  include_directories(${GLFW3_INCLUDE_DIRS})
#  target_link_libraries(${exec} ${GLFW3_STATIC_LIBRARIES})
#else()
#  message(FATAL_ERROR "GLFW3 not found")
#endif(GLFW3_FOUND)

# find_package(GLM REQUIRED)
# if(GLM_FOUND)
#   message("using ${GLM_INCLUDE_DIRS}")
#   include_directories(${GLM_INCLUDE_DIRS})
# else()
#   message(FATAL ERROR "GLM not found")
# endif(GLM_FOUND)

# find_packagE(OpenCL REQUIRED)
# if(OpenCL_FOUND)
#   message("using OpenCL")
#   include_directories(${OpenCL_INCLUDE_DIRS})
#   target_link_libraries(${exec} ${OpenCL_LIBRARIES})
# else()
#   message(FATAL ERROR "OPENCL not found")
# endif(OpenCL_FOUND)
