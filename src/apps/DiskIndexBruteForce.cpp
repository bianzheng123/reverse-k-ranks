//
// Created by BianZheng on 2021/12/22.
//

#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/RankElement.hpp"
#include "struct/VectorMatrix.hpp"
#include "DiskIndexBruteForce.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <map>

//预处理时不做任何动作, 在线计算全部的向量, 然后返回最大的k个rank

using namespace std;
using namespace ReverseMIPS;


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
    printf("dataset_name %s, basic_dir %s\n", dataset_name, basic_dir);

    double preprocess_time, total_preprocess_time;
    char index_path[256];
    sprintf(index_path, "../index/%s.bfi", dataset_name);
    {
        int n_data_item, n_query_item, n_user, vec_dim;
        vector <unique_ptr<float[]>> data = readData(basic_dir, dataset_name, n_data_item, n_query_item, n_user,
                                                     vec_dim);
        float *data_item_ptr = data[0].get();
        float *user_ptr = data[1].get();
        float *query_item_ptr = data[2].get();

        VectorMatrix data_item, user, query_item;
        data_item.init(data_item_ptr, n_data_item, vec_dim);
        user.init(user_ptr, n_user, vec_dim);

        TimeRecord record;
        record.reset();
        preprocess_time = BuildSaveIndex(data_item, user, index_path);
        total_preprocess_time = record.get_elapsed_time_micro() * 1e-6;
        printf("finish preprocess and save the index\n");
    }

    char filename[256];
    sprintf(filename, "%s/%s/%s_query_item.fvecs", basic_dir, dataset_name, dataset_name);
    int n_query_item, vec_dim;
    std::unique_ptr<float[]> query_item_uptr = loadVector<float>(filename, n_query_item, vec_dim);
    float *query_item_ptr = query_item_uptr.get();

    sprintf(filename, "%s/%s/%s_user.fvecs", basic_dir, dataset_name, dataset_name);
    int n_user;
    std::unique_ptr<float[]> user_item_uptr = loadVector<float>(filename, n_user, vec_dim);
    float *user_ptr = user_item_uptr.get();

    VectorMatrix query_item, user;
    query_item.init(query_item_ptr, n_query_item, vec_dim);
    user.init(user_ptr, n_user, vec_dim);

    DiskIndexBruteForce dibf(index_path, user);
    TimeRecord record;
    record.reset();
    vector <vector<RankElement>> result = dibf.Retrieval(query_item, topk);

    double ip_calc_time = dibf.inner_product_calculation_time_;
    double binary_search_time = dibf.binary_search_time_;
    double retrieval_time = record.get_elapsed_time_micro() * 1e-6;

    printf("total preprocess time %.3fs, preprocessed calculation time %.3fs, total retrieval time %.3fs\n",
           total_preprocess_time,
           preprocess_time, retrieval_time);
    printf("inner product calculation time %.3fs, binary search time %.3fs\n", ip_calc_time, binary_search_time);
    writeRank(result, dataset_name, "DiskIndexBruteForce");

    map<string, string> performance_m;
    performance_m.emplace("total preprocess time", double2string(total_preprocess_time));
    performance_m.emplace("preprocess time", double2string(preprocess_time));
    performance_m.emplace("retrieval time", double2string(retrieval_time));
    performance_m.emplace("inner product calculation time", double2string(ip_calc_time));
    performance_m.emplace("binary search time", double2string(binary_search_time));
    writePerformance(dataset_name, "DiskIndexBruteForce", performance_m);

    return 0;
}