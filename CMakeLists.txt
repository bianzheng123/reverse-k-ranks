cmake_minimum_required(VERSION 3.14)
project(Reverse-kRanks)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#add_executable(parhip dataset-partition/graph-partition/parhip_test.cpp)
#TARGET_LINK_LIBRARIES(parhip /home/zhengbian/software/KaHIP/deploy/libparhip.a)
#include_directories(/home/zhengbian/software/KaHIP/deploy)

find_package( PkgConfig )
pkg_check_modules( EIGEN3 REQUIRED eigen3 )
include_directories( ${EIGEN3_INCLUDE_DIRS} )

include(FetchContent)

FetchContent_Declare(
        armadillo
        URL http://sourceforge.net/projects/arma/files/armadillo-10.8.2.tar.xz
)

FetchContent_MakeAvailable(
        armadillo
)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

link_libraries(-lpthread)
#link_libraries(-DNDEBUG)

include_directories(src/include)

SET(APPS
        bfon
        bfmi
        bfdi
        rkbkt
        itvvec
        #        itlsitv
        )
SET(APPS_EXAMPLE
        OnlineBruteForce
        MemoryIndexBruteForce
        DiskIndexBruteForce
        RankBucket
        IntervalVector
        #        ItemListInterval
        )

LIST(LENGTH APPS ll)
message(STATUS "length is ${ll}")
FOREACH (i RANGE 1 ${ll})
    MATH(EXPR temp "${i} - 1")
    #    message(STATUS "current is ${temp}")
    LIST(GET APPS ${temp} app)
    LIST(GET APPS_EXAMPLE ${temp} app_example)
    #    message(STATUS "current is ${app} ${app_example}")
    add_executable(${app} src/apps/${app_example}.cpp)
    target_link_libraries(${app} PRIVATE armadillo)
    #    target_link_libraries(${app} ${MLPACK_LIBRARY})
ENDFOREACH (i)

add_executable(t1 test.cpp)
target_link_libraries(t1 PRIVATE armadillo)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    SET(CMAKE_CXX_FLAGS "-Ofast -DHAVE_CXX0X -march=native -fpic -ftree-vectorize")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    SET(CMAKE_CXX_FLAGS "-Ofast -lrt -DHAVE_CXX0X -march=native -fpic -w -ftree-vectorize -ftree-vectorizer-verbose=0")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    SET(CMAKE_CXX_FLAGS "-Ofast -lrt -DHAVE_CXX0X -march=native -fpic -w -ftree-vectorize")
endif ()
