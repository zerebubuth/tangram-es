# css-color-parser-cpp #
########################

add_library(css-color-parser-cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/css-color-parser-cpp/csscolorparser.cpp)

target_include_directories(css-color-parser-cpp
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/css-color-parser-cpp
)

## geojsonvt ##
###############

file(GLOB_RECURSE GEOJSON_VT_CPP_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/geojson-vt-cpp/src/*.cpp")

include_directories(${INCLUDE_DIR})

add_library(geojson-vt-cpp ${GEOJSON_VT_CPP_SOURCES})
target_include_directories(geojson-vt-cpp
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/geojson-vt-cpp/include)

target_compile_options(geojson-vt-cpp
  PUBLIC
  -Wno-unknown-pragmas)

# duktape #
############

add_library(duktape
  ${CMAKE_CURRENT_SOURCE_DIR}/duktape/duktape.c)

target_compile_options(duktape PRIVATE
  -fstrict-aliasing
  -fomit-frame-pointer
  -std=c99
  -Wall)

target_include_directories(duktape
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/duktape
)
