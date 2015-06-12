# options
set(WIN_DEPENDENCIES_DIR ${PROJECT_SOURCE_DIR}/windows/dependencies)

# HARD CODED curl path
set(LIBCURL_DIR "C:/Program Files/cURL")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11" )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS})
set(CXX_FLAGS_DEBUG "-g -O0")
set(EXECUTABLE_NAME "tangram")

add_definitions(-DPLATFORM_WINDOWS)

# add sources and include headers
find_sources_and_include_directories(
	${PROJECT_SOURCE_DIR}/windows/*.h
	${PROJECT_SOURCE_DIR}/windows/*.cpp)

# add sources and include headers for urlWorkers
find_sources_and_include_directories(
	${PROJECT_SOURCE_DIR}/linux/src/urlWorker.*
	${PROJECT_SOURCE_DIR}/linux/src/urlWorker.*)

# add glfw dependency
add_subdirectory(${WIN_DEPENDENCIES_DIR}/glfw)

# load core library
include_directories(${PROJECT_SOURCE_DIR}/core/include/)
include_directories(${PROJECT_SOURCE_DIR}/core/include/jsoncpp/)
add_subdirectory(${PROJECT_SOURCE_DIR}/core)
include_recursive_dirs(${PROJECT_SOURCE_DIR}/core/src/*.h)


function(link_libraries)
	include_directories(${WIN_DEPENDENCIES_DIR}/glfw/include)
	include_directories(${LIBCURL_DIR}/include)
	target_link_libraries(${EXECUTABLE_NAME} core glfw ${GLFW_LIBRARIES} -L${LIBCURL_DIR}/dlls/ -lcurl)
	target_link_libraries(${EXECUTABLE_NAME} core) 
endfunction()

function(build)
	add_executable(${EXECUTABLE_NAME} ${SOURCES})
endfunction()

