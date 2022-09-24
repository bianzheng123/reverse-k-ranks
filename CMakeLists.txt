cmake_minimum_required(VERSION 3.10)
project(Reverse-kRanks)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_Release} -O3")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")

find_package(Boost COMPONENTS program_options)

#include(FetchContent)
#FetchContent_Declare(
#        spdlog
#        URL https://github.com/gabime/spdlog/archive/refs/tags/v1.10.0.tar.gz
#)
#FetchContent_MakeAvailable(spdlog)

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

set(BUILD_PROGRESS ON)

find_package(Eigen3 3.3 REQUIRED NO_MODULE)

set(fexipro
        src/include/fexipro/alg/Naive.h
        src/include/fexipro/alg/SVDIntUpperBoundIncrPrune2.h

        src/include/fexipro/structs/ExtendMatrix.h
        src/include/fexipro/structs/ExtendMatrixRow.h
        src/include/fexipro/structs/FastHeap.h
        src/include/fexipro/structs/IntMatrixRow.h
        src/include/fexipro/structs/Matrix.h
        src/include/fexipro/structs/SIRMatrixRow.h
        src/include/fexipro/structs/SVDIntMatrixRow.h
        src/include/fexipro/structs/VectorElement.h

        src/include/fexipro/util/Base.h
        src/include/fexipro/util/Calculator.h
        src/include/fexipro/util/Conf.h
        src/include/fexipro/util/FileUtil.h
        src/include/fexipro/util/Logger.h
        src/include/fexipro/util/Monitor.h
        src/include/fexipro/util/SVDUtil.h
        src/include/fexipro/util/TransformUtil.h)

add_library(fexiprolib SHARED ${fexipro})
set_target_properties(fexiprolib PROPERTIES LINKER_LANGUAGE CXX)
#target_link_directories(fexiprolib PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})
target_link_libraries(fexiprolib PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})
target_include_directories(fexiprolib PRIVATE ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})

add_executable(bst src/app/BuildScoreTable.cpp)
target_include_directories(bst PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
target_link_libraries(bst PRIVATE armadillo pthread spdlog::spdlog OpenMP::OpenMP_CXX ${Boost_LIBRARIES} fexiprolib)

add_executable(rri src/app/ReverseRankIndex.cpp)
target_include_directories(rri PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
target_link_libraries(rri PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES} fexiprolib)

add_executable(dbt src/app/query-distribution-index/QueryDistributionIndex.cpp)
target_include_directories(dbt PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
target_link_libraries(dbt PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES} fexiprolib)

##set_source_files_properties(src/app/BatchRunTopT.cpp PROPERTIES LANGUAGE CUDA)
#add_executable(brtt src/app/BatchRunTopT.cpp)
#target_include_directories(brtt PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
##target_include_directories(brtt PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS} ${CUDA_INCLUDE_DIRS})
#target_link_libraries(brtt PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES} Eigen3::Eigen)
##target_link_libraries(brtt PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES} cublas)
#set_target_properties(brtt PROPERTIES
#        POSITION_INDEPENDENT_CODE ON
#        )

#set_property(TARGET brtt PROPERTY CXX_STANDARD "17")
#set_property(TARGET brtt PROPERTY CXX_STANDARD_REQUIRED ON)
#set_property(TARGET brtt PROPERTY CXX_EXTENSIONS OFF)

if (BUILD_PROGRESS)
    add_executable(progress src/app/Progress.cpp)
    target_include_directories(progress PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(progress PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES} fexiprolib)
endif ()

add_executable(t1 test.cpp)
target_include_directories(t1 PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
target_link_libraries(t1 PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog Eigen3::Eigen ${Boost_LIBRARIES} fexiprolib)

if (BUILD_ATTRIBUTION)
    add_subdirectory(attribution)
endif ()

if (BUILD_TEST_MODULE)
    add_subdirectory(test)
endif ()

add_subdirectory(test-direct-io)
