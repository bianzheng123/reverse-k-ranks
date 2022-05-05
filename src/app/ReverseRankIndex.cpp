//
// Created by bianzheng on 2022/4/29.
//

#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/VectorMatrix.hpp"

#include "BruteForce/BatchDiskBruteForce.hpp"
#include "BruteForce/BPlusTree.hpp"
#include "BruteForce/CompressTopTIDIPBruteForce.hpp"
#include "BruteForce/CompressTopTIPBruteForce.hpp"
#include "BruteForce/DiskBruteForce.hpp"
#include "BruteForce/MemoryBruteForce.hpp"
#include "BruteForce/OnlineBruteForce.hpp"
#include "IntervalRankBound.hpp"
#include "RankBound.hpp"

#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <vector>
#include <string>

class Parameter {
public:
    std::string basic_dir, dataset_name, method_name;
    int cache_bound_every, n_interval, n_merge_user, topt_perc;
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
            ("n_interval, nitv", po::value<int>(&para.n_interval)->default_value(1024),
             "the numer of interval")
            ("n_merge_user, nmu", po::value<int>(&para.n_merge_user)->default_value(2),
             "the numer of merged user")
            ("topt_perc, ttp", po::value<int>(&para.topt_perc)->default_value(50),
             "store percent of top-t inner product as index");

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
    if (method_name == "BatchDiskBruteForce") {
        spdlog::info("input parameter: none");
        index = BatchDiskBruteForce::BuildIndex(data_item, user, index_path);
    } else if (method_name == "BPlusTree") {
        spdlog::info("input parameter: none");
        index = BPlusTree::BuildIndex(data_item, user, index_path);
    } else if (method_name == "CompressTopTIDIPBruteForce") {
        const int cache_bound_every = para.cache_bound_every;
        const int n_interval = para.n_interval;
        const int topt_perc = para.topt_perc;
        spdlog::info("input parameter: cache_bound_every {}, n_interval {}, topt_perc {}",
                     cache_bound_every, n_interval, topt_perc);
        index = CompressTopTIDIPBruteForce::BuildIndex(data_item, user, index_path, cache_bound_every, n_interval,
                                                       topt_perc);
        sprintf(parameter_name, "cache_bound_every_%d-n_interval_%d-topt_perc_%d", cache_bound_every, n_interval,
                topt_perc);
    } else if (method_name == "CompressTopTIPBruteForce") {
        const int cache_bound_every = para.cache_bound_every;
        const int n_interval = para.n_interval;
        const int topt_perc = para.topt_perc;
        spdlog::info("input parameter: cache_bound_every {}, n_interval {}, topt_perc {}",
                     cache_bound_every, n_interval, topt_perc);
        index = CompressTopTIPBruteForce::BuildIndex(data_item, user, index_path, cache_bound_every, n_interval,
                                                     topt_perc);
        sprintf(parameter_name, "cache_bound_every_%d-n_interval_%d-topt_perc_%d", cache_bound_every, n_interval,
                topt_perc);
    } else if (method_name == "DiskBruteForce") {
        spdlog::info("input parameter: none");
        index = DiskBruteForce::BuildIndex(data_item, user, index_path);
    } else if (method_name == "MemoryBruteForce") {
        spdlog::info("input parameter: none");
        index = MemoryBruteForce::BuildIndex(data_item, user);
    } else if (method_name == "OnlineBruteForce") {
        spdlog::info("input parameter: none");
        index = OnlineBruteForce::BuildIndex(data_item, user);
    } else if (method_name == "IntervalRankBound") {
        const int cache_bound_every = para.cache_bound_every;
        const int n_interval = para.n_interval;
        spdlog::info("input parameter: cache_bound_every {}, n_interval {}",
                     cache_bound_every, n_interval);
        index = IntervalRankBound::BuildIndex(data_item, user, index_path, cache_bound_every, n_interval);
        sprintf(parameter_name, "cache_bound_every_%d-n_interval_%d", cache_bound_every, n_interval);

    } else if (method_name == "RankBound") {
        const int cache_bound_every = para.cache_bound_every;
        spdlog::info("input parameter: cache_bound_every {}", cache_bound_every);
        index = RankBound::BuildIndex(data_item, user, index_path, cache_bound_every);
        sprintf(parameter_name, "cache_bound_every_%d", cache_bound_every);

    } else {
        spdlog::error("not such method");
    }

    double build_index_time = record.get_elapsed_time_second();
    spdlog::info("finish preprocess and save the index");

    vector<int> topk_l{70, 60, 50, 40, 30, 20, 10};
//    vector<int> topk_l{20};
    RetrievalResult config;
    vector<vector<vector<UserRankElement>>> result_rank_l;
    for (int topk: topk_l) {
        record.reset();
        vector<vector<UserRankElement>> result_rk = index->Retrieval(query_item, topk);

        double retrieval_time = record.get_elapsed_time_second();
        double second_per_query = retrieval_time / n_query_item;

        string performance_str = index->PerformanceStatistics(topk, retrieval_time, second_per_query);
        config.config_l.push_back(performance_str);

        result_rank_l.emplace_back(result_rk);
        spdlog::info("finish top-{}", topk);
    }

    spdlog::info("build index time: total {}s", build_index_time);
    int n_topk = (int) topk_l.size();

    for (int i = 0; i < n_topk; i++) {
        cout << config.config_l[i] << endl;
        WriteRankResult(result_rank_l[i], dataset_name, method_name.c_str(), parameter_name);
    }

    config.AddBuildIndexInfo(index->BuildIndexStatistics());
    config.AddBuildIndexTime(build_index_time);
    config.WritePerformance(dataset_name, method_name.c_str(), parameter_name);
    return 0;
}