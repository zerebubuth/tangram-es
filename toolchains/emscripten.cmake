# options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -std=c++11 -Wno-warn-absolute-paths -s USE_GLFW=3 -s FULL_ES2=1 -s EMULATE_FUNCTION_POINTER_CASTS=1")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file ${PROJECT_SOURCE_DIR}/core/resources@/")
set(EXECUTABLE_NAME "tangram")

add_definitions(-DPLATFORM_JS)

find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

if(NOT GLFW_FOUND)
    message(SEND_ERROR "GLFW not found")
    return()
else()
    include_directories(${GLFW_INCLUDE_DIRS})
    message(STATUS "Found GLFW ${GLFW_PREFIX}")
endif()

# load core library
add_subdirectory(${PROJECT_SOURCE_DIR}/core)
include_directories(${CORE_INCLUDE_DIRS})

# add sources and include headers
set(JS_EXTENSIONS_FILES *.cpp)
foreach(_ext ${JS_EXTENSIONS_FILES})
    find_sources_and_include_directories(
        ${PROJECT_SOURCE_DIR}/emscripten/src/*.h
        ${PROJECT_SOURCE_DIR}/emscripten/src/${_ext})
endforeach()

# locate resource files to include
file(GLOB_RECURSE RESOURCES ${PROJECT_SOURCE_DIR}/osx/resources/**)
file(GLOB_RECURSE CORE_RESOURCES ${PROJECT_SOURCE_DIR}/core/resources/**)
list(APPEND RESOURCES ${CORE_RESOURCES})
string(REGEX REPLACE "[.]DS_Store" "" RESOURCES "${RESOURCES}")

# link and build functions
function(link_libraries)

    target_link_libraries(${EXECUTABLE_NAME} core ${GLFW_STATIC_LIBRARIES})

endfunction()

function(build)

    add_executable(${EXECUTABLE_NAME} ${SOURCES})

endfunction()
