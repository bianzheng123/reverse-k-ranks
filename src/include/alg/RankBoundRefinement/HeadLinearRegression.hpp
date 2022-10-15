//
// Created by BianZheng on 2022/10/14.
//

#ifndef REVERSE_K_RANKS_HEADLINEARREGRESSION_HPP
#define REVERSE_K_RANKS_HEADLINEARREGRESSION_HPP

#include "struct/DistancePair.hpp"
#include <iostream>
#include <memory>
#include <Eigen/Dense>
#include <spdlog/spdlog.h>

namespace ReverseMIPS {

    class HeadLinearRegression {

        size_t n_data_item_, n_user_, n_parameter_;
        int n_sample_rank_;
        std::unique_ptr<int[]> sample_rank_l_; // n_sample_rank
        std::unique_ptr<double[]> predict_para_l_; //n_user_ * n_parameter_
        std::unique_ptr<int[]> error_l_; //n_user_

        //used for loading
        double *preprocess_cache_X_; // n_sample_rank * n_parameter, store queryIP in the  sampled rank
        double *preprocess_cache_Y_; // n_sample_rank, store the double type of sampled rank value
    public:

        inline HeadLinearRegression() {}

        inline HeadLinearRegression(const int &n_data_item, const int &n_user) {
            this->n_data_item_ = n_data_item;
            this->n_user_ = n_user;
            this->n_parameter_ = 3; // real linear regression
            this->predict_para_l_ = std::make_unique<double[]>(n_user * n_parameter_);
            this->error_l_ = std::make_unique<int[]>(n_user);
        }

        void StartPreprocess(const int *sample_rank_l, const int &n_sample_rank) {
            this->n_sample_rank_ = n_sample_rank;
            this->sample_rank_l_ = std::make_unique<int[]>(n_sample_rank);
            this->preprocess_cache_X_ = new double[n_sample_rank * n_parameter_];
            this->preprocess_cache_Y_ = new double[n_sample_rank];
            for (int sampleID = 0; sampleID < n_sample_rank; sampleID++) {
                preprocess_cache_Y_[sampleID] = sampleID;
                sample_rank_l_[sampleID] = sample_rank_l[sampleID];
            }

        }

        void LoopPreprocess(const double *sampleIP_l, const int &userID) {
#pragma omp parallel for default(none) shared(sampleIP_l)
            for (int sampleID = 0; sampleID < n_sample_rank_; sampleID++) {
                preprocess_cache_X_[sampleID * n_parameter_] = 1;
                for (int paraID = 1; paraID < n_parameter_; paraID++) {
                    preprocess_cache_X_[sampleID * n_parameter_ + paraID] =
                            sampleIP_l[sampleID] * preprocess_cache_X_[sampleID * n_parameter_ + paraID - 1];
                }
            }
            using RowMatrixXd = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
            Eigen::Map<RowMatrixXd> X(preprocess_cache_X_, n_sample_rank_, (int64_t) n_parameter_);

//    printf("%.3f %.3f %.3f %.3f\n", X_cache[0], X_cache[1], X_cache[2], X_cache[3]);
//    std::cout << X.row(1) << std::endl;
//    std::cout << X.col(1).size() << std::endl;
//    printf("X rows %ld, cols %ld\n", X.MapBase<Eigen::Map<Eigen::Matrix<double, -1, -1, 1>, 0>, 0>::rows(),
//           X.MapBase<Eigen::Map<Eigen::Matrix<double, -1, -1, 1>, 0>, 0>::cols());

            Eigen::Map<Eigen::VectorXd> Y(preprocess_cache_Y_, n_sample_rank_);
            Eigen::VectorXd res = (X.transpose() * X).ldlt().solve(X.transpose() * Y);
            assert(res.rows() == n_parameter_);
//            printf("res rows %ld, cols %ld\n", res.rows(), res.cols());
//            printf("res [0]: %.3f, [1]: %.3f\n", res[0], res[1]);

            //assign parameter
            for (int paraID = 0; paraID < n_parameter_; paraID++) {
                predict_para_l_[userID * n_parameter_ + paraID] = res[paraID];
            }

            //assign error
            int error = -1;
#pragma omp parallel for default(none) shared(userID, error)
            for (int sampleID = 0; sampleID < n_sample_rank_; sampleID++) {
                double pred_rank = 0;
                for (int paraID = 0; paraID < n_parameter_; paraID++) {
                    pred_rank += predict_para_l_[userID * n_parameter_ + paraID] *
                                 preprocess_cache_X_[sampleID * n_parameter_ + paraID];
                }
                const int real_rank = sampleID;
                const int tmp_error = std::abs(std::floor(pred_rank) - real_rank);
#pragma omp critical
                error = std::max(tmp_error, error);
            }

            error_l_[userID] = error;
            assert(-1 < error && error < n_sample_rank_);
        }

        void FinishPreprocess() {
            delete[] preprocess_cache_X_;
            delete[] preprocess_cache_Y_;
            preprocess_cache_X_ = nullptr;
            preprocess_cache_Y_ = nullptr;
        }

        inline void
        ComputeRankBound(const double &queryIP, const int &userID,
                         int &rank_lb, int &rank_ub, const int &queryID) const {

            const size_t pred_pos = userID * n_parameter_;
            double pred_rank = predict_para_l_[pred_pos];
            double accu_times = 1;
            for (int paraID = 1; paraID < n_parameter_; paraID++) {
                accu_times *= queryIP;
                pred_rank += predict_para_l_[pred_pos + paraID] * accu_times;
            }
            const int pred_int_rank = std::floor(pred_rank);
            const int pred_sample_rank_lb = pred_int_rank + error_l_[userID];
            const int pred_sample_rank_ub = pred_int_rank - error_l_[userID];

//            if (queryID == 3 && userID == 865) {
//                printf("queryID %d, userID %d, queryIP %.3f, pred_int_rank %d, error %d, pred_sample_rank_lb %d, pred_sample_rank_ub %d\n",
//                       queryID, userID, queryIP, pred_int_rank, error_l_[userID], pred_sample_rank_lb,
//                       pred_sample_rank_ub);
//            }

            if (pred_sample_rank_lb >= n_sample_rank_) {
                rank_lb = n_data_item_;
            } else if (pred_sample_rank_lb < 0) {
                rank_lb = sample_rank_l_[0];
            } else {
                rank_lb = sample_rank_l_[pred_sample_rank_lb];
            }

            if (pred_sample_rank_ub >= n_sample_rank_) {
                rank_ub = n_data_item_;
            } else if (pred_sample_rank_ub < 0) {
                rank_ub = 0;
            } else {
                rank_ub = sample_rank_l_[pred_sample_rank_ub];
            }
        }

//        void RankBound(const std::vector<double> &queryIP_l,
//                       const std::vector<bool> &prune_l, const std::vector<bool> &result_l,
//                       std::vector<int> &rank_lb_l, std::vector<int> &rank_ub_l) const {
//            assert(queryIP_l.size() == n_user_);
//            assert(prune_l.size() == n_user_);
//            assert(result_l.size() == n_user_);
//            assert(rank_lb_l.size() == n_user_);
//            assert(rank_ub_l.size() == n_user_);
//            for (int userID = 0; userID < n_user_; userID++) {
//                if (prune_l[userID] || result_l[userID]) {
//                    continue;
//                }
//                int lower_rank = rank_lb_l[userID];
//                int upper_rank = rank_ub_l[userID];
//                assert(upper_rank <= lower_rank);
//                double queryIP = queryIP_l[userID];
//
//                ComputeRankBound(queryIP, userID,
//                                 lower_rank, upper_rank);
//
//                rank_lb_l[userID] = lower_rank;
//                rank_ub_l[userID] = upper_rank;
//            }
//        }

        void RankBound(const std::vector<std::pair<double, double>> &queryIP_l,
                       std::vector<int> &rank_lb_l, std::vector<int> &rank_ub_l, const int &queryID) const {
            for (int userID = 0; userID < n_user_; userID++) {
                const double queryIP_lb = queryIP_l[userID].first;
                int qIP_lb_tmp_lower_rank, qIP_lb_tmp_upper_rank;

                ComputeRankBound(queryIP_lb, userID,
                                 qIP_lb_tmp_lower_rank, qIP_lb_tmp_upper_rank, queryID);

                const double queryIP_ub = queryIP_l[userID].second;
                int qIP_ub_tmp_lower_rank, qIP_ub_tmp_upper_rank;
                ComputeRankBound(queryIP_ub, userID,
                                 qIP_ub_tmp_lower_rank, qIP_ub_tmp_upper_rank, queryID);

                rank_lb_l[userID] = qIP_lb_tmp_lower_rank;
                rank_ub_l[userID] = qIP_ub_tmp_upper_rank;

//                if (queryID == 3 && userID == 865) {
//                    printf("queryID %d, userID %d, queryIP_lb %.3f, queryIP_ub %.3f, rank_lb %d, rank_ub %d\n",
//                           queryID, userID, queryIP_lb, queryIP_ub, rank_lb_l[userID], rank_ub_l[userID]);
//                }

                assert(qIP_lb_tmp_upper_rank <= qIP_lb_tmp_lower_rank);
                assert(qIP_ub_tmp_upper_rank <= qIP_ub_tmp_lower_rank);
                assert(qIP_ub_tmp_upper_rank <= qIP_lb_tmp_lower_rank);
            }
        }

        uint64_t IndexSizeByte() const {
            return n_user_ * (sizeof(double) + sizeof(double) + sizeof(int));
        }

    };
}
#endif //REVERSE_K_RANKS_HEADLINEARREGRESSION_HPP
