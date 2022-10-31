//
// Created by BianZheng on 2022/10/29.
//

#ifndef REVERSE_K_RANKS_MINMAXHEADLINEARREGRESSION_HPP
#define REVERSE_K_RANKS_MINMAXHEADLINEARREGRESSION_HPP

#include "struct/DistancePair.hpp"
#include "util/MathUtil.hpp"
#include "sdlp/sdlp.hpp"

#include <iostream>
#include <memory>
#include <Eigen/Dense>
#include <spdlog/spdlog.h>

namespace ReverseMIPS {

    class MinMaxHeadLinearRegression {

        size_t n_data_item_, n_user_;
        static constexpr int n_predict_parameter_ = 2; // (a, b) for linear estimation
        static constexpr int n_distribution_parameter_ = 2; // mu, sigma
        static constexpr double sqrt_2_ = sqrt(2.0);
        int n_sample_rank_;
        std::unique_ptr<int[]> sample_rank_l_; // n_sample_rank
        std::unique_ptr<double[]> predict_para_l_; // n_user_ * n_predict_parameter
        std::unique_ptr<double[]> distribution_para_l_; // n_user_ * n_distribution_parameter
        std::unique_ptr<int[]> error_l_; //n_user_

        //used for loading
        double *preprocess_cache_A_; // (2 * n_sample_rank) * (n_predict_parameter_ + 1), constraint matrix
        double *preprocess_cache_b_; // (2 * n_sample_rank), constraint bound
        double *preprocess_cache_c_; // 3, objective coefficients
    public:

        inline MinMaxHeadLinearRegression() {}

        inline MinMaxHeadLinearRegression(const int &n_data_item, const int &n_user) {
            this->n_data_item_ = n_data_item;
            this->n_user_ = n_user;
            this->predict_para_l_ = std::make_unique<double[]>(n_user * n_predict_parameter_);
            this->distribution_para_l_ = std::make_unique<double[]>(n_user * n_distribution_parameter_);
            this->error_l_ = std::make_unique<int[]>(n_user);
            static_assert(n_predict_parameter_ == 2 && n_distribution_parameter_ == 2);
        }

        void StartPreprocess(const int *sample_rank_l, const int &n_sample_rank) {
            this->n_sample_rank_ = n_sample_rank;
            this->sample_rank_l_ = std::make_unique<int[]>(n_sample_rank);
            this->preprocess_cache_A_ = new double[(2 * n_sample_rank) * (n_predict_parameter_ + 1)];
            this->preprocess_cache_b_ = new double[2 * n_sample_rank];
            this->preprocess_cache_c_ = new double[3];
            for (int sampleID = 0; sampleID < n_sample_rank; sampleID++) {
                sample_rank_l_[sampleID] = sample_rank_l[sampleID];

                preprocess_cache_b_[2 * sampleID] = -sampleID;
                preprocess_cache_b_[2 * sampleID + 1] = sampleID;
            }

            preprocess_cache_c_[0] = 0;
            preprocess_cache_c_[1] = 0;
            preprocess_cache_c_[2] = 1;

        }

        double ComputeAverage(const double *sampleIP_l) {
            double average = 0;
            for (int sampleID = 0; sampleID < n_sample_rank_; sampleID++) {
                average += sampleIP_l[sampleID];
            }
            return average / n_sample_rank_;
        }

        double ComputeStd(const double *sampleIP_l, const double average) {
            double sigma = 0;
            for (int sampleID = 0; sampleID < n_sample_rank_; sampleID++) {
                const double minus = sampleIP_l[sampleID] - average;
                const double term = minus * minus;
                sigma += term;
            }
            sigma /= n_sample_rank_;
            return std::sqrt(sigma);
        }

        double CDFPhi(double x) const {
            // constants
            double a1 = 0.254829592;
            double a2 = -0.284496736;
            double a3 = 1.421413741;
            double a4 = -1.453152027;
            double a5 = 1.061405429;
            double p = 0.3275911;

            // Save the sign of x
            int sign = 1;
            if (x < 0)
                sign = -1;
            x = fabs(x) / sqrt_2_;

            // A&S formula 7.1.26
            double t = 1.0 / (1.0 + p * x);
            double y = 1.0 - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * exp(-x * x);

            return 0.5 * (1.0 + sign * y);
        }

        void LoopPreprocess(const double *sampleIP_l, const int &userID) {
            //compute average, std
            const double mu = ComputeAverage(sampleIP_l);
            const double sigma = ComputeStd(sampleIP_l, mu);
            distribution_para_l_[userID * n_distribution_parameter_] = mu;
            distribution_para_l_[userID * n_distribution_parameter_ + 1] = sigma;

#pragma omp parallel for default(none) shared(sampleIP_l, mu, sigma)
            for (int sampleID = 0; sampleID < n_sample_rank_; sampleID++) {
                const double normal_num = (sampleIP_l[sampleID] - mu) / sigma;
                const double cdf = CDFPhi(normal_num);
                preprocess_cache_A_[(2 * sampleID) * (n_predict_parameter_ + 1)] = -cdf;
                preprocess_cache_A_[(2 * sampleID) * (n_predict_parameter_ + 1) + 1] = -1;
                preprocess_cache_A_[(2 * sampleID) * (n_predict_parameter_ + 1) + 2] = -1;

                preprocess_cache_A_[(2 * sampleID) * (n_predict_parameter_ + 1) + 3] = cdf;
                preprocess_cache_A_[(2 * sampleID) * (n_predict_parameter_ + 1) + 4] = 1;
                preprocess_cache_A_[(2 * sampleID) * (n_predict_parameter_ + 1) + 5] = -1;

            }
            using RowMatrixXd = Eigen::Matrix<double, Eigen::Dynamic, n_predict_parameter_ + 1, Eigen::RowMajor>;
            Eigen::Map<RowMatrixXd> A(preprocess_cache_A_, 2 * n_sample_rank_, (int64_t) n_predict_parameter_ + 1);

//    printf("%.3f %.3f %.3f %.3f\n", X_cache[0], X_cache[1], X_cache[2], X_cache[3]);
//    std::cout << X.row(1) << std::endl;
//    std::cout << X.col(1).size() << std::endl;
//    printf("X rows %ld, cols %ld\n", X.MapBase<Eigen::Map<Eigen::Matrix<double, -1, -1, 1>, 0>, 0>::rows(),
//           X.MapBase<Eigen::Map<Eigen::Matrix<double, -1, -1, 1>, 0>, 0>::cols());

            Eigen::Map<Eigen::VectorXd> b(preprocess_cache_b_, 2 * n_sample_rank_);

            Eigen::Map<Eigen::Matrix<double, n_predict_parameter_ + 1, 1>> c(preprocess_cache_c_,
                                                                             n_predict_parameter_ + 1);
            Eigen::Matrix<double, n_predict_parameter_ + 1, 1> x;

            double min_obj = sdlp::linprog<3>(c, A, b, x);

            assert(x.rows() == n_predict_parameter_ + 1);
//            printf("res rows %ld, cols %ld\n", res.rows(), res.cols());
//            printf("res [0]: %.3f, [1]: %.3f\n", res[0], res[1]);

            //assign parameter
            for (int paraID = 0; paraID < n_predict_parameter_; paraID++) {
                predict_para_l_[userID * n_predict_parameter_ + paraID] = x[paraID];
            }

            //assign error
            int error = -1;
#pragma omp parallel for default(none) shared(userID, error)
            for (int sampleID = 0; sampleID < n_sample_rank_; sampleID++) {
                const double tmp_a = predict_para_l_[userID * n_predict_parameter_];
                const double tmp_b = predict_para_l_[userID * n_predict_parameter_ + 1];
                const double tmp_x = preprocess_cache_A_[(2 * sampleID) * (n_predict_parameter_ + 1) + 3];
                double pred_rank = tmp_x * tmp_a + tmp_b;
                const int real_rank = sampleID;
                const int tmp_error = std::abs(std::floor(pred_rank) - real_rank);
#pragma omp critical
                error = std::max(tmp_error, error);
            }

            error_l_[userID] = error;
            assert(-1 < error && error < n_sample_rank_);
        }

        void FinishPreprocess() {
            delete[] preprocess_cache_A_;
            delete[] preprocess_cache_b_;
            delete[] preprocess_cache_c_;
            preprocess_cache_A_ = nullptr;
            preprocess_cache_b_ = nullptr;
            preprocess_cache_c_ = nullptr;
        }

        inline void
        ComputeRankBound(const double &queryIP, const int &userID,
                         int &rank_lb, int &rank_ub, const int &queryID) const {

            const size_t distribution_pos = userID * n_distribution_parameter_;
            const double mu = distribution_para_l_[distribution_pos];
            const double sigma = distribution_para_l_[distribution_pos + 1];
            const double normalize_x = (queryIP - mu) / sigma;
            const double input_x = CDFPhi(normalize_x);

            const size_t pred_pos = userID * n_predict_parameter_;
            const double pred_rank = input_x * predict_para_l_[pred_pos] + predict_para_l_[pred_pos + 1];
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
            const uint64_t sample_rank_size = sizeof(int) * n_sample_rank_;
            const uint64_t para_size = sizeof(double) * n_user_ * (n_predict_parameter_ + n_distribution_parameter_);
            const uint64_t error_size = sizeof(int) * n_user_;
            return sample_rank_size + para_size + error_size;
        }

    };
}
#endif //REVERSE_K_RANKS_MINMAXHEADLINEARREGRESSION_HPP
