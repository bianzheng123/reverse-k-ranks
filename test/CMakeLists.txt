add_executable(svd SVD.cpp)
target_include_directories(svd PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
target_link_libraries(svd PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

add_executable(bs BallSearch.cpp)
target_include_directories(bs PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
target_link_libraries(bs PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)
