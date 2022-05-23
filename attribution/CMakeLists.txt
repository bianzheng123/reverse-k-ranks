#add_executable(parhip dataset-partition/graph-partition/parhip_test.cpp)
#TARGET_LINK_LIBRARIES(parhip /home/zhengbian/software/KaHIP/deploy/libparhip.a)
#include_directories(/home/zhengbian/software/KaHIP/deploy)

#include_directories(${PROJECT_SOURCE_DIR}/Gorder)

set(FINISH OFF)

if (FINISH)
    add_executable(bd bound-distribution/BoundDistribution.cpp)
    target_include_directories(bd PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(bd PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

    add_executable(bs bound-selection/BoundSelection.cpp)
    target_include_directories(bs PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(bs PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

    add_executable(escp evaluation-sequence/CauchyPercentile.cpp)
    target_include_directories(escp PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(escp PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

    add_executable(kmrt kmeans/KMeansRunTime.cpp)
    target_include_directories(kmrt PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(kmrt PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

    add_executable(ipc prune/IPCompare.cpp)
    target_include_directories(ipc PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(ipc PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

    add_executable(pi prune/PruneImplementation.cpp)
    target_include_directories(pi PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(pi PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

    add_executable(pp prune/PruneParameterClass.cpp)
    target_include_directories(pp PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(pp PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

    add_executable(dvd risk-read-distance-computation/DiskReadVSDistanceComputation.cpp)
    target_include_directories(dvd PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(dvd PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

    add_executable(svdcmp svd-compare/SVDCompare.cpp)
    target_include_directories(svdcmp PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(svdcmp PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)
endif ()

add_executable(pur print-user-rank/PrintUserRank.cpp)
target_include_directories(pur PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
target_link_libraries(pur PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

add_executable(usd score-distribution/UserScoreDistribution.cpp)
target_include_directories(usd PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
target_link_libraries(usd PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)
