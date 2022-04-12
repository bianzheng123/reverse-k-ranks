//
// Created by BianZheng on 2022/3/17.
//

#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/VectorMatrix.hpp"
#include "IntervalRankBound.hpp"
#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <vector>
#include <string>

class Parameter {
public:
    int cache_bound_every, n_interval;
    std::string dataset_name, basic_dir;
};

void LoadOptions(int argc, char **argv, Parameter &para) {
    namespace po = boost::program_options;

    po::options_description opts("Allowed options");
    opts.add_options()
            ("help,h", "help info")
            ("dataset_name, ds", po::value<std::string>(&para.dataset_name)->default_value("fake"), "dataset_name")
            ("cache_bound_every, cbe", po::value<int>(&para.cache_bound_every)->default_value(1000),
             "how many numbers would cache a value")
            ("n_interval, nitv", po::value<int>(&para.n_interval)->default_value(1024),
             "the numer of interval")
            ("basic_dir,bd",
             po::value<std::string>(&para.basic_dir)->default_value("/home/bianzheng/Dataset/ReverseMIPS"),
             "basic directory");

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
    const int cache_bound_every = para.cache_bound_every;
    const int n_interval = para.n_interval;
    const char *method_name = "IntervalRankBound";

    spdlog::info("{} dataset_name {}, basic_dir {}, cache_bound_every {}, n_interval {}", method_name,
                 dataset_name, basic_dir, cache_bound_every, n_interval);

    int n_data_item, n_query_item, n_user, vec_dim;
    vector<VectorMatrix> data = readData(basic_dir, dataset_name, n_data_item, n_query_item, n_user,
                                         vec_dim);
    VectorMatrix &user = data[0];
    VectorMatrix &data_item = data[1];
    VectorMatrix &query_item = data[2];
    spdlog::info("n_data_item {}, n_query_item {}, n_user {}, vec_dim {}", n_data_item, n_query_item, n_user, vec_dim);

    char index_path[256];
    sprintf(index_path, "../index/%s.index", dataset_name);

    TimeRecord record;
    record.reset();
    IntervalRankBound::Index &irb = IntervalRankBound::BuildIndex(user, data_item, index_path, cache_bound_every,
                                                                  n_interval);
    double build_index_time = record.get_elapsed_time_second();
    spdlog::info("finish preprocess and save the index");

    vector<int> topk_l{70, 60, 50, 40, 30, 20, 10};
//    vector<int> topk_l{10};
    IntervalRankBound::RetrievalResult config;
    vector<vector<vector<UserRankElement>>> result_rank_l;
    for (const int &topk: topk_l) {
        record.reset();
        vector<vector<UserRankElement>> result_rk = irb.Retrieval(query_item, topk);

        double retrieval_time = record.get_elapsed_time_second();
        double interval_search_time = irb.interval_search_time_;
        double inner_product_time = irb.inner_product_time_;
        double coarse_binary_search_time = irb.coarse_binary_search_time_;
        double read_disk_time = irb.read_disk_time_;
        double fine_binary_search_time = irb.fine_binary_search_time_;

        double interval_prune_ratio = irb.interval_prune_ratio_;
        double rank_search_prune_ratio = irb.rank_search_prune_ratio_;
        double second_per_query = retrieval_time / n_query_item;

        result_rank_l.emplace_back(result_rk);
        config.AddResultConfig(topk, retrieval_time, interval_search_time, inner_product_time,
                               coarse_binary_search_time, read_disk_time, fine_binary_search_time,
                               interval_prune_ratio,
                               rank_search_prune_ratio,
                               second_per_query);
        spdlog::info("finish top-{}", topk);
    }

    spdlog::info("build index time: total {}s", build_index_time);
    int n_topk = (int) topk_l.size();
    for (int i = 0; i < n_topk; i++) {
        cout << config.config_l[i] << endl;
        writeRankResult(result_rank_l[i], dataset_name, method_name);
    }

    config.AddPreprocess(build_index_time);
    config.writePerformance(dataset_name, method_name);
    return 0;
}