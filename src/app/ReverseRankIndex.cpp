//
// Created by bianzheng on 2022/4/29.
//

#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/VectorMatrix.hpp"

#include "BruteForce/CompressTopTIDBruteForce.hpp"
#include "BruteForce/CompressTopTIDIPBruteForce.hpp"
#include "BruteForce/CompressTopTIPBruteForce.hpp"
#include "BruteForce/MemoryBruteForce.hpp"

#include "Online/GridIndex.hpp"

#include "RankSample.hpp"
#include "ScoreSample.hpp"
#include "SSComputeAll.hpp"
#include "SSMergeIntervalIDByBitmap.hpp"
#include "SSMergeIntervalIDByInterval.hpp"
#include "SSMergeQuadraticRankBoundByBitmap.hpp"
#include "SSMergeRankByInterval.hpp"

#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <vector>
#include <string>

class Parameter {
public:
    std::string basic_dir, dataset_name, method_name;
    int cache_bound_every, n_sample;
    uint64_t index_size_gb;
};

void LoadOptions(int argc, char **argv, Parameter &para) {
    namespace po = boost::program_options;

    po::options_description opts("Allowed options");
    opts.add_options()
            ("help,h", "help info")
            ("basic_dir,bd",
             po::value<std::string>(&para.basic_dir)->default_value("/home/bianzheng/Dataset/ReverseMIPS"),
             "basic directory")
            ("dataset_name, ds", po::value<std::string>(&para.dataset_name)->default_value("fake-normal"),
             "dataset_name")
            ("method_name, mn", po::value<std::string>(&para.method_name)->default_value("BatchDiskBruteForce"),
             "method_name")

            ("cache_bound_every, cbe", po::value<int>(&para.cache_bound_every)->default_value(512),
             "how many numbers would cache a value")
            ("n_sample, ns", po::value<int>(&para.n_sample)->default_value(20),
             "the numer of sample")
            ("index_size_gb, tt", po::value<uint64_t>(&para.index_size_gb)->default_value(50),
             "index size, in unit of GB");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, opts), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << opts << std::endl;
        exit(0);
    }
}

using namespace std;
using namespace ReverseMIPS;


int main(int argc, char **argv) {
    Parameter para;
    LoadOptions(argc, argv, para);
    const char *dataset_name = para.dataset_name.c_str();
    const char *basic_dir = para.basic_dir.c_str();
    string method_name = para.method_name;
    spdlog::info("{} dataset_name {}, basic_dir {}", method_name, dataset_name, basic_dir);

    int n_data_item, n_query_item, n_user, vec_dim;
    vector<VectorMatrix> data = readData(basic_dir, dataset_name, n_data_item, n_query_item, n_user,
                                         vec_dim);
    VectorMatrix &user = data[0];
    VectorMatrix &data_item = data[1];
    VectorMatrix &query_item = data[2];
    spdlog::info("n_data_item {}, n_query_item {}, n_user {}, vec_dim {}", n_data_item, n_query_item, n_user, vec_dim);

    char index_path[256];
    sprintf(index_path, "../index/index");

    TimeRecord record;
    record.reset();
    unique_ptr<BaseIndex> index;
    char parameter_name[256] = "";
    if (method_name == "CompressTopTIDBruteForce") {
        const int n_sample = para.n_sample;
        const uint64_t index_size_gb = para.index_size_gb;
        spdlog::info("input parameter: n_sample {}, index_size_gb {}",
                     n_sample, index_size_gb);
        index = CompressTopTIDBruteForce::BuildIndex(data_item, user, index_path,
                                                     n_sample, index_size_gb);
        sprintf(parameter_name, "n_sample_%d-index_size_gb_%lu",
                n_sample, index_size_gb);

    } else if (method_name == "CompressTopTIDIPBruteForce") {
        const int n_sample = para.n_sample;
        const uint64_t index_size_gb = para.index_size_gb;
        spdlog::info("input parameter: n_sample {}, index_size_gb {}",
                     n_sample, index_size_gb);
        index = CompressTopTIDIPBruteForce::BuildIndex(data_item, user, index_path,
                                                       n_sample, index_size_gb);
        sprintf(parameter_name, "n_sample_%d-index_size_gb_%lu",
                n_sample, index_size_gb);

    } else if (method_name == "CompressTopTIPBruteForce") {
        const int n_sample = para.n_sample;
        const uint64_t index_size_gb = para.index_size_gb;
        spdlog::info("input parameter: n_sample {}, index_size_gb {}",
                     n_sample, index_size_gb);
        index = CompressTopTIPBruteForce::BuildIndex(data_item, user, index_path,
                                                     n_sample, index_size_gb);
        sprintf(parameter_name, "n_sample_%d-index_size_gb_%lu",
                n_sample, index_size_gb);

    } else if (method_name == "GridIndex") {
        ///Online
        spdlog::info("input parameter: none");
        index = GridIndex::BuildIndex(data_item, user);

    } else if (method_name == "RankSample") {
        const int cache_bound_every = para.cache_bound_every;
        spdlog::info("input parameter: cache_bound_every {}", cache_bound_every);
        index = RankSample::BuildIndex(data_item, user, index_path, cache_bound_every);
        sprintf(parameter_name, "cache_bound_every_%d", cache_bound_every);

    } else if (method_name == "ScoreSample") {
        const int n_sample = para.n_sample;
        spdlog::info("input parameter: n_sample {}", n_sample);
        index = ScoreSample::BuildIndex(data_item, user, index_path, n_sample);
        sprintf(parameter_name, "n_sample_%d", n_sample);

    } else if (method_name == "SSComputeAll") {
        const int n_sample = para.n_sample;
        spdlog::info("input parameter: n_sample {}", n_sample);
        index = SSComputeAll::BuildIndex(data_item, user, index_path, n_sample);
        sprintf(parameter_name, "n_sample_%d", n_sample);

    } else if (method_name == "SSMergeIntervalIDByBitmap") {
        const int n_sample = para.n_sample;
        const uint64_t index_size_gb = para.index_size_gb;
        spdlog::info("input parameter: n_sample {}, index_size_gb {}",
                     n_sample, index_size_gb);
        index = SSMergeIntervalIDByBitmap::BuildIndex(data_item, user, index_path,
                                                      n_sample, index_size_gb);
        sprintf(parameter_name, "n_sample_%d-index_size_gb_%lu",
                n_sample, index_size_gb);

    } else if (method_name == "SSMergeIntervalIDByInterval") {
        const int n_sample = para.n_sample;
        const uint64_t index_size_gb = para.index_size_gb;
        spdlog::info("input parameter: n_sample {}, index_size_gb {}",
                     n_sample, index_size_gb);
        index = SSMergeIntervalIDByInterval::BuildIndex(data_item, user, index_path,
                                                        n_sample, index_size_gb);
        sprintf(parameter_name, "n_sample_%d-index_size_gb_%lu",
                n_sample, index_size_gb);

    } else if (method_name == "SSMergeQuadraticRankBoundByBitmap") {
        const int n_sample = para.n_sample;
        const uint64_t index_size_gb = para.index_size_gb;
        spdlog::info("input parameter: n_sample {}, index_size_gb {}",
                     n_sample, index_size_gb);
        index = SSMergeQuadraticRankBoundByBitmap::BuildIndex(data_item, user, index_path,
                                                              n_sample, index_size_gb);
        sprintf(parameter_name, "n_sample_%d-index_size_gb_%lu",
                n_sample, index_size_gb);

    } else if (method_name == "SSMergeRankByInterval") {
        const int n_sample = para.n_sample;
        const uint64_t index_size_gb = para.index_size_gb;
        spdlog::info("input parameter: n_sample {}, index_size_gb {}",
                     n_sample, index_size_gb);
        index = SSMergeRankByInterval::BuildIndex(data_item, user, index_path,
                                                  n_sample, index_size_gb);
        sprintf(parameter_name, "n_sample_%d-index_size_gb_%lu",
                n_sample, index_size_gb);

    } else {
        spdlog::error("not such method");
    }

    double build_index_time = record.get_elapsed_time_second();
    spdlog::info("finish preprocess and save the index");

    vector<int> topk_l{70, 60, 50, 40, 30, 20, 10};
//    vector<int> topk_l{30, 20, 10};
//    vector<int> topk_l{10000, 8192, 4096, 2048, 1024, 512, 256, 128, 64, 32, 16, 8};
//    vector<int> topk_l{20};
    RetrievalResult config;
    vector<vector<vector<UserRankElement>>> result_rank_l;
    for (int topk: topk_l) {
        record.reset();
        vector<vector<UserRankElement>> result_rk = index->Retrieval(query_item, topk);

        double retrieval_time = record.get_elapsed_time_second();
        double ms_per_query = retrieval_time / n_query_item * 1000;

        string performance_str = index->PerformanceStatistics(topk, retrieval_time, ms_per_query);
        config.AddRetrievalInfo(performance_str, topk, retrieval_time, ms_per_query);

        result_rank_l.emplace_back(result_rk);
        spdlog::info("finish top-{}", topk);
        spdlog::info("{}", performance_str);
    }

    spdlog::info("build index time: total {}s", build_index_time);
    int n_topk = (int) topk_l.size();

    for (int i = 0; i < n_topk; i++) {
        cout << config.GetConfig(i) << endl;
        WriteRankResult(result_rank_l[i], dataset_name, method_name.c_str(), parameter_name);
    }

    config.AddBuildIndexInfo(index->BuildIndexStatistics());
    config.AddBuildIndexTime(build_index_time);
    config.WritePerformance(dataset_name, method_name.c_str(), parameter_name);
    return 0;
}