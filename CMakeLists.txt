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

add_executable(bfon src/apps/OnlineBruteForce.cpp)
target_include_directories(bfon PRIVATE ${armadillo_SOURCE_DIR}/include src/include)
target_link_libraries(bfon PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

add_executable(bfmi src/apps/MemoryBruteForce.cpp)
target_include_directories(bfmi PRIVATE ${armadillo_SOURCE_DIR}/include src/include)
target_link_libraries(bfmi PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

add_executable(bfdi src/apps/DiskBruteForce.cpp)
target_include_directories(bfdi PRIVATE ${armadillo_SOURCE_DIR}/include src/include)
target_link_libraries(bfdi PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

add_executable(bscb src/apps/BinarySearchCacheBound.cpp)
target_include_directories(bscb PRIVATE ${armadillo_SOURCE_DIR}/include src/include)
target_link_libraries(bscb PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

add_executable(bbfdi src/apps/BatchDiskBruteForce.cpp)
target_include_directories(bbfdi PRIVATE ${armadillo_SOURCE_DIR}/include src/include)
target_link_libraries(bbfdi PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

#add_executable(bbscb obsoluted/BatchBinarySearchCacheBound.cpp)
#target_include_directories(bbscb PRIVATE ${armadillo_SOURCE_DIR}/include src/include)
#target_link_libraries(bbscb PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

#add_executable(iscb src/apps/IntervalSearchCacheBound.cpp)
#target_include_directories(iscb PRIVATE ${armadillo_SOURCE_DIR}/include src/include)
#target_link_libraries(iscb PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

add_executable(t1 test.cpp)
target_link_libraries(t1 PRIVATE spdlog::spdlog)
