#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/VectorMatrix.hpp"
#include "RkRank/OnlineBruteForce.hpp"
#include <iostream>
#include <vector>
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
    const char *method_name = "OnlineBruteForce";
    const char *problem_name = "RkRank";
    spdlog::info("{} {} dataset_name {}, basic_dir {}", problem_name, method_name, dataset_name, basic_dir);

    int n_data_item, n_query_item, n_user, vec_dim;
    vector<VectorMatrix> data = readData(basic_dir, dataset_name, n_data_item, n_query_item, n_user,
                                         vec_dim);
    VectorMatrix &user = data[0];
    VectorMatrix &data_item = data[1];
    VectorMatrix &query_item = data[2];
    spdlog::info("n_data_item {}, n_query_item {}, n_user {}, vec_dim {}", n_data_item, n_query_item, n_user, vec_dim);

    TimeRecord record;
    record.reset();
    OnlineBruteForce::Index obf(data_item, user);
    obf.Preprocess();
    double preprocessed_time = record.get_elapsed_time_second();
    record.reset();
    spdlog::info("finish preprocess");

    vector<int> topk_l{70, 60, 50, 40, 30, 20, 10};
    OnlineBruteForce::RetrievalResult config;
    vector<vector<vector<UserRankElement>>> result_rank_l;
    for (int topk: topk_l) {
        record.reset();
        vector<vector<UserRankElement>> result_rk = obf.Retrieval(query_item, topk);

        double retrieval_time = record.get_elapsed_time_second();
        double second_per_query = retrieval_time / n_query_item;

        result_rank_l.emplace_back(result_rk);
        string str = config.AddResultConfig(topk, retrieval_time, second_per_query);
        spdlog::info("{}", str);
    }


    spdlog::info("build index time: total {}s", preprocessed_time);
    int n_topk = (int) topk_l.size();
    for (int i = 0; i < n_topk; i++) {
        cout << config.config_l[i] << endl;
        writeRkRankResult(result_rank_l[i], dataset_name, method_name);
    }
    config.AddPreprocess(preprocessed_time);
    config.writePerformance(problem_name, dataset_name, method_name);
    return 0;
}