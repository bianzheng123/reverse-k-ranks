#include <vector>
#include "alg/SVD.hpp"
#include "util/VectorIO.hpp"
#include "struct/VectorMatrix.hpp"
#include <string>
#include "alg/MonotonicityReduction.hpp"
#include "struct/UserRankElement.hpp"
#include <spdlog/spdlog.h>

using namespace ReverseMIPS;
using namespace std;

const double SIGMA = 0.7;

int TransformUserItem(VectorMatrix &user, VectorMatrix &data_item, VectorMatrix &transfer_item,
                      std::vector<double> &eigen_l) {

    //SVD
    int check_dim = SVD::SVD(user, data_item, transfer_item, eigen_l, SIGMA);

    //monotonicity  reduction
    MonotonicityReduction::ConvertUserItem(user, data_item, eigen_l);

    //user normalization
    user.vectorNormalize();

    return check_dim;
}

int getRank(double queryIP, double *user_vec, int vec_dim, VectorMatrix &data_item) {

    int n_data_item = data_item.n_vector_;
    int rank = 1;

    for (int i = 0; i < n_data_item; i++) {
        double data_dist = InnerProduct(data_item.getVector(i), user_vec, vec_dim);
        rank += data_dist > queryIP ? 1 : 0;
    }

    return rank;
}

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

    int n_data_item, n_query_item, n_user, old_vec_dim;
    vector<VectorMatrix> data = readData(basic_dir, dataset_name, n_data_item, n_query_item, n_user,
                                         old_vec_dim);
    VectorMatrix &user = data[0];
    VectorMatrix &data_item = data[1];
    VectorMatrix &query_item = data[2];
    spdlog::info("n_data_item {}, n_query_item {}, n_user {}, old_vec_dim {}", n_data_item, n_query_item, n_user,
                 old_vec_dim);

    printf("before transform\n");
    printf("user %.3f %.3f %.3f %.3f\n", user.getVector(0)[0], user.getVector(0)[1], user.getVector(0)[2],
           user.getVector(0)[3]);
    printf("data_item %.3f %.3f %.3f %.3f\n", data_item.getVector(0)[0], data_item.getVector(0)[1],
           data_item.getVector(0)[2],
           data_item.getVector(0)[3]);

    VectorMatrix transfer_item;
    std::vector<double> eigen_l;
    int check_dim = TransformUserItem(user, data_item, transfer_item, eigen_l);
    printf("check dim %d\n", check_dim);

    printf("after transform\n");
    printf("user %.3f %.3f %.3f %.3f\n", user.getVector(0)[0], user.getVector(0)[1], user.getVector(0)[2],
           user.getVector(0)[3]);
    printf("data_item %.3f %.3f %.3f %.3f\n", data_item.getVector(0)[0], data_item.getVector(0)[1],
           data_item.getVector(0)[2],
           data_item.getVector(0)[3]);

    int topk = 10;

    for (int queryID = 0; queryID < 1; queryID++) {
        double *item_vecs = query_item.getVector(queryID);
        SVD::TransferItem(item_vecs, transfer_item, old_vec_dim);
        std::vector<double> &query = MonotonicityReduction::ConvertQuery(item_vecs, old_vec_dim);
        for (int dim = 2; dim < query.size(); dim++) {
//            assert(query[dim] > 0);
        }

//        std::vector<double> query(old_vec_dim);
//        double *tmp_query_vecs = query_item.getVector(queryID);
//        for (int i = 0; i < query.size(); i++) {
//            query[i] = tmp_query_vecs[i];
//        }

        int vec_dim = (int) query.size();

        double *query_item_vec = query.data();
        std::vector<UserRankElement> minHeap(topk);

        for (int userID = 0; userID < topk; userID++) {
            double queryIP = InnerProduct(query_item_vec, user.getVector(userID), vec_dim);
            int tmp_rank = getRank(queryIP, user.getVector(userID), vec_dim, data_item);
            UserRankElement element(userID, tmp_rank, queryIP);
            minHeap[userID] = element;
        }

        std::make_heap(minHeap.begin(), minHeap.end(), std::less<UserRankElement>());
        UserRankElement minHeapEle = minHeap.front();
        for (int userID = topk; userID < n_user; userID++) {
            double queryIP = InnerProduct(query_item_vec, user.getVector(userID), vec_dim);
            int tmp_rank = getRank(queryIP, user.getVector(userID), vec_dim, data_item);
            UserRankElement element(userID, tmp_rank, queryIP);

            if (userID == 546) {
                printf("%d %d\n", userID, tmp_rank);
            }

            if (minHeapEle > element) {
                std::pop_heap(minHeap.begin(), minHeap.end(), std::less<UserRankElement>());
                minHeap.pop_back();
                minHeap.push_back(element);
                std::push_heap(minHeap.begin(), minHeap.end(), std::less<UserRankElement>());
                minHeapEle = minHeap.front();
            }
        }
        std::make_heap(minHeap.begin(), minHeap.end(), std::less<UserRankElement>());
        std::sort_heap(minHeap.begin(), minHeap.end(), std::less<UserRankElement>());

        printf("index\n");
        for (int i = 0; i < topk; i++) {
            printf("%d ", minHeap[i].userID_);
        }
        printf("\n");

        printf("rank\n");
        for (int i = 0; i < topk; i++) {
            printf("%d ", minHeap[i].rank_);
        }
        printf("\n");

    }

    for (int i = 0; i < n_data_item; i++) {
        double *data_item_ptr = data_item.getVector(i);
        for (int dim = 2; dim < data_item.vec_dim_; dim++) {
            assert(data_item_ptr[dim] > 0);
        }
    }
    for (int i = 0; i < n_user; i++) {
        double *user_ptr = user.getVector(i);
        for (int dim = 2; dim < user.vec_dim_; dim++) {
            assert(user_ptr[dim] > 0);
        }
    }

    return 0;
}