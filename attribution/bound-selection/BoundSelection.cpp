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

namespace ReverseMIPS {
    const int integer_scale = 100;

    class Config {
    public:
        double time_used_, lb_tightness_, ub_tightness_;
        std::string bound_name_;

        inline Config(const double &time_used, const std::pair<double, double> tightness_pair,
                      const std::string &bound_name) {
            this->time_used_ = time_used;
            this->lb_tightness_ = tightness_pair.first;
            this->ub_tightness_ = tightness_pair.second;
            this->bound_name_ = bound_name;
        }

    };

    std::pair<double, double>
    Tightness(std::vector<double> &exactIP_l, std::vector<double> &lb_IP_l, std::vector<double> &ub_IP_l) {
        const int arr_size = (int) exactIP_l.size();
        assert(arr_size == lb_IP_l.size());
        assert(arr_size == ub_IP_l.size());

        double lb_tightness = 0;
        for (int i = 0; i < arr_size; i++) {
            assert(lb_IP_l[i] <= exactIP_l[i]);
            double tmp_tightness = (exactIP_l[i] - lb_IP_l[i]) / std::abs(exactIP_l[i]);
            assert(0 <= tmp_tightness);
            lb_tightness += tmp_tightness;
        }
        lb_tightness /= arr_size;

        double ub_tightness = 0;
        for (int i = 0; i < arr_size; i++) {
            assert(exactIP_l[i] <= ub_IP_l[i]);
            double tmp_tightness = (ub_IP_l[i] - exactIP_l[i]) / std::abs(exactIP_l[i]);
            assert(0 <= tmp_tightness);
            ub_tightness += tmp_tightness;
        }
        ub_tightness /= arr_size;
        return std::make_pair(lb_tightness, ub_tightness);
    }

    void AttributionWrite(std::vector<Config> config_l, const char *dataset_name, const char *method_name) {

        char resPath[256];
        std::sprintf(resPath, "../../result/attribution/BoundSelection/config-%s-%s.txt", dataset_name, method_name);
        std::ofstream file(resPath);
        if (!file) {
            std::printf("error in write result\n");
        }

        int config_size = (int) config_l.size();
        for (int i = 0; i < config_size; i++) {
            Config config = config_l[i];
            file << "method name " << config.bound_name_ << ", time used " << config.time_used_
                 << "s, lower bound tightness " << config.lb_tightness_
                 << ", upper bound tightness " << config.ub_tightness_ << std::endl;
        }

        file.close();
    }

    double FullNorm(const VectorMatrix &user, const VectorMatrix &query,
                    std::vector<double> &lb_l, std::vector<double> &ub_l, const int &check_dim) {
        int n_user = user.n_vector_;
        int vec_dim = user.vec_dim_;
        int n_query = query.n_vector_;
        assert(vec_dim == query.vec_dim_);

        //preprocessing, calculate the norm of query and user for all the dimension
        std::vector<double> user_norm_l(n_user);
        for (int userID = 0; userID < n_user; userID++) {
            double IP = InnerProduct(user.getVector(userID), user.getVector(userID), vec_dim);
            IP = std::sqrt(IP);
            user_norm_l[userID] = IP;
        }

        lb_l.resize(n_query * n_user);
        ub_l.resize(n_query * n_user);

        TimeRecord record;
        record.reset();

        for (int qID = 0; qID < n_query; qID++) {
            double *query_vecs = query.getVector(qID);
            double query_norm = InnerProduct(query_vecs, query_vecs, vec_dim);
            query_norm = std::sqrt(query_norm);

            double *lb_vecs = lb_l.data() + qID * n_user;
            double *ub_vecs = ub_l.data() + qID * n_user;
            for (int userID = 0; userID < n_user; userID++) {
                double times_norm = query_norm * user_norm_l[userID];
                double tmp_lb = -times_norm;
                lb_vecs[userID] = tmp_lb;

                double tmp_ub = times_norm;
                ub_vecs[userID] = tmp_ub;
            }
        }

        double time_used = record.get_elapsed_time_second();

        return time_used;
    }

    double PartIntPartNorm(const VectorMatrix &user, const VectorMatrix &query,
                           std::vector<double> &lb_l, std::vector<double> &ub_l,
                           const int &check_dim) {

        const int n_user = user.n_vector_;
        const int vec_dim = user.vec_dim_;
        const int n_query = query.n_vector_;
        const int remain_dim = vec_dim - check_dim;

        std::unique_ptr<int[]> user_int_unique_ptr = std::make_unique<int[]>(n_user * check_dim);
        std::unique_ptr<int[]> user_int_sum_ptr = std::make_unique<int[]>(n_user);
        double user_max_part_dim = user.getVector(0)[0];

        std::unique_ptr<double[]> user_part_norm_ptr = std::make_unique<double[]>(n_user);

        //compute the integer bound for the first part
        for (int userID = 0; userID < n_user; userID++) {
            double *user_vecs = user.getVector(userID);
            for (int dim = 0; dim < check_dim; dim++) {
                user_max_part_dim = std::max(user_max_part_dim, user_vecs[dim]);
            }
        }
        for (int userID = 0; userID < n_user; userID++) {
            double norm2 = InnerProduct(user.getVector(userID, check_dim),
                                        user.getVector(userID, check_dim),
                                        remain_dim);
            user_part_norm_ptr[userID] = std::sqrt(norm2);

            double *user_vecs = user.getVector(userID);
            int *user_int_vecs = user_int_unique_ptr.get() + userID * check_dim;

            user_int_sum_ptr[userID] = 0;
            for (int dim = 0; dim < check_dim; dim++) {
                user_int_vecs[dim] = std::floor(user_vecs[dim] * integer_scale / user_max_part_dim);
                user_int_sum_ptr[userID] += std::abs(user_int_vecs[dim]) + 1;
            }
        }
        double user_convert_coe = user_max_part_dim / (integer_scale * integer_scale);

        lb_l.resize(n_query * n_user);
        ub_l.resize(n_query * n_user);

        TimeRecord record;
        record.reset();

        std::unique_ptr<int[]> query_int_ptr = std::make_unique<int[]>(check_dim);
        for (int qID = 0; qID < n_query; qID++) {

            double *query_vecs = query.getVector(qID);
            double query_max_part_dim = query_vecs[0];

            for (int dim = 1; dim < check_dim; dim++) {
                query_max_part_dim = std::max(query_max_part_dim, query_vecs[dim]);
            }

            double qratio = integer_scale / query_max_part_dim;
            int query_int_sum = 0;
            for (int dim = 0; dim < check_dim; dim++) {
                query_int_ptr[dim] = std::floor(query_vecs[dim] * qratio);
                query_int_sum += std::abs(query_int_ptr[dim]);
            }
            double convert_coe = user_convert_coe * query_max_part_dim;

            double *query_norm_vecs = query_vecs + check_dim;
            double part_query_norm = InnerProduct(query_norm_vecs, query_norm_vecs, remain_dim);
            part_query_norm = std::sqrt(part_query_norm);

            for (int userID = 0; userID < n_user; userID++) {
                int *user_int_vecs = user_int_unique_ptr.get() + userID * check_dim;
                int *query_int_vecs = query_int_ptr.get();
                int intIP = InnerProduct(user_int_vecs, query_int_vecs, check_dim);
                int int_otherIP = user_int_sum_ptr[userID] + query_int_sum;
                int lb_int_part = intIP - int_otherIP;
                int ub_int_part = intIP + int_otherIP;
                double lb_part = lb_int_part * convert_coe;
                double ub_part = ub_int_part * convert_coe;

                double norm_times_res = part_query_norm * user_part_norm_ptr[userID];
                double lb = -norm_times_res + lb_part;
                double ub = norm_times_res + ub_part;
                int arr_offset = qID * n_user + userID;
                lb_l[arr_offset] = lb;
                ub_l[arr_offset] = ub;
            }
        }
        double time_used = record.get_elapsed_time_second();

        return time_used;
    }

    double FullInt(const VectorMatrix &user, const VectorMatrix &query,
                   std::vector<double> &lb_l, std::vector<double> &ub_l,
                   const int &check_dim) {

        const int n_user = user.n_vector_;
        const int vec_dim = user.vec_dim_;
        const int n_query = query.n_vector_;
        const int remain_dim = vec_dim - check_dim;

        std::unique_ptr<int[]> user_int_ptr = std::make_unique<int[]>(n_user * vec_dim);
        std::unique_ptr<std::pair<int, int>[]> user_int_sum_ptr = std::make_unique<std::pair<int, int>[]>(n_user);
        std::pair<double, double> user_max_dim;
        user_max_dim.first = user.getVector(0)[0];
        user_max_dim.second = user.getVector(0)[check_dim];

        //compute the integer bound for the first part
        for (int userID = 0; userID < n_user; userID++) {
            double *user_vecs = user.getVector(userID);
            for (int dim = 0; dim < check_dim; dim++) {
                user_max_dim.first = std::max(user_max_dim.first, user_vecs[dim]);
            }
            for (int dim = check_dim; dim < vec_dim; dim++) {
                user_max_dim.second = std::max(user_max_dim.second, user_vecs[dim]);
            }
        }

        for (int userID = 0; userID < n_user; userID++) {
            int *user_int_vecs = user_int_ptr.get() + userID * vec_dim;
            double *user_double_vecs = user.getVector(userID);
            user_int_sum_ptr[userID].first = 0;
            for (int dim = 0; dim < check_dim; dim++) {
                user_int_vecs[dim] = std::floor(user_double_vecs[dim] * integer_scale / user_max_dim.first);
                user_int_sum_ptr[userID].first += std::abs(user_int_vecs[dim]) + 1;
            }

            user_int_sum_ptr[userID].second = 0;
            for (int dim = check_dim; dim < vec_dim; dim++) {
                user_int_vecs[dim] = std::floor(user_double_vecs[dim] * integer_scale / user_max_dim.second);
                user_int_sum_ptr[userID].second += std::abs(user_int_vecs[dim]) + 1;
            }

        }

        std::pair<double, double> convert_coe;
        convert_coe.first = user_max_dim.first / (integer_scale * integer_scale);
        convert_coe.second = user_max_dim.second / (integer_scale * integer_scale);

        lb_l.resize(n_query * n_user);
        ub_l.resize(n_query * n_user);

        TimeRecord record;
        record.reset();

        std::unique_ptr<int[]> query_int_ptr = std::make_unique<int[]>(vec_dim);
        for (int qID = 0; qID < n_query; qID++) {
            double *query_vecs = query.getVector(qID);
            std::pair<double, double> query_max_dim(query_vecs[0], query_vecs[check_dim]);
            for (int dim = 1; dim < check_dim; dim++) {
                query_max_dim.first = std::max(query_max_dim.first, query_vecs[dim]);
            }

            for (int dim = check_dim + 1; dim < vec_dim; dim++) {
                query_max_dim.second = std::max(query_max_dim.second, query_vecs[dim]);
            }
            double left_convert_coe = convert_coe.first * query_max_dim.first;
            double right_convert_coe = convert_coe.second * query_max_dim.second;

            std::pair<double, double> qratio(integer_scale / query_max_dim.first, integer_scale / query_max_dim.second);

            std::pair<int, int> query_int_sum(0, 0);
            for (int dim = 0; dim < check_dim; dim++) {
                query_int_ptr[dim] = std::floor(query_vecs[dim] * qratio.first);
                query_int_sum.first += std::abs(query_int_ptr[dim]);
            }

            for (int dim = check_dim; dim < vec_dim; dim++) {
                query_int_ptr[dim] = std::floor(query_vecs[dim] * qratio.second);
                query_int_sum.second += std::abs(query_int_ptr[dim]);
            }

            int *query_int_vecs = query_int_ptr.get();
            int *query_remain_int_vecs = query_int_vecs + check_dim;
            for (int userID = 0; userID < n_user; userID++) {
                int *user_int_vecs = user_int_ptr.get() + userID * vec_dim;
                int leftIP = InnerProduct(user_int_vecs, query_int_vecs, check_dim);
                int left_otherIP = user_int_sum_ptr[userID].first + query_int_sum.first;
                int lb_left_part = leftIP - left_otherIP;
                int ub_left_part = leftIP + left_otherIP;

                int *user_remain_int_vecs = user_int_vecs + check_dim;
                int rightIP = InnerProduct(user_remain_int_vecs, query_remain_int_vecs, remain_dim);
                int right_otherIP = user_int_sum_ptr[userID].second + query_int_sum.second;
                int lb_right_part = rightIP - right_otherIP;
                int ub_right_part = rightIP + right_otherIP;

                double lb_part = left_convert_coe * lb_left_part + right_convert_coe * lb_right_part;
                double ub_part = left_convert_coe * ub_left_part + right_convert_coe * ub_right_part;

                int arr_offset = qID * n_user + userID;
                lb_l[arr_offset] = lb_part;
                ub_l[arr_offset] = ub_part;

            }
        }
        double time_used = record.get_elapsed_time_second();

        return time_used;
    }


    double PartDimPartNorm(const VectorMatrix &user, const VectorMatrix &query,
                           std::vector<double> &lb_l, std::vector<double> &ub_l,
                           const int &check_dim) {

        const int n_user = user.n_vector_;
        const int vec_dim = user.vec_dim_;
        const int n_query = query.n_vector_;
        const int remain_dim = vec_dim - check_dim;

        std::unique_ptr<double[]> user_norm_l = std::make_unique<double[]>(n_user);
        for (int userID = 0; userID < n_user; userID++) {
            double *user_part_vecs = user.getVector(userID) + check_dim;
            double IP = InnerProduct(user_part_vecs, user_part_vecs, remain_dim);
            user_norm_l[userID] = std::sqrt(IP);
        }

        lb_l.resize(n_query * n_user);
        ub_l.resize(n_query * n_user);

        TimeRecord record;
        record.reset();
        for (int qID = 0; qID < n_query; qID++) {

            double *query_vecs = query.getVector(qID);
            double *query_part_vecs = query_vecs + check_dim;
            double query_norm = InnerProduct(query_part_vecs, query_part_vecs, remain_dim);
            query_norm = std::sqrt(query_norm);

            for (int userID = 0; userID < n_user; userID++) {
                double *user_vecs = user.getVector(userID);
                double partIP = InnerProduct(user_vecs, query_vecs, check_dim);

                double times_norm = query_norm * user_norm_l[userID];
                int arr_offset = qID * n_user + userID;
                lb_l[arr_offset] = partIP - times_norm;
                ub_l[arr_offset] = partIP + times_norm;
            }
        }
        double time_used = record.get_elapsed_time_second();

        return time_used;
    }

    double PartDimPartInt(const VectorMatrix &user, const VectorMatrix &query,
                          std::vector<double> &lb_l, std::vector<double> &ub_l,
                          const int &check_dim) {

        const int n_user = user.n_vector_;
        const int vec_dim = user.vec_dim_;
        const int n_query = query.n_vector_;
        const int remain_dim = vec_dim - check_dim;

        std::unique_ptr<int[]> user_int_ptr = std::make_unique<int[]>(n_user * remain_dim);
        double user_max_dim = user.getVector(0)[check_dim];
        std::unique_ptr<int[]> user_int_sum_ptr = std::make_unique<int[]>(n_user);

        for (int userID = 0; userID < n_user; userID++) {
            double *user_vecs = user.getVector(userID);
            for (int dim = check_dim; dim < vec_dim; dim++) {
                user_max_dim = std::max(user_max_dim, user_vecs[dim]);
            }
        }

        for (int userID = 0; userID < n_user; userID++) {
            int *user_int_vecs = user_int_ptr.get() + userID * remain_dim;
            double *tmp_user_vecs = user.getVector(userID) + check_dim;
            user_int_sum_ptr[userID] = 0;
            for (int dim = 0; dim < remain_dim; dim++) {
                user_int_vecs[dim] = std::floor(tmp_user_vecs[dim] * integer_scale / user_max_dim);
                user_int_sum_ptr[userID] += std::abs(user_int_vecs[dim]) + 1;
            }
        }
        double user_convert_coe = user_max_dim / (integer_scale * integer_scale);

        lb_l.resize(n_query * n_user);
        ub_l.resize(n_query * n_user);

        TimeRecord record;
        record.reset();

        std::unique_ptr<int[]> query_int_ptr = std::make_unique<int[]>(remain_dim);
        for (int qID = 0; qID < n_query; qID++) {
            double *query_vecs = query.getVector(qID);
            double query_max_dim = query_vecs[check_dim];
            for (int dim = check_dim + 1; dim < vec_dim; dim++) {
                query_max_dim = std::max(query_max_dim, query_vecs[dim]);
            }
            double qratio = integer_scale / query_max_dim;
            int query_int_sum = 0;
            double *tmp_query_vecs = query_vecs + check_dim;
            for (int dim = 0; dim < remain_dim; dim++) {
                query_int_ptr[dim] = std::floor(tmp_query_vecs[dim] * qratio);
                query_int_sum += std::abs(query_int_ptr[dim]);
            }

            for (int userID = 0; userID < n_user; userID++) {
                double *user_vecs = user.getVector(userID);
                double partIP = InnerProduct(user_vecs, query_vecs, check_dim);

                int *user_int_vecs = user_int_ptr.get() + userID * remain_dim;
                int intIP = InnerProduct(user_int_vecs, query_int_ptr.get(), remain_dim);
                int int_otherIP = user_int_sum_ptr[userID] + query_int_sum;
                int lb_int_part = intIP - int_otherIP;
                int ub_int_part = intIP + int_otherIP;
                double convert_coe = user_convert_coe * query_max_dim;
                double lb_part = lb_int_part * convert_coe;
                double ub_part = ub_int_part * convert_coe;

                lb_l[qID * n_user + userID] = partIP + lb_part;
                ub_l[qID * n_user + userID] = partIP + ub_part;

            }
        }

        double time_used = record.get_elapsed_time_second();

        return time_used;
    }

    double FullDim(const VectorMatrix &user, const VectorMatrix &query,
                   std::vector<double> &lb_l, std::vector<double> &ub_l,
                   const int &check_dim) {
        const int n_user = user.n_vector_;
        const int vec_dim = user.vec_dim_;
        const int n_query = query.n_vector_;

        lb_l.resize(n_query * n_user);
        ub_l.resize(n_query * n_user);

        TimeRecord record;
        record.reset();

        for (int qID = 0; qID < n_query; qID++) {
            double *query_vecs = query.getVector(qID);

            for (int userID = 0; userID < n_user; userID++) {
                double *user_vecs = user.getVector(userID);
                double IP = InnerProduct(user_vecs, query_vecs, vec_dim);

                int arr_offset = qID * n_user + userID;
                lb_l[arr_offset] = IP;
                ub_l[arr_offset] = IP;
            }
        }

        double time_used = record.get_elapsed_time_second();

        return time_used;
    }

}

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
    printf("BoundSelection dataset_name %s, basic_dir %s\n", dataset_name, basic_dir);

    int n_data_item, n_query_item, n_user, vec_dim;
    vector<VectorMatrix> data = readData(basic_dir, dataset_name, n_data_item, n_query_item, n_user,
                                         vec_dim);
    VectorMatrix &user = data[0];
    VectorMatrix &data_item = data[1];
    VectorMatrix &query_item = data[2];
    spdlog::info("n_data_item {}, n_query_item {}, n_user {}, vec_dim {}", n_data_item, n_query_item, n_user,
                 vec_dim);

    VectorMatrix transfer_item;
    const double SIGMA = 0.7;
    int check_dim = SVD::SVD(user, data_item, transfer_item, SIGMA);

    for (int queryID = 0; queryID < n_query_item; queryID++) {
        SVD::TransferItem(query_item.getVector(queryID), transfer_item, vec_dim);
    }

    //calc exact bound
    std::vector<double> exactIP_l(n_query_item * n_user);
    for (int queryID = 0; queryID < n_query_item; queryID++) {
        for (int userID = 0; userID < n_user; userID++) {
            double IP = InnerProduct(user.getVector(userID), query_item.getVector(queryID), vec_dim);
            exactIP_l[queryID * n_user + userID] = IP;
        }
    }

    std::vector<Config> config_l;
    std::vector<double> lb_l;
    std::vector<double> ub_l;
    double time_used = FullNorm(user, query_item, lb_l, ub_l, check_dim);
    std::pair<double, double> tightness_pair = Tightness(exactIP_l, lb_l, ub_l);
    printf("finish evaluate %s\n", "FullNorm");
    config_l.emplace_back(time_used, tightness_pair, "FullNorm");

    time_used = PartIntPartNorm(user, query_item, lb_l, ub_l, check_dim);
    tightness_pair = Tightness(exactIP_l, lb_l, ub_l);
    printf("finish evaluate %s\n", "PartIntPartNorm");
    config_l.emplace_back(time_used, tightness_pair, "PartIntPartNorm");

    time_used = FullInt(user, query_item, lb_l, ub_l, check_dim);
    tightness_pair = Tightness(exactIP_l, lb_l, ub_l);
    printf("finish evaluate %s\n", "FullInt");
    config_l.emplace_back(time_used, tightness_pair, "FullInt");

    time_used = PartDimPartNorm(user, query_item, lb_l, ub_l, check_dim);
    tightness_pair = Tightness(exactIP_l, lb_l, ub_l);
    printf("finish evaluate %s\n", "PartDimPartNorm");
    config_l.emplace_back(time_used, tightness_pair, "PartDimPartNorm");

    time_used = PartDimPartInt(user, query_item, lb_l, ub_l, check_dim);
    tightness_pair = Tightness(exactIP_l, lb_l, ub_l);
    printf("finish evaluate %s\n", "PartDimPartInt");
    config_l.emplace_back(time_used, tightness_pair, "PartDimPartInt");

    time_used = FullDim(user, query_item, lb_l, ub_l, check_dim);
    tightness_pair = Tightness(exactIP_l, lb_l, ub_l);
    printf("finish evaluate %s\n", "FullDim");
    config_l.emplace_back(time_used, tightness_pair, "FullDim");


    AttributionWrite(config_l, dataset_name, "FullDimension");
    return 0;
}