//
// Created by BianZheng on 2022/2/28.
//

#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/VectorMatrix.hpp"
#include "CauchyPercentile.hpp"
#include <iostream>
#include <vector>

void AttributionWriteRank(std::vector<std::vector<ReverseMIPS::UserRankElement>> &result, const char *dataset_name,
                          const char *method_name) {
    int n_query_item = (int) result.size();
    int topk = (int) result[0].size();

    char resPath[256];
    std::sprintf(resPath, "../../result/attribution/CauchyPercentile/%s-%s-all-index.csv", dataset_name, method_name);
    std::ofstream file(resPath);
    if (!file) {
        std::printf("error in write result\n");
    }

    for (int i = 0; i < n_query_item; i++) {
        for (int j = 0; j < topk - 1; j++) {
            file << result[i][j].userID_ << ",";
        }
        file << result[i][topk - 1].userID_ << std::endl;
    }
    file.close();

    const int output_topk = 10;

    std::sprintf(resPath, "../../result/attribution/CauchyPercentile/%s-%s-top%d-rank.csv", dataset_name, method_name, output_topk);
    file.open(resPath);
    if (!file) {
        std::printf("error in write result\n");
    }

    for (int i = 0; i < n_query_item; i++) {
        for (int j = 0; j < output_topk - 1; j++) {
            file << result[i][j].rank_ << ",";
        }
        file << result[i][output_topk - 1].rank_ << std::endl;
    }
    file.close();

    std::sprintf(resPath, "../../result/attribution/CauchyPercentile/%s-%s-top%d-IP.csv", dataset_name, method_name, output_topk);
    file.open(resPath);
    if (!file) {
        std::printf("error in write result\n");
    }

    for (int i = 0; i < n_query_item; i++) {
        for (int j = 0; j < output_topk - 1; j++) {
            file << result[i][j].queryIP_ << ",";
        }
        file << result[i][output_topk - 1].queryIP_ << std::endl;
    }
    file.close();
}

void AttributionWriteEvaluationSequence(std::vector<ReverseMIPS::CauchyPercentile::Bound> &bound_l,
                                        std::vector<int> &eval_seq_l, const char *dataset_name,
                                        const char *method_name) {
    int bound_len = bound_l.size();

    char resPath[256];
    std::sprintf(resPath, "../../result/attribution/CauchyPercentile/%s-%s-bound.txt", dataset_name, method_name);
    std::ofstream file(resPath);
    if (!file) {
        std::printf("error in write result\n");
    }

    for (int i = 0; i < bound_len; i++) {
        file << bound_l[i].lower_bound_ << " " << bound_l[i].upper_bound_ << std::endl;
    }
    file.close();

    int n_eval = eval_seq_l.size();

    std::sprintf(resPath, "../../result/attribution/CauchyPercentile/%s-%s-evaluation-sequence.txt", dataset_name, method_name);
    file.open(resPath);
    if (!file) {
        std::printf("error in write result\n");
    }

    for (int i = 0; i < n_eval; i++) {
        file << eval_seq_l[i] << std::endl;
    }
    file.close();
}

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
    printf("CauchyPercentile dataset_name %s, basic_dir %s\n", dataset_name, basic_dir);

    double total_build_index_time;

    int n_data_item, n_query_item, n_user, vec_dim;
    vector<VectorMatrix> data = readData(basic_dir, dataset_name, n_data_item, n_query_item, n_user,
                                         vec_dim);
    VectorMatrix &user = data[0];
    VectorMatrix &data_item = data[1];
    VectorMatrix &query_item = data[2];
    spdlog::info("n_data_item {}, n_query_item {}, n_user {}, vec_dim {}", n_data_item, n_query_item, n_user, vec_dim);

    vector<ReverseMIPS::CauchyPercentile::Bound> bound_l(n_user);

    ReverseMIPS::TimeRecord record;
    record.reset();
    vector<vector<ReverseMIPS::UserRankElement>> result_rk = GetGroundTruth(user, data_item, query_item, bound_l);
    vector<int> eval_seq_l = EvaluationSequence(bound_l);
    total_build_index_time = record.get_elapsed_time_second();
    printf("finish preprocess and save the index\n");

    AttributionWriteRank(result_rk, dataset_name, "CauchyPercentile");
    AttributionWriteEvaluationSequence(bound_l, eval_seq_l, dataset_name, "CauchyPercentile");

    printf("process time: total %.3fs\n", total_build_index_time);

    return 0;
}