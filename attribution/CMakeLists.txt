#add_executable(parhip dataset-partition/graph-partition/parhip_test.cpp)
#TARGET_LINK_LIBRARIES(parhip /home/zhengbian/software/KaHIP/deploy/libparhip.a)
#include_directories(/home/zhengbian/software/KaHIP/deploy)

#include_directories(${PROJECT_SOURCE_DIR}/Gorder)

add_executable(t2 test.cpp)

add_executable(bd bound-distribution/BoundDistribution.cpp)
target_include_directories(bd PRIVATE ../src/include)
target_link_libraries(bd PRIVATE OpenMP::OpenMP_CXX)
