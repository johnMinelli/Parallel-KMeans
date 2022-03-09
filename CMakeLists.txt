cmake_minimum_required(VERSION 3.21)
project(ParallelK)

find_package(OpenMP REQUIRED)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS} -DUSE_OMP")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ${OpenMP_CXX_FLAGS} -DUSE_OMP")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}" )
endif()

SET(SDL2_DIR "C:/tools/SDL2/x86_64-w64-mingw32/lib/cmake/SDL2")  #TODO set here your SDL2 path
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
if (SDL2_FOUND)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_SDL")
endif()

add_executable(ParallelK main.cpp dataManager.cpp dataManager.h argsParser.cpp argsParser.h kmeans.h GUIRenderer.cpp GUIRenderer.h)
target_link_libraries(ParallelK ${SDL2_LIBRARIES})