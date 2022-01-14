//
// Created by BianZheng on 2021/12/27.
//

#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/RankElement.hpp"
#include "struct/VectorMatrix.hpp"
#include "struct/SetVector.hpp"
#include "RankInterval.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <map>

//预处理时不做任何动作, 在线计算全部的向量, 然后返回最大的k个rank

using namespace std;
using namespace ReverseMIPS;

/*
 * 首先进行merge用户, 然后建立索引, 根据指定的方向进行merge
 */

int main(int argc, char **argv) {
    if (!(argc == 3 or argc == 4)) {
        cout << argv[0] << " dataset_name top-k [basic_dir]" << endl;
        return 0;
    }
    const char *dataset_name = argv[1];
    int topk = atoi(argv[2]);
    const char *basic_dir = "/home/bianzheng/Dataset/MIPS/Reverse-kRanks";
    if (argc == 4) {
        basic_dir = argv[3];
    }
    printf("dataset_name %s, topk %d, basic_dir %s\n", dataset_name, topk, basic_dir);

    double index_time;
    char optimal_index_path[256];
    sprintf(optimal_index_path, "../index/sample_%s/%s.opti", dataset_name, dataset_name);
    {
        int n_data_item, n_query_item, n_user, vec_dim;
        vector<unique_ptr < float[]>>
        data = readData(basic_dir, dataset_name, n_data_item, n_query_item, n_user,
                        vec_dim);
        float *data_item_ptr = data[0].get();
        float *user_ptr = data[1].get();

        VectorMatrix data_item, user;
        data_item.init(data_item_ptr, n_data_item, vec_dim);
        user.init(user_ptr, n_user, vec_dim);

        BuildIndex(user, data_item, dataset_name, index_time);
        printf("finish build bruteforce index and save the index\n");
    }

//    char filename[256];
//    sprintf(filename, "%s/%s/%s_query_item.fvecs", basic_dir, dataset_name, dataset_name);
//    int n_query_item, vec_dim;
//    std::unique_ptr<float[]> query_item_uptr = loadVector<float>(filename, n_query_item, vec_dim);
//    float *query_item_ptr = query_item_uptr.get();
//
//    sprintf(filename, "%s/%s/%s_user.fvecs", basic_dir, dataset_name, dataset_name);
//    int n_user;
//    std::unique_ptr<float[]> user_item_uptr = loadVector<float>(filename, n_user, vec_dim);
//    float *user_ptr = user_item_uptr.get();
//
//    VectorMatrix query_item, user;
//    query_item.init(query_item_ptr, n_query_item, vec_dim);
//    user.init(user_ptr, n_user, vec_dim);
//
//    SampleIndex si(user, set_vector_l, set_vector_idx_l, distance_pair_l);
//    TimeRecord record;
//    record.reset();
//    vector<vector<RankElement>> result = si.Retrieval(query_item, topk);
//
//    double retrieval_time = record.get_elapsed_time_micro() * 1e-6;
//
//    printf("bruteforce index time %.3fs, build optimal index time %.3fs, total retrieval time %.3fs\n",
//           bruteforce_index_time, build_optimal_index_time, retrieval_time);
//    writeRank(result, dataset_name, "DiskIndexBruteForce");
//
//    map<string, string> performance_m;
//    performance_m.emplace("bruteforce index time", double2string(bruteforce_index_time));
//    performance_m.emplace("optimal index time", double2string(build_optimal_index_time));
//    performance_m.emplace("retrieval time", double2string(retrieval_time));
//    writePerformance(dataset_name, "SampleIndex", performance_m);

    return 0;
}