cmake_minimum_required(VERSION 3.10)
project(Reverse-kRanks)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#add_executable(parhip dataset-partition/graph-partition/parhip_test.cpp)
#TARGET_LINK_LIBRARIES(parhip /home/zhengbian/software/KaHIP/deploy/libparhip.a)
#include_directories(/home/zhengbian/software/KaHIP/deploy)

include_directories(src/include)

add_executable(t1 test.cpp)
add_executable(bfon src/apps/OnlineBruteForce.cpp)
add_executable(bfmi src/apps/MemoryIndexBruteForce.cpp)
add_executable(bfdi src/apps/DiskIndexBruteForce.cpp)
#add_executable(opti src/apps/OptimalIndex.cpp)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    SET(CMAKE_CXX_FLAGS "-Ofast -DNDEBUG -std=c++11 -DHAVE_CXX0X -openmp -march=native -fpic -ftree-vectorize")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    SET(CMAKE_CXX_FLAGS "-Ofast -lrt -DNDEBUG -std=c++11 -DHAVE_CXX0X -openmp -march=native -fpic -w -fopenmp -ftree-vectorize -ftree-vectorizer-verbose=0")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    SET(CMAKE_CXX_FLAGS "-Ofast -lrt -DNDEBUG -std=c++11 -DHAVE_CXX0X -openmp -march=native -fpic -w -fopenmp -ftree-vectorize")
endif ()

