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
set(BUILD_ATTRIBUTION OFF)

set(BUILD_RKRANK ON)
set(BUILD_RKRANK_FINISH ON)
set(BUILD_INTERVAL_SEARCH ON)
set(BUILD_RANK_SAMPLE OFF)
set(BUILD_DEV ON)

#target_include_directories(TestArmadilloOpenMP PRIVATE ${armadillo_SOURCE_DIR}/include)
#target_include_directories(TestArmadillo PRIVATE ${armadillo_SOURCE_DIR}/include)
#
#target_link_libraries(TestArmadilloOpenMP PRIVATE armadillo openblas_static pthread gfortran OpenMP::OpenMP_CXX)
#target_link_libraries(TestArmadillo PRIVATE armadillo openblas_static pthread gfortran)

if (BUILD_DEV)
    add_executable(irbc src/app/IntervalRankBoundCompress.cpp)
    target_include_directories(irbc PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(irbc PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})
endif ()

if (BUILD_RKRANK)
    add_executable(bdbf src/app/BatchDiskBruteForce.cpp)
    target_include_directories(bdbf PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(bdbf PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

    add_executable(bpt src/app/BPlusTree.cpp)
    target_include_directories(bpt PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(bpt PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

#    add_executable(rb src/app/RankBound.cpp)
#    target_include_directories(rb PRIVATE src/include src/impl ${Boost_INCLUDE_DIRS})
#    target_link_libraries(rb PRIVATE OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

    add_executable(irb src/app/IntervalRankBound.cpp)
    target_include_directories(irb PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(irb PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

    if (BUILD_RKRANK_FINISH)
        add_executable(obf src/app/OnlineBruteForce.cpp)
        target_include_directories(obf PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
        target_link_libraries(obf PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

        add_executable(mbf src/app/MemoryBruteForce.cpp)
        target_include_directories(mbf PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
        target_link_libraries(mbf PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

        add_executable(dbf src/app/DiskBruteForce.cpp)
        target_include_directories(dbf PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
        target_link_libraries(dbf PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})
    endif ()

    if (BUILD_INTERVAL_SEARCH)
        add_executable(irbfdp src/app/IntervalSearch/IRBFullDimPrune.cpp)
        target_include_directories(irbfdp PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
        target_link_libraries(irbfdp PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

        add_executable(irbfip src/app/IntervalSearch/IRBFullIntPrune.cpp)
        target_include_directories(irbfip PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
        target_link_libraries(irbfip PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

        add_executable(irbfnp src/app/IntervalSearch/IRBFullNormPrune.cpp)
        target_include_directories(irbfnp PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
        target_link_libraries(irbfnp PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

        add_executable(irbpdpip src/app/IntervalSearch/IRBPartDimPartIntPrune.cpp)
        target_include_directories(irbpdpip PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
        target_link_libraries(irbpdpip PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

        add_executable(irbpdpnp src/app/IntervalSearch/IRBPartDimPartNormPrune.cpp)
        target_include_directories(irbpdpnp PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
        target_link_libraries(irbpdpnp PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

        add_executable(irbpipnp src/app/IntervalSearch/IRBPartIntPartNormPrune.cpp)
        target_include_directories(irbpipnp PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
        target_link_libraries(irbpipnp PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

        add_executable(irbbp src/app/IntervalSearch/IRBBallPrune.cpp)
        target_include_directories(irbbp PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
        target_link_libraries(irbbp PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})
    endif ()

    if (BUILD_RANK_SAMPLE)
        add_executable(rbnet src/app/RankSample/RBNoEarlyTermination.cpp)
        target_include_directories(rbnet PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
        target_link_libraries(rbnet PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})
    endif ()

endif ()

add_executable(t1 test.cpp)
target_include_directories(t1 PRIVATE ${armadillo_SOURCE_DIR}/include src/include ${Boost_INCLUDE_DIRS})
target_link_libraries(t1 PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

if (BUILD_ATTRIBUTION)
    add_subdirectory(attribution)
endif ()

if (BUILD_TEST_MODULE)
    add_subdirectory(test)
endif ()
