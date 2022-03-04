#include <vector>
#include "alg/SVD.hpp"
#include "util/VectorIO.hpp"
#include "struct/VectorMatrix.hpp"
#include <string>
#include "alg/MonotonicityReduction.hpp"
#include <spdlog/spdlog.h>

using namespace ReverseMIPS;
using namespace std;

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

    double *query_ptr = query_item.getVector(0);
    printf("%.3f %.3f %.3f %.3f\n", query_ptr[0], query_ptr[1], query_ptr[2], query_ptr[3]);
    double *user_normal_ptr = user.getRawData();
    printf("%.3f %.3f %.3f %.3f\n", user_normal_ptr[0], user_normal_ptr[1], user_normal_ptr[2], user_normal_ptr[3]);

    VectorMatrix transfer_item;
    std::vector<double> eigen_l(vec_dim);
    const double SIGMA = 0.7;
    int check_dim = SVD::SVD(user, data_item, transfer_item, eigen_l, SIGMA);

    MonotonicityReduction::ConvertUserItem(user, data_item, eigen_l);

    for (int queryID = 0; queryID < n_query_item; queryID++) {
        SVD::TransferItem(query_item.getVector(queryID), transfer_item, vec_dim);
        std::vector<double> &query = MonotonicityReduction::ConvertQuery(query_item.getVector(queryID), eigen_l,
                                                                         vec_dim);

        for (int dim = 2; dim < query.size(); dim++) {
            assert(query[dim] > 0);
        }
    }

    for (int i = 0; i < n_data_item; i++) {
        double *data_item_ptr = data_item.getVector(i);
        for (int dim = 2; dim < vec_dim; dim++) {
            assert(data_item_ptr[dim] > 0);
        }
    }
    for (int i = 0; i < n_user; i++) {
        double *user_ptr = user.getVector(i);
        for (int dim = 2; dim < vec_dim; dim++) {
            assert(user_ptr[dim] > 0);
        }
    }
    printf("%d %d\n", user.vec_dim_, data_item.vec_dim_);

    return 0;
}