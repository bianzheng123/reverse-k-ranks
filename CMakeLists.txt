cmake_minimum_required(VERSION 3.14)
project(Reverse-kRanks)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_Release} -O3")

find_package(Boost COMPONENTS program_options)

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

set(BUILD_TEST_MODULE OFF)
set(BUILD_ATTRIBUTION ON)
set(BUILD_FINISH OFF)
set(BUILD_CHECK_PROGRAM ON)
set(BUILD_INTERVAL_SEARCH OFF)

#target_include_directories(TestArmadilloOpenMP PRIVATE ${armadillo_SOURCE_DIR}/include)
#target_include_directories(TestArmadillo PRIVATE ${armadillo_SOURCE_DIR}/include)
#
#target_link_libraries(TestArmadilloOpenMP PRIVATE armadillo openblas_static pthread gfortran OpenMP::OpenMP_CXX)
#target_link_libraries(TestArmadillo PRIVATE armadillo openblas_static pthread gfortran)

if (BUILD_CHECK_PROGRAM)
    add_executable(bbfdi src/apps/BatchDiskBruteForce.cpp)
    target_include_directories(bbfdi PRIVATE ${armadillo_SOURCE_DIR}/include src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(bbfdi PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})
endif ()

if (BUILD_FINISH)
    add_executable(irbs src/apps/IntervalRankBoundSwap.cpp)
    target_include_directories(irbs PRIVATE ${armadillo_SOURCE_DIR}/include src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(irbs PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

    add_executable(bfon src/apps/OnlineBruteForce.cpp)
    target_include_directories(bfon PRIVATE ${armadillo_SOURCE_DIR}/include src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(bfon PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

    add_executable(bfmi src/apps/MemoryBruteForce.cpp)
    target_include_directories(bfmi PRIVATE ${armadillo_SOURCE_DIR}/include src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(bfmi PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

    add_executable(bfdi src/apps/DiskBruteForce.cpp)
    target_include_directories(bfdi PRIVATE ${armadillo_SOURCE_DIR}/include src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(bfdi PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

    add_executable(rb src/apps/RankBound.cpp)
    target_include_directories(rb PRIVATE ${armadillo_SOURCE_DIR}/include src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(rb PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})
endif ()

add_executable(irb src/apps/IntervalRankBound.cpp)
target_include_directories(irb PRIVATE ${armadillo_SOURCE_DIR}/include src/include ${Boost_INCLUDE_DIRS})
target_link_libraries(irb PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

add_executable(t1 test.cpp)
target_include_directories(t1 PRIVATE ${armadillo_SOURCE_DIR}/include src/include ${Boost_INCLUDE_DIRS})
target_link_libraries(t1 PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

if (BUILD_ATTRIBUTION)
    add_subdirectory(attribution)
endif ()

if (BUILD_TEST_MODULE)
    add_subdirectory(test)
endif ()

if (BUILD_INTERVAL_SEARCH)
    add_executable(irbfds src/apps/IntervalSearchSwap/IRBFullDim.cpp)
    target_include_directories(irbfds PRIVATE ${armadillo_SOURCE_DIR}/include src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(irbfds PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

    add_executable(irbfis src/apps/IntervalSearchSwap/IRBFullInt.cpp)
    target_include_directories(irbfis PRIVATE ${armadillo_SOURCE_DIR}/include src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(irbfis PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

    add_executable(irbfns src/apps/IntervalSearchSwap/IRBFullNorm.cpp)
    target_include_directories(irbfns PRIVATE ${armadillo_SOURCE_DIR}/include src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(irbfns PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

    add_executable(irbpdpis src/apps/IntervalSearchSwap/IRBPartDimPartInt.cpp)
    target_include_directories(irbpdpis PRIVATE ${armadillo_SOURCE_DIR}/include src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(irbpdpis PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

    add_executable(irbpdpns src/apps/IntervalSearchSwap/IRBPartDimPartNorm.cpp)
    target_include_directories(irbpdpns PRIVATE ${armadillo_SOURCE_DIR}/include src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(irbpdpns PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

    add_executable(irbpipns src/apps/IntervalSearchSwap/IRBPartIntPartNorm.cpp)
    target_include_directories(irbpipns PRIVATE ${armadillo_SOURCE_DIR}/include src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(irbpipns PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})
endif ()


