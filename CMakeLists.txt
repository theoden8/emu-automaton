cmake_minimum_required(VERSION 3.14)
project(LifeEmulator)

message("C++ Compiler ${CMAKE_CXX_COMPILER}")

# if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
#     if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.0)
#         message(FATAL_ERROR "GCC version must be at least 4.8 to support OpenMP")
#     endif()
# elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
#     if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.8)
#         message(FATAL_ERROR "Clang version must be at least 3.8 to support OpenMP.")
#     endif()
# else()
#     message(WARNING "You are using unsupported compiler, and will probably have to change the source code.")
# endif()

if(UNIX AND NOT APPLE)
  set(CMAKE_CXX_FLAGS "-std=c++20 -fopenmp -Wall -Wextra -Wno-unused-variable -Wno-deprecated-enum-enum-conversion -Wno-unused-parameter")
else()
if(WIN32)
  set(CMAKE_CXX_FLAGS "/std:c++20")
else()
  set(CMAKE_CXX_FLAGS "-std=c++20 -Wall -Wextra")
endif(WIN32)
endif()

set(exec automaton)
file(GLOB SHADERS shaders/*)
file(GLOB HEADERS *.hpp)
if(APPLE)
  set_source_files_properties(resources/automaton.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
  set_source_files_properties(resources/DroidSans.ttf PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/resources")
  set_source_files_properties(${SHADERS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/shaders")
  set(CMAKE_INSTALL_RPATH @loader_path/../Libraries)
  add_executable(${exec} MACOSX_BUNDLE
    resources/automaton.icns
    resources/DroidSans.ttf
    ${SHADERS}
    ${HEADERS}
    ./main.cpp)
    set_target_properties(${exec} PROPERTIES
        BUNDLE True
        MACOSX_BUNDLE_GUI_IDENTIFIER org.codeberg.theoden8.${exec}
        MACOSX_BUNDLE_BUNDLE_NAME ${exec}
        MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
        MACOSX_BUNDLE_ICONFILE "automaton.icns"
        #MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/resources/Info.plist
    )
else()
  add_executable(${exec} ./main.cpp ${HEADERS} ${SHADERS})
endif()
include_directories($(CMAKE_CURRENT_SOURCE_DIR))

file(COPY shaders DESTINATION ${CMAKE_BINARY_DIR})
file(COPY resources/DroidSans.ttf DESTINATION ${CMAKE_BINARY_DIR}/resources)

find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  target_compile_options(${exec} PUBLIC "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(${exec} "${CMAKE_THREAD_LIBS_INIT}")
endif()

if(WIN32)
  # find_package(EPOXY REQUIRED)
  # pkg_search_module(EPOXY REQUIRED epoxy)
  # pkg_check_modules(EPOXY epoxy REQUIRED)
  # if(EPOXY_FOUND)
  #   message("using ${EPOXY_LIBRARIES} ${EPOXY_INCLUDE_DIRS}")
    include_directories(${EPOXY_INCLUDE_DIRS})
    target_link_libraries(${exec} epoxy)
  # else()
  #   message(FATAL_ERROR "EPOXY not found")
  # endif(EPOXY_FOUND)

  # find_package(GLFW3 REQUIRED)
  # pkg_search_module(GLFW3 REQUIRED glfw3)
  # pkg_check_modules(GLFW3 glfw3 REQUIRED)
  # if(GLFW3_FOUND)
  #   message("using ${GLFW3_STATIC_LIBRARIES} ${GLFW3_INCLUDE_DIRS}")
    include_directories(${GLFW3_INCLUDE_DIRS})
    target_link_libraries(${exec} glfw3)
  # else()
  #   message(FATAL_ERROR "GLFW3 not found")
  # endif(GLFW3_FOUND)

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
else()
  find_package(PkgConfig REQUIRED)

  pkg_check_modules(EPOXY REQUIRED IMPORTED_TARGET epoxy)
  pkg_check_modules(GLFW3 REQUIRED IMPORTED_TARGET glfw3)
  pkg_check_modules(GLM REQUIRED IMPORTED_TARGET glm)
  target_link_libraries(${exec}
    PkgConfig::EPOXY
    PkgConfig::GLFW3
    PkgConfig::GLM
  )
endif(WIN32)

# if(APPLE)
#   set(BUNDLE_FRAMEWORK_PATH "${CMAKE_BINARY_DIR}/gemrb/${exec}.app/Contents/Libraries")

#   file(COPY PkgConfig::EPOXY DESTINATION ${BUNDLE_FRAMEWORK_PATH} FILE_PERMISSIONS OWNER_READ OWNER_WRITE)
#   get_filename_component(EPOXY_BASENAME PkgConfig::EPOXY NAME)
#   set(EPOXY_LIBRARY "${BUNDLE_FRAMEWORK_PATH}/${EPOXY_BASENAME}")
#   add_custom_command(TARGET ${exec} PRE_BUILD COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id "@loader_path/../Libraries/${EPOXY_BASENAME}" "${EPOXY_LIBRARY}")
#   message(STATUS "Copying EPOXY:" PkgConfig::EPOXY)

#   file(COPY PkgConfig::GLFW3 DESTINATION ${BUNDLE_FRAMEWORK_PATH} FILE_PERMISSIONS OWNER_READ OWNER_WRITE)
#   get_filename_component(GLFW3_BASENAME PkgConfig::GLFW3 NAME)
#   set(GLFW3_LIBRARY "${BUNDLE_FRAMEWORK_PATH}/${GLFW3_BASENAME}")
#   add_custom_command(TARGET ${exec} PRE_BUILD COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id "@loader_path/../Libraries/${GLFW3_BASENAME}" "${GLFW3_LIBRARY}")
#   message(STATUS "Copying GLFW3:" PkgConfig::GLFW3)

#   set(BUNDLE_LOADER "${CMAKE_CURRENT_BINARY_DIR}/${exec}.app/Contents/MacOS/${exec}") # BUNDLE_LOADER is used by plugins needing to extend the cocoa wrapper (SDLVideo).
#   set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -bundle_loader \"${BUNDLE_LOADER}\"")
# endif(APPLE)

include(ExternalProject)
ExternalProject_Add(Nuklear
    GIT_REPOSITORY https://github.com/Immediate-Mode-UI/Nuklear
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)
include_directories(${CMAKE_BINARY_DIR}/Nuklear-prefix/src)

add_dependencies(${exec} Nuklear)
if(UNIX AND NOT APPLE)
  install(DIRECTORY resources DESTINATION "${CMAKE_INSTALL_PREFIX}/..")
  install(TARGETS ${exec} DESTINATION "${CMAKE_INSTALL_PREFIX}")
  install(DIRECTORY shaders DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
  install(DIRECTORY resources DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
endif()

# if(APPLE)
#   set_target_properties(${exec} PROPERTIES MACOSX_BUNDLE_INFO_PLIST resources/Info.plist OUTPUT_NAME ${exec})
# endif(APPLE)
