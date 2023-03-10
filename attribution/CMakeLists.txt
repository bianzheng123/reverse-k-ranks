#add_executable(parhip dataset-partition/graph-partition/parhip_test.cpp)
#TARGET_LINK_LIBRARIES(parhip /home/zhengbian/software/KaHIP/deploy/libparhip.a)
#include_directories(/home/zhengbian/software/KaHIP/deploy)

#include_directories(${PROJECT_SOURCE_DIR}/Gorder)

set(FINISH OFF)

if (FINISH)
    add_executable(usd score-rank-distribution/UserScoreDistribution.cpp)
    target_include_directories(usd PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(usd PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog)

    add_executable(sd score-distribution/ScoreDistribution.cpp)
    target_include_directories(sd PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(sd PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

    add_executable(ca compute-all/ComputeAll.cpp)
    target_include_directories(ca PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(ca PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

    add_executable(pc print-candidates/PrintCandidates.cpp)
    target_include_directories(pc PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(pc PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})
endif ()

add_executable(dvd risk-read-distance-computation/DiskReadVSDistanceComputation.cpp)
target_include_directories(dvd PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
target_link_libraries(dvd PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog)

add_executable(qdur memory-index-time-compare/QueryDistributionUserRank.cpp)
target_include_directories(qdur PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
target_link_libraries(qdur PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

add_executable(bfcs memory-index-time-compare/BruteForceChooseSample.cpp)
target_include_directories(bfcs PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
target_link_libraries(bfcs PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

add_executable(ibc int-bound-compare/IntBoundCompare.cpp)
target_include_directories(ibc PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
target_link_libraries(ibc PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

add_executable(rmips RMIPS-unstable-result/ReverseMIPS.cpp)
target_include_directories(rmips PRIVATE ../src/include ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
target_link_libraries(rmips PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog Eigen3::Eigen ${Boost_LIBRARIES})

add_executable(cc complexity_compare/ComputeIPComplexity.cpp)
target_include_directories(cc PRIVATE ../src/include ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
target_link_libraries(cc PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog Eigen3::Eigen ${Boost_LIBRARIES})

add_executable(bsc complexity_compare/BinarySearchComplexity.cpp)
target_include_directories(bsc PRIVATE ../src/include ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
target_link_libraries(bsc PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog Eigen3::Eigen ${Boost_LIBRARIES})

#add_executable(biipb build-index-bound-selection/BuildIndexIPBound.cpp)
#target_include_directories(biipb PRIVATE ../src/include ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
#target_link_libraries(biipb PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog Eigen3::Eigen ${Boost_LIBRARIES})

#add_executable(rsbist build-index-time/RSBuildIndexSingleThread.cpp)
#target_include_directories(rsbist PRIVATE ../src/include ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
#target_link_libraries(rsbist PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog Eigen3::Eigen ${Boost_LIBRARIES})
#
#add_executable(rsbimt build-index-time/RSBuildIndexMultipleThread.cpp)
#target_include_directories(rsbimt PRIVATE ../src/include ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
#target_link_libraries(rsbimt PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog Eigen3::Eigen ${Boost_LIBRARIES})
