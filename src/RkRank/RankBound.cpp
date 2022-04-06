//
// Created by BianZheng on 2022/2/25.
//

#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/VectorMatrix.hpp"
#include "RkRank/RankBound.hpp"
#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <vector>
#include <string>

class Parameter {
public:
    int cache_bound_every;
    std::string dataset_name, basic_dir;
};

void LoadOptions(int argc, char **argv, Parameter &para) {
    namespace po = boost::program_options;

    po::options_description opts("Allowed options");
    opts.add_options()
            ("help,h", "help info")
            ("dataset_name, ds", po::value<std::string>(&para.dataset_name), "dataset_name")
            ("cache_bound_every, cbe", po::value<int>(&para.cache_bound_every)->default_value(1000),
             "how many numbers would cache a value")
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
    const char *method_name = "RankBound";
    const char *problem_name = "RkRank";
    spdlog::info("{} {} dataset_name {}, basic_dir {} cache_bound_every {}", problem_name, method_name, dataset_name, basic_dir,
                 cache_bound_every);

    int n_data_item, n_query_item, n_user, vec_dim;
    vector <VectorMatrix> data = readData(basic_dir, dataset_name, n_data_item, n_query_item, n_user,
                                          vec_dim);
    VectorMatrix &user = data[0];
    VectorMatrix &data_item = data[1];
    VectorMatrix &query_item = data[2];
    spdlog::info("n_data_item {}, n_query_item {}, n_user {}, vec_dim {}", n_data_item, n_query_item, n_user, vec_dim);

    char index_path[256];
    sprintf(index_path, "../index/%s.index", dataset_name);

    TimeRecord record;
    record.reset();
    RankBound::Index &bscb = RankBound::BuildIndex(data_item, user, index_path, cache_bound_every);
    double build_index_time = record.get_elapsed_time_second();
    spdlog::info("finish preprocess and save the index");

    vector<int> topk_l{70, 60, 50, 40, 30, 20, 10};
//    vector<int> topk_l{20};
    RankBound::RetrievalResult config;
    vector < vector < vector < UserRankElement>>> result_rank_l;
    for (int topk: topk_l) {
        record.reset();
        vector <vector<UserRankElement>> result_rk = bscb.Retrieval(query_item, topk);

        double retrieval_time = record.get_elapsed_time_second();
        double read_disk_time = bscb.read_disk_time_;
        double inner_product_time = bscb.inner_product_time_;
        double coarse_binary_search_time = bscb.coarse_binary_search_time_;
        double fine_binary_search_time = bscb.fine_binary_search_time_;
        double prune_ratio = bscb.rank_prune_ratio_;
        double second_per_query = retrieval_time / n_query_item;

        result_rank_l.emplace_back(result_rk);
        string str = config.AddResultConfig(topk, retrieval_time, read_disk_time, inner_product_time,
                                            coarse_binary_search_time, fine_binary_search_time, prune_ratio,
                                            second_per_query);
        spdlog::info("finish top-{}", topk);
    }

    spdlog::info("build index time: total {}s", build_index_time);
    int n_topk = (int) topk_l.size();

    char other_name[256];
    sprintf(other_name, "cache_bound_every_%d", cache_bound_every);
    for (int i = 0; i < n_topk; i++) {
        cout << config.config_l[i] << endl;
//        writeRkRankResult(result_rank_l[i], dataset_name, method_name, other_name);
        writeRkRankResult(result_rank_l[i], dataset_name, method_name);
    }
    config.AddPreprocess(build_index_time);
//    config.writePerformance(problem_name, dataset_name, method_name, other_name);
    config.writePerformance(problem_name, dataset_name, method_name);
    return 0;
}