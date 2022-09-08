#add_executable(parhip dataset-partition/graph-partition/parhip_test.cpp)
#TARGET_LINK_LIBRARIES(parhip /home/zhengbian/software/KaHIP/deploy/libparhip.a)
#include_directories(/home/zhengbian/software/KaHIP/deploy)

#include_directories(${PROJECT_SOURCE_DIR}/Gorder)

set(FINISH OFF)

if (FINISH)
    add_executable(usd score-rank-distribution/UserScoreDistribution.cpp)
    target_include_directories(usd PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(usd PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

    add_executable(sd score-distribution/ScoreDistribution.cpp)
    target_include_directories(sd PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(sd PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

    add_executable(udd disk-index-compression/user-distance-distribution/UserDistanceDistribution.cpp)
    target_include_directories(udd PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(udd PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

    add_executable(crd disk-index-compression/similar-item-rank/CompressRankDistribution.cpp)
    target_include_directories(crd PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(crd PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

    add_executable(ca compute-all/ComputeAll.cpp)
    target_include_directories(ca PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(ca PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

    add_executable(sst disk-index-compression/score-sample-table/ScoreSampleTable.cpp)
    target_include_directories(sst PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(sst PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

    add_executable(pc print-candidates/PrintCandidates.cpp)
    target_include_directories(pc PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(pc PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

    add_executable(mpr disk-index-compression/measure-prune-ratio/MeasurePruneRatio.cpp)
    target_include_directories(mpr PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(mpr PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})
endif ()

if (USE_CUDA)
    set_source_files_properties(print-prune-ratio/PrintPruneRatio.cpp PROPERTIES LANGUAGE CUDA)
    add_executable(ppr print-prune-ratio/PrintPruneRatio.cpp)
    target_include_directories(ppr PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS} ${CUDA_INCLUDE_DIRS})
    target_link_libraries(ppr PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES} cublas)
    set_target_properties(ppr PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_property(TARGET ppr PROPERTY CXX_STANDARD "17")
    set_property(TARGET ppr PROPERTY CXX_STANDARD_REQUIRED ON)
    set_property(TARGET ppr PROPERTY CXX_EXTENSIONS OFF)

else ()
    add_executable(ppr print-prune-ratio/PrintPruneRatio.cpp)
    target_include_directories(ppr PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(ppr PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

endif ()

add_executable(dvd risk-read-distance-computation/DiskReadVSDistanceComputation.cpp)
target_include_directories(dvd PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
target_link_libraries(dvd PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

add_executable(pprnbi print-prune-ratio/PrintPruneRatioNoBuildIndex.cpp)
target_include_directories(pprnbi PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
target_link_libraries(pprnbi PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

add_executable(qdur memory-index-time-compare/QueryDistributionUserRank.cpp)
target_include_directories(qdur PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
target_link_libraries(qdur PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

add_executable(bfcs memory-index-time-compare/BruteForceChooseSample.cpp)
target_include_directories(bfcs PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
target_link_libraries(bfcs PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

add_executable(biipb build-index-bound-selection/BuildIndexIPBound.cpp)
target_include_directories(biipb PRIVATE ../src/include ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
target_link_libraries(biipb PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog Eigen3::Eigen ${Boost_LIBRARIES})

add_executable(rsbist build-index-time/RSBuildIndexSingleThread.cpp)
target_include_directories(rsbist PRIVATE ../src/include ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
target_link_libraries(rsbist PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog Eigen3::Eigen ${Boost_LIBRARIES})

add_executable(rsbimt build-index-time/RSBuildIndexMultipleThread.cpp
        target_include_directories (rsbimt PRIVATE ../src/include ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
        target_link_libraries (rsbimt PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog Eigen3::Eigen ${Boost_LIBRARIES})
