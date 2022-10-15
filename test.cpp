#include "struct/VectorMatrix.hpp"
#include <vector>
#include <iostream>
#include <Eigen/Dense>
#include <numeric>
#include "util/VectorIO.hpp"

using namespace ReverseMIPS;
using namespace std;

//int Preprocess(VectorMatrix &user, VectorMatrix &data_item, const double &SIGMA) {
//    const int vec_dim = user.vec_dim_; // p->colNum, m
//    const int n_user = user.n_vector_; // p->rowNum, n
//    const int n_data_item = data_item.n_vector_;
//    data_item_cache_.resize(vec_dim);
//
//    std::unique_ptr<double[]> transfer_ptr = std::make_unique<double[]>(vec_dim * vec_dim);
//    transfer_item_.init(transfer_ptr, vec_dim, vec_dim);
//    assert(transfer_item_.n_vector_ == transfer_item_.vec_dim_);
//
//    //Q is item, since a new query would be added
//    //U is user, since user matrix would not changed
//    arma::mat P_t(user.getRawData(), user.vec_dim_, user.n_vector_, false, true);
//
//    arma::mat U_t;
//    arma::vec s;
//    arma::mat V;
//
//    // see: http://arma.sourceforge.net/docs.html#svd_econ
//    //	svd_econ(U_t, s, V, P_t, "both", "std");
//    arma::svd_econ(U_t, s, V, P_t, "both", "std"); // P = U * sigma * V_t
//
//    U_t = U_t.t();
//
//    double *uData = transfer_item_.getRawData();
//
//    for (int rowIndex = 0; rowIndex < vec_dim; rowIndex++) {
//        for (int colIndex = 0; colIndex < vec_dim; colIndex++) {
//            uData[rowIndex * vec_dim + colIndex] = s[rowIndex] * U_t(rowIndex, colIndex);
//        }
//    }
//
//    for (int rowIndex = 0; rowIndex < n_user; rowIndex++) {
//        for (int colIndex = 0; colIndex < vec_dim; colIndex++) {
//            user.getRawData()[rowIndex * vec_dim + colIndex] = V(rowIndex, colIndex);
//        }
//    }
//
//    for (int itemID = 0; itemID < n_data_item; itemID++) {
//        TransferItem(data_item.getVector(itemID), vec_dim);
//    }
//
//    std::vector<double> sum(vec_dim);
//    sum[0] = s[0];
//    for (int colIndex = 1; colIndex < vec_dim; colIndex++) {
//        sum[colIndex] = sum[colIndex - 1] + s[colIndex];
//    }
//
//    int check_dim = 0;
//    for (int colIndex = 0; colIndex < vec_dim; colIndex++) {
//        if (sum[colIndex] / sum[vec_dim - 1] >= SIGMA) {
//            check_dim = colIndex;
//            break;
//        }
//    }
//    return check_dim;
//}


void LoopPreprocess(const int *sample_rank_l, const double *sampleIP_l,
                    const int &n_sample_rank, const int &userID) {
    const int n_parameter = 2;
    double *X_cache = new double[n_sample_rank * n_parameter];
    for (int sampleID = 0; sampleID < n_sample_rank; sampleID++) {
        X_cache[sampleID * n_parameter] = 1;
        for (int paraID = 1; paraID < n_parameter; paraID++) {
            X_cache[sampleID * n_parameter + paraID] = sampleIP_l[sampleID];
        }
    }
    Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> X(X_cache, n_sample_rank,
                                                                                         n_parameter);

//    printf("%.3f %.3f %.3f %.3f\n", X_cache[0], X_cache[1], X_cache[2], X_cache[3]);
//    std::cout << X.row(1) << std::endl;
//    std::cout << X.col(1).size() << std::endl;
//    printf("X rows %ld, cols %ld\n", X.MapBase<Eigen::Map<Eigen::Matrix<double, -1, -1, 1>, 0>, 0>::rows(),
//           X.MapBase<Eigen::Map<Eigen::Matrix<double, -1, -1, 1>, 0>, 0>::cols());

    double *Y_cache = new double[n_sample_rank];
    for (int i = 0; i < n_sample_rank; i++) {
        Y_cache[i] = sample_rank_l[i];
    }
    Eigen::Map<Eigen::VectorXd> Y(Y_cache, n_sample_rank);

    Eigen::VectorXd res = (X.transpose() * X).ldlt().solve(X.transpose() * Y);
    printf("res rows %ld, cols %ld\n", res.rows(), res.cols());
    printf("res [0]: %.3f, [1]: %.3f\n", res[0], res[1]);

//    Eigen::MatrixXf A = Eigen::MatrixXf(n_sample_rank, n_parameter, X_cache);
//    Eigen::VectorXf b = Eigen::VectorXf::Random(3);
//    cout << "The solution using normal equations is:\n"
//         << (A.transpose() * A).ldlt().solve(A.transpose() * b) << endl;

//    para_vec.print("para_vec: ");

//    for (int crankID = 0; crankID < n_sample_; crankID++) {
//        unsigned int rankID = known_rank_idx_l_[crankID];
//        bound_distance_table_[n_sample_ * userID + crankID] = distance_ptr[rankID];
//    }
}

int main() {
    Eigen::MatrixXf test = Eigen::MatrixXf::Random(3, 2);
    Eigen::VectorXf test2 = Eigen::VectorXf::Random(3);
    std::cout << "test: " << test << std::endl;
    std::cout << "test2: " << test2 << std::endl;
    std::cout << "solution: " << test.transpose() * test2 << std::endl;


    Eigen::MatrixXf A = Eigen::MatrixXf::Random(3, 2);
    Eigen::VectorXf b = Eigen::VectorXf::Random(3);
    cout << "The solution using normal equations is:\n"
         << (A.transpose() * A).ldlt().solve(A.transpose() * b) << endl;

    const char *dataset_dir = "/home/bianzheng/Dataset/ReverseMIPS";
    const char *dataset_name = "fake-normal";

    int n_data_item, n_query_item, n_user, vec_dim;
    vector<VectorMatrix> data = readData(dataset_dir, dataset_name, n_data_item, n_query_item, n_user,
                                         vec_dim);
    VectorMatrix &user = data[0];
    VectorMatrix &data_item = data[1];
    VectorMatrix &query_item = data[2];
    spdlog::info("n_data_item {}, n_query_item {}, n_user {}, vec_dim {}", n_data_item, n_query_item, n_user, vec_dim);

    std::unique_ptr<int[]> sample_rank_l = make_unique<int[]>(n_data_item);
    std::iota(sample_rank_l.get(), sample_rank_l.get() + n_data_item, 0);
    const double *sampleIP_l = data_item.getVector(0);
    LoopPreprocess(sample_rank_l.get(), sampleIP_l, n_data_item, 0);

}