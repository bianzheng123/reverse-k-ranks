//
// Created by BianZheng on 2022/3/21.
//

#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/VectorMatrix.hpp"
#include "IntervalSearch/IRBPartDimPartIntPrune.hpp"
#include <spdlog/spdlog.h>
#include <iostream>
#include <vector>
#include <string>

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
    spdlog::info("IRBPartDimPartIntPrune dataset_name {}, basic_dir {}", dataset_name, basic_dir);

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
    IntervalRankBound::Index &irb = IntervalRankBound::BuildIndex(user, data_item, index_path);
    double build_index_time = record.get_elapsed_time_second();
    spdlog::info("finish preprocess and save the index");

    vector<int> topk_l{50, 40, 30, 20, 10};
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
        double binary_search_prune_ratio = irb.binary_search_prune_ratio_;
        double second_per_query = retrieval_time / n_query_item;

        result_rank_l.emplace_back(result_rk);
        config.AddResultConfig(topk, retrieval_time, interval_search_time, inner_product_time,
                               coarse_binary_search_time, read_disk_time, fine_binary_search_time,
                               interval_prune_ratio,
                               binary_search_prune_ratio,
                               second_per_query);
        spdlog::info("finish top-{}", topk);
    }

    spdlog::info("build index time: total {}s", build_index_time);
    int n_topk = (int) topk_l.size();
    for (int i = 0; i < n_topk; i++) {
        cout << config.config_l[i] << endl;
        writeRank(result_rank_l[i], dataset_name, "IRBPartDimPartIntPrune");
    }

    config.AddPreprocess(build_index_time);
    config.writePerformance(dataset_name, "IRBPartDimPartIntPrune");
    return 0;
}