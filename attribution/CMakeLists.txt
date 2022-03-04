#add_executable(parhip dataset-partition/graph-partition/parhip_test.cpp)
#TARGET_LINK_LIBRARIES(parhip /home/zhengbian/software/KaHIP/deploy/libparhip.a)
#include_directories(/home/zhengbian/software/KaHIP/deploy)

#include_directories(${PROJECT_SOURCE_DIR}/Gorder)

add_executable(bd bound-distribution/BoundDistribution.cpp)
target_include_directories(bd PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include)
target_link_libraries(bd PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

add_executable(cp evaluation-sequence/CauchyPercentile.cpp)
target_include_directories(cp PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include)
target_link_libraries(cp PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)
