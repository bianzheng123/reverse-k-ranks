cmake_minimum_required(VERSION 3.14)
project(Reverse-kRanks)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_Release} -O3")

find_package(spdlog REQUIRED)

set(BUILD_STATIC_LIBS ON)
set(BUILD_WITHOUT_LAPACK OFF)

find_package(BLAS REQUIRED)

set(OpenBLAS_LIBRARY ${openblas_BINARY_DIR}/lib/libopenblas.a)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(DETECT_HDF5 OFF CACHE BOOL "")
set(OPENBLAS_PROVIDES_LAPACK ON CACHE BOOL "")
set(ALLOW_FLEXIBLAS_LINUX OFF CACHE BOOL "")
set(ALLOW_OPENBLAS_MACOS ON CACHE BOOL "")

find_package(Armadillo)

find_package(OpenMP)

#target_include_directories(TestArmadilloOpenMP PRIVATE ${armadillo_SOURCE_DIR}/include)
#target_include_directories(TestArmadillo PRIVATE ${armadillo_SOURCE_DIR}/include)
#
#target_link_libraries(TestArmadilloOpenMP PRIVATE armadillo openblas_static pthread gfortran OpenMP::OpenMP_CXX)
#target_link_libraries(TestArmadillo PRIVATE armadillo openblas_static pthread gfortran)

SET(APPS
        bfon
        bfmi
        bfdi
        rkbkt
        itvvec
        bscb
        )
SET(APPS_EXAMPLE
        OnlineBruteForce
        MemoryBruteForce
        DiskBruteForce
        RankBucket
        IntervalVector
        BinarySearchCacheBound
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

    target_include_directories(${app} PRIVATE ${armadillo_SOURCE_DIR}/include src/include)
    target_link_libraries(${app} PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)
    #    target_link_libraries(${app} ${MLPACK_LIBRARY})
ENDFOREACH (i)

#add_executable(Main src/apps/Main.cpp)
#target_include_directories(Main PRIVATE ${armadillo_SOURCE_DIR}/include src/include)
#target_link_libraries(Main PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

add_executable(t1 test.cpp)
target_link_libraries(t1 PRIVATE spdlog::spdlog)