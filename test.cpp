//
// Created by BianZheng on 2022/3/11.
//

#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/VectorMatrix.hpp"
#include "alg/SVD.hpp"
#include "alg/SpaceInnerProduct.hpp"
#include <iostream>
#include <vector>
#include <spdlog/spdlog.h>

using namespace std;
using namespace ReverseMIPS;

//output bound tightness and time consumption
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
    printf("FullCauchy dataset_name %s, basic_dir %s\n", dataset_name, basic_dir);

    int n_data_item, n_query_item, n_user, vec_dim;
    vector<VectorMatrix> data = readData(basic_dir, dataset_name, n_data_item, n_query_item, n_user,
                                         vec_dim);
    VectorMatrix &user = data[0];
    VectorMatrix &data_item = data[1];
    VectorMatrix &query_item = data[2];
    spdlog::info("n_data_item {}, n_query_item {}, n_user {}, vec_dim {}", n_data_item, n_query_item, n_user,
                 vec_dim);

    TimeRecord record;
    record.reset();
    double avg_double = 0;
    for (int userID = 0; userID < n_user; userID++) {
        double *user_vecs = user.getVector(userID);
        for (int itemID = 0; itemID < n_data_item; itemID++) {
            double *item_vecs = data_item.getVector(itemID);
            double IP = InnerProduct(user_vecs, item_vecs, vec_dim);
            avg_double += IP;
        }
    }
    double double_time = record.get_elapsed_time_second();

    unique_ptr<int[]> user_int_ptr = make_unique<int[]>(n_user * vec_dim);

    for (int userID = 0; userID < n_user; userID++) {
        int *user_int_vecs = user_int_ptr.get() + userID * vec_dim;
        double *user_double_vecs = user.getVector(userID);
        for (int dim = 0; dim < vec_dim; dim++) {
            user_int_vecs[dim] = std::floor(user_double_vecs[dim]);
        }
    }

    unique_ptr<int[]> data_item_int_ptr = make_unique<int[]>(n_data_item * vec_dim);
    for (int itemID = 0; itemID < n_data_item; itemID++) {
        int *item_int_vecs = data_item_int_ptr.get() + itemID * vec_dim;
        double *item_double_vecs = data_item.getVector(itemID);
        for (int dim = 0; dim < vec_dim; dim++) {
            item_int_vecs[dim] = std::floor(item_double_vecs[dim]);
        }
    }

    record.reset();

    int avg_int= 0;
    for(int userID=0;userID < n_user;userID++){
        int *user_vecs = user_int_ptr.get() + userID * vec_dim;
        for(int itemID =0 ;itemID < n_data_item;itemID++){
            int* item_vecs = data_item_int_ptr.get() + itemID * vec_dim;
            int ip = InnerProduct(user_vecs, item_vecs, vec_dim);
            avg_int += ip;
        }
    }
    double int_time = record.get_elapsed_time_second();

    printf("time used double: %.3f int: %.3f\n", double_time, int_time);

    printf("%.3f %d\n", avg_double, avg_int);

    return 0;
}