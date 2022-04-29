//
// Created by BianZheng on 2022/3/28.
//

#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/VectorMatrix.hpp"
#include "BPlusTree.hpp"
#include <boost/program_options.hpp>
#include <iostream>
#include <vector>
#include <string>


class Parameter {
public:
    std::string dataset_name, basic_dir;
};

void LoadOptions(int argc, char **argv, Parameter &para) {
    namespace po = boost::program_options;

    po::options_description opts("Allowed options");
    opts.add_options()
            ("help,h", "help info")
            ("dataset_name, ds", po::value<std::string>(&para.dataset_name)->default_value("fake"), "dataset_name")
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
    const char *method_name = "BPlusTree";
    spdlog::info("{} dataset_name {}, basic_dir {}", method_name, dataset_name, basic_dir);

    double total_build_index_time;
    char index_path[256];
    sprintf(index_path, "../index/index");

    int n_data_item, n_query_item, n_user, vec_dim;
    vector<VectorMatrix> data = readData(basic_dir, dataset_name, n_data_item, n_query_item, n_user,
                                         vec_dim);
    VectorMatrix &user = data[0];
    VectorMatrix &data_item = data[1];
    VectorMatrix &query_item = data[2];
    spdlog::info("n_data_item {}, n_query_item {}, n_user {}, vec_dim {}", n_data_item, n_query_item, n_user, vec_dim);

    TimeRecord record;
    record.reset();
    BPlusTree::Index &index = BPlusTree::BuildIndex(data_item, user, index_path);
    total_build_index_time = record.get_elapsed_time_second();
    spdlog::info("finish preprocess and save the index");

    vector<int> topk_l{70, 60, 50, 40, 30, 20, 10};
//    vector<int> topk_l{10};
    BPlusTree::RetrievalResult config;
    vector<vector<vector<UserRankElement>>> result_rank_l;
    for (const int &topk: topk_l) {
        record.reset();
        vector<vector<UserRankElement>> result_rk = index.Retrieval(query_item, topk);

        double retrieval_time = record.get_elapsed_time_second();
        double read_disk_time = index.read_disk_time_;
        double inner_product_time = index.inner_product_time_;
        double binary_search_time = index.binary_search_time_;
        double second_per_query = retrieval_time / n_query_item;

        result_rank_l.emplace_back(result_rk);
        string str = config.AddResultConfig(topk, retrieval_time, read_disk_time, inner_product_time,
                                            binary_search_time,
                                            second_per_query);
        spdlog::info("finish top-{}", topk);
    }

    spdlog::info("build index time: total {}s", total_build_index_time);
    int n_topk = (int) topk_l.size();
    for (int i = 0; i < n_topk; i++) {
        cout << config.config_l[i] << endl;
        WriteRankResult(result_rank_l[i], dataset_name, method_name);
    }

    config.AddPreprocess(total_build_index_time);
    config.WritePerformance(dataset_name, method_name);

    return 0;
}