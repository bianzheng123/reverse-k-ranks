#include <algorithm>
#include <iostream>
#include <vector>
#include "alg/SVD.hpp"
#include "util/VectorIO.hpp"
#include "struct/VectorMatrix.hpp"
#include <string>
#include <spdlog/spdlog.h>

using namespace std;
using namespace ReverseMIPS;


int main(int argc, char **argv) {
//    if (!(argc == 2 or argc == 3)) {
//        cout << argv[0] << " dataset_name [basic_dir]" << endl;
//        return 0;
//    }
//    const char *dataset_name = argv[1];
    const char *dataset_name = "fake";
    const char *basic_dir = "/home/bianzheng/Dataset/ReverseMIPS";
    if (argc == 3) {
        basic_dir = argv[2];
    }
    printf("test dataset_name %s, basic_dir %s\n", dataset_name, basic_dir);

    double total_build_index_time;
    char index_path[256];
    sprintf(index_path, "../index/%s.index", dataset_name);

    int n_data_item, n_query_item, n_user, vec_dim;
    vector<VectorMatrix> data = readData(basic_dir, dataset_name, n_data_item, n_query_item, n_user,
                                         vec_dim);
    VectorMatrix &user = data[0];
    VectorMatrix &data_item = data[1];
    VectorMatrix &query_item = data[2];
    spdlog::info("n_data_item {}, n_query_item {}, n_user {}, vec_dim {}", n_data_item, n_query_item, n_user, vec_dim);

    int n_eval = 5;
    std::vector<double> before_user_data(n_eval * n_eval);
    std::vector<double> before_user_query(n_eval * n_eval);

    for (int i = 0; i < n_eval; i++) {
        for (int j = 0; j < n_eval; j++) {
            double *user_vecs = user.getVector(i);
            double *item_vecs = data_item.getVector(j);
            double ip = InnerProduct(user_vecs, item_vecs, vec_dim);
            before_user_data[i * n_eval + j] = ip;

            double *query_item_vecs = query_item.getVector(j);
            ip = InnerProduct(user_vecs, query_item_vecs, vec_dim);
            before_user_query[i * n_eval + j] = ip;
        }
    }

    double *query_ptr = query_item.getVector(0);
    printf("%.3f %.3f %.3f %.3f\n", query_ptr[0], query_ptr[1], query_ptr[2], query_ptr[3]);
    double *user_normal_ptr = user.getRawData();
    printf("%.3f %.3f %.3f %.3f\n", user_normal_ptr[0], user_normal_ptr[1], user_normal_ptr[2], user_normal_ptr[3]);

    VectorMatrix transfer_item;
    std::vector<double> eigen_l;
    const double SIGMA = 0.7;
    int check_dim = SVD::SVD(user, data_item, transfer_item, eigen_l, SIGMA);

    printf("after transfer item %.6f %.6f %.6f\n", transfer_item.getRawData()[0], transfer_item.getRawData()[1],
           transfer_item.getRawData()[2]);

    for (int queryID = 0; queryID < n_query_item; queryID++) {
        SVD::TransferItem(query_item.getVector(queryID), transfer_item, vec_dim);
    }

    query_ptr = query_item.getVector(0);
    double *user_vecs = user.getRawData();
    printf("%.3f %.3f %.3f %.3f\n", query_ptr[0], query_ptr[1], query_ptr[2], query_ptr[3]);
    printf("%.3f %.3f %.3f %.3f\n", user_vecs[0], user_vecs[1], user_vecs[2], user_vecs[3]);

    std::vector<double> after_user_data(n_eval * n_eval);
    std::vector<double> after_user_query(n_eval * n_eval);

    printf("%d %d\n", user.vec_dim_, data_item.vec_dim_);

    for (int i = 0; i < n_eval; i++) {
        for (int j = 0; j < n_eval; j++) {
            double *user_vecs = user.getVector(i);
            double *item_vecs = data_item.getVector(j);
            double ip = InnerProduct(user_vecs, item_vecs, vec_dim);
            after_user_data[i * n_eval + j] = ip;

            double *query_item_vecs = query_item.getVector(j);
            ip = InnerProduct(user_vecs, query_item_vecs, vec_dim);
            after_user_query[i * n_eval + j] = ip;
        }
    }

    for (int i = 0; i < n_eval * n_eval; i++) {
        printf("%d %.6f %.6f\n", i, before_user_data[i], after_user_data[i]);
        assert(abs(before_user_data[i] - after_user_data[i]) < 0.1);
    }

    printf("\n\n");
    for (int i = 0; i < n_eval * n_eval; i++) {
        printf("%d %.6f %.6f\n", i, before_user_query[i], after_user_query[i]);
        assert(abs(before_user_query[i] - after_user_query[i]) < 0.1);
    }

    return 0;
}