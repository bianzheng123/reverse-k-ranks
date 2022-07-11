cmake_minimum_required(VERSION 3.20)
project(Reverse-kRanks)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_Release} -O3")
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

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

set(BUILD_PROGRESS ON)
set(BUILD_VAR OFF)

#set(CUDA_TOOLKIT_INCLUDE /usr/local/cuda/include)
#set(CUDA_NVCC_EXECUTABLE /usr/local/cuda/bin/nvcc)
#set(CUDA_INCLUDE_DIRS /usr/local/cuda/include)
#set(CUDA_CUDART_LIBRARY /usr/local/cuda)
#set(CMAKE_CUDA_ARCHITECTURES 75)
#set(CMAKE_CUDA_LINK_EXECUTABLE "${CMAKE_INSTALL_BINDIR}")
#set(CMAKE_CUDA_LINK_EXECUTABLE "usr/local/cuda/bin/nvcc")
#find_package(CUDA REQUIRED)
#set(CUDA_64_BIT_DEVICE_CODE ON)
#set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} -std=c++17 -arch=sm_52")

#CUDA_ADD_LIBRARY(gpu ${PROJECT_SOURCE_DIR}/src/gpu/GPUScoreTable.cu)
#target_include_directories(gpu PRIVATE ${PROJECT_SOURCE_DIR}/src/gpu/GPUScoreTable.hpp ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
#target_link_libraries(gpu ${CUDA_LIBRARIES})

#add_library(gpu SHARED
#        ${PROJECT_SOURCE_DIR}/src/gpu/GPUScoreTable.cu
#        )
#target_include_directories(gpu
#        PUBLIC
#        ${PROJECT_SOURCE_DIR}/src/gpu
#        ${PROJECT_SOURCE_DIR}/src/include
#        ${CUDA_INCLUDE_DIRS}
#        )
#set_target_properties(gpu
#        PROPERTIES
#        LINKER_LANGUAGE CUDA
#        CUDA_STANDARD 17
#        CUDA_STANDARD_REQUIRED ON
#        )

#target_include_directories(TestArmadilloOpenMP PRIVATE ${armadillo_SOURCE_DIR}/include)
#target_include_directories(TestArmadillo PRIVATE ${armadillo_SOURCE_DIR}/include)
#
#target_link_libraries(TestArmadilloOpenMP PRIVATE armadillo openblas_static pthread gfortran OpenMP::OpenMP_CXX)
#target_link_libraries(TestArmadillo PRIVATE armadillo openblas_static pthread gfortran)

enable_language(CUDA)

set_source_files_properties(src/app/ReverseRankIndex.cpp PROPERTIES LANGUAGE CUDA)
add_executable(rri src/app/ReverseRankIndex.cpp)
target_include_directories(rri PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
target_link_libraries(rri PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})
set_property(TARGET rri PROPERTY CXX_STANDARD "17")
set_property(TARGET rri PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET rri PROPERTY CXX_EXTENSIONS OFF)

#add_executable(rri src/app/ReverseRankIndex.cpp)
#target_include_directories(rri PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
#target_link_libraries(rri PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

if (BUILD_VAR)
    add_executable(rrivar src/app/ReverseRankIndexVariance.cpp)
    target_include_directories(rrivar PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(rrivar PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})
endif ()

add_executable(ca src/app/ComputeAll.cpp)
target_include_directories(ca PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
target_link_libraries(ca PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

if (BUILD_PROGRESS)
    add_executable(progress src/app/Progress.cpp)
    target_include_directories(progress PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(progress PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})
endif ()

set_source_files_properties(test.cpp PROPERTIES LANGUAGE CUDA)
add_executable(t1 test.cpp)
target_include_directories(t1 PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
target_link_libraries(t1 PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})
set_property(TARGET t1 PROPERTY CXX_STANDARD "17")
set_property(TARGET t1 PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET t1 PROPERTY CXX_EXTENSIONS OFF)

#if (BUILD_ATTRIBUTION)
#    add_subdirectory(attribution)
#endif ()

if (BUILD_TEST_MODULE)
    #    add_subdirectory(test)
endif ()
