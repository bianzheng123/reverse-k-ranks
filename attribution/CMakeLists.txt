#add_executable(parhip dataset-partition/graph-partition/parhip_test.cpp)
#TARGET_LINK_LIBRARIES(parhip /home/zhengbian/software/KaHIP/deploy/libparhip.a)
#include_directories(/home/zhengbian/software/KaHIP/deploy)

#include_directories(${PROJECT_SOURCE_DIR}/Gorder)

set(FINISH OFF)

if (FINISH)
    add_executable(kmrt kmeans/KMeansRunTime.cpp)
    target_include_directories(kmrt PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(kmrt PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

    add_executable(ipc prune-implementation/IPCompare.cpp)
    target_include_directories(ipc PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(ipc PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

    add_executable(pi prune-implementation/PruneImplementation.cpp)
    target_include_directories(pi PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(pi PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

    add_executable(pp prune-implementation/PruneParameterClass.cpp)
    target_include_directories(pp PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(pp PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

    add_executable(dvd risk-read-distance-computation/DiskReadVSDistanceComputation.cpp)
    target_include_directories(dvd PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(dvd PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

    add_executable(pur print-user-rank/PrintUserRank.cpp)
    target_include_directories(pur PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(pur PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

    add_executable(usd score-rank-distribution/UserScoreDistribution.cpp)
    target_include_directories(usd PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(usd PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)
endif ()

add_executable(npd norm-popularity-distribution/NormPopularityDistribution.cpp)
target_include_directories(npd PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
target_link_libraries(npd PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

add_executable(srt run-time/ScoreRunTime.cpp)
target_include_directories(srt PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
target_link_libraries(srt PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

add_executable(sd score-distribution/ScoreDistribution.cpp)
target_include_directories(sd PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
target_link_libraries(sd PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})