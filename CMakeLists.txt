cmake_minimum_required(VERSION 3.10)
project(Reverse-kRanks)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#add_executable(parhip dataset-partition/graph-partition/parhip_test.cpp)
#TARGET_LINK_LIBRARIES(parhip /home/zhengbian/software/KaHIP/deploy/libparhip.a)
#include_directories(/home/zhengbian/software/KaHIP/deploy)

find_package(Armadillo REQUIRED)

if(Armadillo_FOUND)
    message(STATUS "Armadillo found " ${ARMADILLO_LIBRARY_DIRS})
    include_directories(${ARMADILLO_INCLUDE_DIRS})
    message(STATUS "Armadillo_LIBRARY_DIRS in  " ${ARMADILLO_LIBRARY_DIRS})
    message(STATUS "Armadillo_INCLUDE_DIRS in  " ${ARMADILLO_INCLUDE_DIRS})
endif()

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
FOREACH(i RANGE 1 ${ll})
    MATH(EXPR temp "${i} - 1")
#    message(STATUS "current is ${temp}")
    LIST(GET APPS ${temp} app)
    LIST(GET APPS_EXAMPLE ${temp} app_example)
#    message(STATUS "current is ${app} ${app_example}")
    add_executable(${app} src/apps/${app_example}.cpp)
    target_link_libraries(${app} ${ARMADILLO_LIBRARIES})
    target_link_libraries(${app} ${MLPACK_LIBRARY})
ENDFOREACH(i)

add_executable(t1 test.cpp)
#target_link_libraries(rkitv ${ARMADILLO_LIBRARIES})
#target_link_libraries(rkitv ${MLPACK_LIBRARY})

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    SET(CMAKE_CXX_FLAGS "-Ofast -DNDEBUG -std=c++11 -DHAVE_CXX0X -openmp -march=native -fpic -ftree-vectorize")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    SET(CMAKE_CXX_FLAGS "-Ofast -lrt -DNDEBUG -std=c++11 -DHAVE_CXX0X -openmp -march=native -fpic -w -fopenmp -ftree-vectorize -ftree-vectorizer-verbose=0")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    SET(CMAKE_CXX_FLAGS "-Ofast -lrt -DNDEBUG -std=c++11 -DHAVE_CXX0X -openmp -march=native -fpic -w -fopenmp -ftree-vectorize")
endif ()

