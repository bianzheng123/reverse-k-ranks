#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/VectorMatrix.hpp"
#include "MemoryBruteForce.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <spdlog/spdlog.h>

using namespace std;
using namespace ReverseMIPS;

int main(int argc, char **argv) {
    if (!(argc == 2 or argc == 3)) {
        cout << argv[0] << " dataset_name [basic_dir]" << endl;
        return 0;
    }
    const char *dataset_name = argv[1];
    const char *basic_dir = "/home/bianzheng/Dataset/ReverseMIPS";
    if (argc == 3) {
        basic_dir = argv[2];
    }
    const char *method_name = "MemoryBruteForce";
    spdlog::info("{} dataset_name {}, basic_dir {}", method_name, dataset_name, basic_dir);

    int n_data_item, n_query_item, n_user, vec_dim;
    vector<VectorMatrix> data = readData(basic_dir, dataset_name, n_data_item, n_query_item, n_user,
                                         vec_dim);
    VectorMatrix &user = data[0];
    VectorMatrix &data_item = data[1];
    VectorMatrix &query_item = data[2];
    spdlog::info("n_data_item {}, n_query_item {}, n_user {}, vec_dim {}", n_data_item, n_query_item, n_user, vec_dim);

    TimeRecord record;
    record.reset();
    MemoryBruteForce::Index mibf(data_item, user);
    mibf.Preprocess();
    double preprocessed_time = record.get_elapsed_time_second();
    spdlog::info("finish preprocessing");

    vector<int> topk_l{70, 60, 50, 40, 30, 20, 10};
//    vector<int> topk_l{10};
    MemoryBruteForce::RetrievalResult config;
    vector<vector<vector<UserRankElement>>> result_rank_l;
    for (int topk: topk_l) {
        record.reset();
        vector<vector<UserRankElement>> result_rk = mibf.Retrieval(query_item, topk);

        double retrieval_time = record.get_elapsed_time_second();
        double ip_calc_time = mibf.inner_product_time_;
        double binary_search_time = mibf.binary_search_time_;
        double second_per_query = retrieval_time / n_query_item;

        result_rank_l.emplace_back(result_rk);
        string str = config.AddResultConfig(topk, retrieval_time, ip_calc_time, binary_search_time, second_per_query);
        spdlog::info("{}", str);
    }

    spdlog::info("build index time: total {}s", preprocessed_time);
    int n_topk = (int) topk_l.size();

    for (int i = 0; i < n_topk; i++) {
        cout << config.config_l[i] << endl;
        writeRankResult(result_rank_l[i], dataset_name, method_name);
    }

    config.AddPreprocess(preprocessed_time);

    config.writePerformance(dataset_name, method_name);
    return 0;
}