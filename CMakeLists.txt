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

set(BUILD_TEST OFF)
set(BUILD_FINISH ON)
set(BUILD_TEST_PROGRAM ON)
set(BUILD_INTERVAL_SEARCH OFF)

#target_include_directories(TestArmadilloOpenMP PRIVATE ${armadillo_SOURCE_DIR}/include)
#target_include_directories(TestArmadillo PRIVATE ${armadillo_SOURCE_DIR}/include)
#
#target_link_libraries(TestArmadilloOpenMP PRIVATE armadillo openblas_static pthread gfortran OpenMP::OpenMP_CXX)
#target_link_libraries(TestArmadillo PRIVATE armadillo openblas_static pthread gfortran)

if (BUILD_TEST_PROGRAM)
    add_executable(bbfdi src/apps/BatchDiskBruteForce.cpp)
    target_include_directories(bbfdi PRIVATE ${armadillo_SOURCE_DIR}/include src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(bbfdi PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})
endif ()

if (BUILD_FINISH)
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

if (BUILD_TEST)
    add_subdirectory(attribution)

    add_executable(tsvd test/SVD.cpp)
    target_include_directories(tsvd PRIVATE ${armadillo_SOURCE_DIR}/include src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(tsvd PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})
endif ()

if (BUILD_INTERVAL_SEARCH)
    add_executable(irbfd src/apps/IntervalSearch/IRBFullDim.cpp)
    target_include_directories(irbfd PRIVATE ${armadillo_SOURCE_DIR}/include src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(irbfd PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

    add_executable(irbfi src/apps/IntervalSearch/IRBFullInt.cpp)
    target_include_directories(irbfi PRIVATE ${armadillo_SOURCE_DIR}/include src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(irbfi PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

    add_executable(irbfn src/apps/IntervalSearch/IRBFullNorm.cpp)
    target_include_directories(irbfn PRIVATE ${armadillo_SOURCE_DIR}/include src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(irbfn PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

    add_executable(irbpdpi src/apps/IntervalSearch/IRBPartDimPartInt.cpp)
    target_include_directories(irbpdpi PRIVATE ${armadillo_SOURCE_DIR}/include src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(irbpdpi PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

    add_executable(irbpdpn src/apps/IntervalSearch/IRBPartDimPartNorm.cpp)
    target_include_directories(irbpdpn PRIVATE ${armadillo_SOURCE_DIR}/include src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(irbpdpn PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

    add_executable(irbpipn src/apps/IntervalSearch/IRBPartIntPartNorm.cpp)
    target_include_directories(irbpipn PRIVATE ${armadillo_SOURCE_DIR}/include src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(irbpipn PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})
endif ()


