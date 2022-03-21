//
// Created by BianZheng on 2022/2/25.
//

#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/VectorMatrix.hpp"
#include "DiskBruteForce.hpp"
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
    spdlog::info("DiskBruteForce dataset_name {}, basic_dir {}", dataset_name, basic_dir);

    double total_build_index_time;
    char index_path[256];
    sprintf(index_path, "../index/%s.index", dataset_name);

    int n_data_item, n_query_item, n_user, vec_dim;
    vector<VectorMatrix> data = readData(basic_dir, dataset_name, n_data_item, n_query_item, n_user,
                                         vec_dim);
    VectorMatrix &user = data[0];
    user.vectorNormalize();
    VectorMatrix &data_item = data[1];
    VectorMatrix &query_item = data[2];
    spdlog::info("n_data_item {}, n_query_item {}, n_user {}, vec_dim {}", n_data_item, n_query_item, n_user, vec_dim);
    int userID = 254;
    int queryID = 0;
    printf("userID %d, queryID %d, ip %.3f\n", userID, queryID,
           InnerProduct(user.getVector(userID), query_item.getVector(queryID), vec_dim));

    return 0;
}