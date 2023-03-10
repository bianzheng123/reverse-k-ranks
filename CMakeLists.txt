cmake_minimum_required(VERSION 3.10)
project(Reverse-kRanks)

set(CMAKE_CXX_STANDARD 17)
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

find_package(OpenMP REQUIRED)

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

set(pgm
        obsoluted/PGMIndex/pgm/pgm_index.hpp
        obsoluted/PGMIndex/pgm/piecewise_linear_model.hpp
        )

add_library(pgmindexlib SHARED ${pgm})
set_target_properties(pgmindexlib PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(pgmindexlib PRIVATE OpenMP::OpenMP_CXX)
#target_include_directories(pgmindexlib INTERFACE include/)

set(USE_CUDA OFF)

if (USE_CUDA)
    set(CMAKE_CUDA_STANDARD 17)
    set(CMAKE_CUDA_STANDARD_REQUIRED ON)

    add_definitions(-DUSE_GPU)
    enable_language(CUDA)
    #    find_package(CUDAToolkit)

    #    set(CMAKE_CUDA_ARCHITECTURES 86)
    #    set(CMAKE_CUDA_COMPILER /usr/local/cuda-11.8/bin/nvcc)
    #    set(Thrust_DIR /usr/include/thrust)
    set(Thrust_DIR /mnt/c/ProgramData/Anaconda3/lib/cmake/thrust)
    find_package(Thrust REQUIRED CONFIG)
    thrust_create_target(Thrust)

    set_source_files_properties(src/app/BuildScoreTable.cpp PROPERTIES LANGUAGE CUDA)
    add_executable(bst src/app/BuildScoreTable.cpp)
    #target_include_directories(bst PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
    target_include_directories(bst PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS} ${CUDA_INCLUDE_DIRS})
    #target_link_libraries(bst PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES} Eigen3::Eigen)
    target_link_libraries(bst PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES} cublas)
    set_target_properties(bst PROPERTIES
            POSITION_INDEPENDENT_CODE ON
            )
    set_property(TARGET bst PROPERTY CXX_STANDARD "17")
    set_property(TARGET bst PROPERTY CXX_STANDARD_REQUIRED ON)
    set_property(TARGET bst PROPERTY CXX_EXTENSIONS OFF)
    set_target_properties(bst PROPERTIES CUDA_ARCHITECTURES "35;50;72")


    set_source_files_properties(src/app/BuildScoreTableBatch.cpp PROPERTIES LANGUAGE CUDA)
    add_executable(bstb src/app/BuildScoreTableBatch.cpp)
    target_include_directories(bstb PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS} ${CUDA_INCLUDE_DIRS})
    target_link_libraries(bstb PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES} cublas)
    set_target_properties(bstb PROPERTIES
            POSITION_INDEPENDENT_CODE ON
            )
    set_property(TARGET bstb PROPERTY CXX_STANDARD "17")
    set_property(TARGET bstb PROPERTY CXX_STANDARD_REQUIRED ON)
    set_property(TARGET bstb PROPERTY CXX_EXTENSIONS OFF)
    set_target_properties(bstb PROPERTIES CUDA_ARCHITECTURES "35;50;72")

    set_source_files_properties(test.cpp PROPERTIES LANGUAGE CUDA)
    add_executable(t1 test.cpp)
    target_include_directories(t1 PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS} ${CUDA_INCLUDE_DIRS})
    target_link_libraries(t1 PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES} cublas)
    set_target_properties(t1 PROPERTIES
            POSITION_INDEPENDENT_CODE ON
            )
    set_property(TARGET t1 PROPERTY CXX_STANDARD "17")
    set_property(TARGET t1 PROPERTY CXX_STANDARD_REQUIRED ON)
    set_property(TARGET t1 PROPERTY CXX_EXTENSIONS OFF)
    set_target_properties(t1 PROPERTIES CUDA_ARCHITECTURES "35;50;72")

else () # not use cuda

    add_executable(bst src/app/BuildScoreTable.cpp)
    target_include_directories(bst PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(bst PRIVATE armadillo pthread spdlog::spdlog OpenMP::OpenMP_CXX ${Boost_LIBRARIES} fexiprolib)

    add_executable(bstb src/app/BuildScoreTableBatch.cpp)
    target_include_directories(bstb PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(bstb PRIVATE armadillo pthread spdlog::spdlog OpenMP::OpenMP_CXX ${Boost_LIBRARIES} fexiprolib)

    add_executable(rri src/app/ReverseRankIndex.cpp)
    target_include_directories(rri PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(rri PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog Eigen3::Eigen ${Boost_LIBRARIES} fexiprolib)

    add_executable(rtk src/app/ReverseTopk.cpp)
    target_include_directories(rtk PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(rtk PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog Eigen3::Eigen ${Boost_LIBRARIES} fexiprolib)

    add_executable(qdi src/app/build-index/QueryDistributionIndex.cpp)
    target_include_directories(qdi PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(qdi PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES} fexiprolib)

    add_executable(qdibc src/app/build-index/QueryDistributionIndexByComputation.cpp)
    target_include_directories(qdibc PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(qdibc PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES} fexiprolib)

    add_executable(fsr src/app/build-index/FindSampleRank.cpp)
    target_include_directories(fsr PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(fsr PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES} fexiprolib)

    add_executable(bsibs src/app/build-index/BuildSampleIndexBySample.cpp)
    target_include_directories(bsibs PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(bsibs PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES} fexiprolib)

    add_executable(bsibc src/app/build-index/BuildSampleIndexByComputation.cpp)
    target_include_directories(bsibc PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(bsibc PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES} fexiprolib)

    add_executable(bilrbc src/app/build-index/BuildIntLRByComputation.cpp)
    target_include_directories(bilrbc PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(bilrbc PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog Eigen3::Eigen ${Boost_LIBRARIES} fexiprolib)

    add_executable(ti src/app/TransformIndex.cpp)
    target_include_directories(ti PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(ti PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES} fexiprolib)

    add_executable(t1 test.cpp)
    target_include_directories(t1 PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(t1 PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog Eigen3::Eigen ${Boost_LIBRARIES} fexiprolib)

    if (BUILD_PROGRESS)
        add_executable(progress src/app/Progress.cpp)
        target_include_directories(progress PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
        target_link_libraries(progress PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES} fexiprolib)
    endif ()

    if (BUILD_ATTRIBUTION)
        add_subdirectory(attribution)
    endif ()

    if (BUILD_TEST_MODULE)
        add_subdirectory(test)
    endif ()

endif ()

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

