#add_executable(parhip dataset-partition/graph-partition/parhip_test.cpp)
#TARGET_LINK_LIBRARIES(parhip /home/zhengbian/software/KaHIP/deploy/libparhip.a)
#include_directories(/home/zhengbian/software/KaHIP/deploy)

#include_directories(${PROJECT_SOURCE_DIR}/Gorder)

set(FINISH OFF)

if (FINISH)
    add_executable(kmrt kmeans/KMeansRunTime.cpp)
    target_include_directories(kmrt PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(kmrt PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

    add_executable(dvd risk-read-distance-computation/DiskReadVSDistanceComputation.cpp)
    target_include_directories(dvd PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(dvd PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

    add_executable(usd score-rank-distribution/UserScoreDistribution.cpp)
    target_include_directories(usd PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(usd PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

    add_executable(npd norm-popularity-distribution/NormPopularityDistribution.cpp)
    target_include_directories(npd PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(npd PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

    add_executable(srt run-time/ScoreRunTime.cpp)
    target_include_directories(srt PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(srt PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})

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

    add_executable(pur score-sample-advantage/print-user-rank/PrintUserRank.cpp)
    target_include_directories(pur PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(pur PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog)

    add_executable(mpr disk-index-compression/measure-prune-ratio/MeasurePruneRatio.cpp)
    target_include_directories(mpr PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(mpr PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})
endif ()

if (USE_CUDA)
    set_source_files_properties(user-query-relationship/SampleItemTopKUser.cpp PROPERTIES LANGUAGE CUDA)
    add_executable(sitku user-query-relationship/SampleItemTopKUser.cpp)
    target_include_directories(sitku PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS} ${CUDA_INCLUDE_DIRS})
    target_link_libraries(sitku PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES} cublas)
    set_target_properties(sitku PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_property(TARGET sitku PROPERTY CXX_STANDARD "17")
    set_property(TARGET sitku PROPERTY CXX_STANDARD_REQUIRED ON)
    set_property(TARGET sitku PROPERTY CXX_EXTENSIONS OFF)

else ()
    add_executable(sitku user-query-relationship/SampleItemTopKUser.cpp)
    target_include_directories(sitku PRIVATE ${armadillo_SOURCE_DIR}/include ../src/include ${Boost_INCLUDE_DIRS})
    target_link_libraries(sitku PRIVATE armadillo BLAS::BLAS pthread gfortran OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})
endif ()