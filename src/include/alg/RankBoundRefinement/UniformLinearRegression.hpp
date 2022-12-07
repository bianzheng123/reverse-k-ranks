//
// Created by BianZheng on 2022/12/3.
//

#ifndef REVERSE_K_RANKS_UNIFORMLINEARREGRESSION_HPP
#define REVERSE_K_RANKS_UNIFORMLINEARREGRESSION_HPP

#include "struct/DistancePair.hpp"
#include "util/MathUtil.hpp"
#include "sdlp/sdlp.hpp"
#include "alg/RankBoundRefinement/BaseLinearRegression.hpp"

#include <iostream>
#include <memory>
#include <Eigen/Dense>
#include <spdlog/spdlog.h>

namespace ReverseMIPS {

    class UniformLinearRegression : public BaseLinearRegression {

        std::string method_name_ = "QueryRankSampleUniformIntLR";
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

        inline UniformLinearRegression() {}

        inline UniformLinearRegression(const int &n_data_item, const int &n_user) {
            this->n_data_item_ = n_data_item;
            this->n_user_ = n_user;
            this->predict_para_l_ = std::make_unique<double[]>(n_user * n_predict_parameter_);
            this->distribution_para_l_ = std::make_unique<double[]>(n_user * n_distribution_parameter_);
            this->error_l_ = std::make_unique<int[]>(n_user);
            static_assert(n_predict_parameter_ == 2 && n_distribution_parameter_ == 2);
        }

        inline UniformLinearRegression(const int &n_data_item, const int &n_user, const std::string &method_name) {
            method_name_ = method_name;
            this->n_data_item_ = n_data_item;
            this->n_user_ = n_user;
            this->predict_para_l_ = std::make_unique<double[]>(n_user * n_predict_parameter_);
            this->distribution_para_l_ = std::make_unique<double[]>(n_user * n_distribution_parameter_);
            this->error_l_ = std::make_unique<int[]>(n_user);
            static_assert(n_predict_parameter_ == 2 && n_distribution_parameter_ == 2);
        }

        inline UniformLinearRegression(const char *index_basic_dir, const char *dataset_name,
                                       const size_t &n_sample, const size_t &n_sample_query,
                                       const size_t &sample_topk) {
            LoadIndex(index_basic_dir, dataset_name, n_sample, n_sample_query, sample_topk);
        }

        inline UniformLinearRegression(const char *index_basic_dir, const char *dataset_name,
                                       const size_t &n_sample, const size_t &n_sample_query, const size_t &sample_topk,
                                       const bool &is_uniform_rank) {
            LoadIndex(index_basic_dir, dataset_name, n_sample, n_sample_query, sample_topk, is_uniform_rank);
        }

        void StartPreprocess(const int *sample_rank_l, const int &n_sample_rank) override {
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


        void LoopPreprocess(const double *sampleIP_l, const int &userID) override {
            const int high_score_quantile_idx = (int) (n_sample_rank_ * 0.05);
            const int low_score_quantile_idx = (int) (n_sample_rank_ * 0.95);
            const double high_score = sampleIP_l[high_score_quantile_idx];
            const double low_score = sampleIP_l[low_score_quantile_idx];
            const double score_diff = high_score - low_score;
            distribution_para_l_[userID * n_distribution_parameter_] = low_score;
            distribution_para_l_[userID * n_distribution_parameter_ + 1] = score_diff;

#pragma omp parallel for default(none) shared(sampleIP_l, low_score, score_diff)
            for (int sampleID = 0; sampleID < n_sample_rank_; sampleID++) {
                const double input_x = (sampleIP_l[sampleID] - low_score) / score_diff;
                preprocess_cache_A_[(2 * sampleID) * (n_predict_parameter_ + 1)] = -input_x;
                preprocess_cache_A_[(2 * sampleID) * (n_predict_parameter_ + 1) + 1] = -1;
                preprocess_cache_A_[(2 * sampleID) * (n_predict_parameter_ + 1) + 2] = -1;

                preprocess_cache_A_[(2 * sampleID) * (n_predict_parameter_ + 1) + 3] = input_x;
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

            sdlp::linprog<3>(c, A, b, x);

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

        void FinishPreprocess() override {
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
            const double low_score = distribution_para_l_[distribution_pos];
            const double score_diff = distribution_para_l_[distribution_pos + 1];
            const double input_x = (queryIP - low_score) / score_diff;

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

                assert(qIP_lb_tmp_upper_rank <= qIP_lb_tmp_lower_rank);
                assert(qIP_ub_tmp_upper_rank <= qIP_ub_tmp_lower_rank);
                assert(qIP_ub_tmp_upper_rank <= qIP_lb_tmp_lower_rank);
            }
        }

        void SaveIndex(const char *index_basic_dir, const char *dataset_name, const size_t &n_sample_query,
                       const size_t &sample_topk) override {
            char index_path[256];
            if (method_name_ == "QueryRankSampleUniformIntLR") {
                sprintf(index_path,
                        "%s/memory_index/UniformLinearRegression-%s-n_sample_%d-n_sample_query_%ld-sample_topk_%ld.index",
                        index_basic_dir, dataset_name, n_sample_rank_, n_sample_query, sample_topk);
            } else {
                sprintf(index_path,
                        "%s/memory_index/UniformLinearRegression-QueryRankSampleSearchUniformRankUniformIntLR-%s-n_sample_%d-n_sample_query_%ld-sample_topk_%ld.index",
                        index_basic_dir, dataset_name, n_sample_rank_, n_sample_query, sample_topk);
            }


            std::ofstream out_stream_ = std::ofstream(index_path, std::ios::binary | std::ios::out);
            if (!out_stream_) {
                spdlog::error("error in write result, not found index");
                exit(-1);
            }
            out_stream_.write((char *) &n_data_item_, sizeof(size_t));
            out_stream_.write((char *) &n_user_, sizeof(size_t));
            out_stream_.write((char *) &n_sample_rank_, sizeof(int));

            out_stream_.write((char *) sample_rank_l_.get(), (int64_t) (n_sample_rank_ * sizeof(int)));
            out_stream_.write((char *) predict_para_l_.get(),
                              (int64_t) (n_user_ * n_predict_parameter_ * sizeof(double)));
            out_stream_.write((char *) distribution_para_l_.get(),
                              (int64_t) (n_user_ * n_distribution_parameter_ * sizeof(double)));
            out_stream_.write((char *) error_l_.get(), (int64_t) (n_user_ * sizeof(int)));

            out_stream_.close();
        }

        void LoadIndex(const char *index_basic_dir, const char *dataset_name,
                       const size_t &n_sample, const size_t &n_sample_query, const size_t &sample_topk,
                       const bool &is_uniform_rank = false) {
            char index_path[256];
            if (is_uniform_rank) {
                sprintf(index_path,
                        "%s/memory_index/UniformLinearRegression-QueryRankSampleSearchUniformRankUniformIntLR-%s-n_sample_%ld-n_sample_query_%ld-sample_topk_%ld.index",
                        index_basic_dir, dataset_name, n_sample, n_sample_query, sample_topk);
            } else {
                sprintf(index_path,
                        "%s/memory_index/UniformLinearRegression-%s-n_sample_%ld-n_sample_query_%ld-sample_topk_%ld.index",
                        index_basic_dir, dataset_name, n_sample, n_sample_query, sample_topk);
            }
            spdlog::info("index path {}", index_path);

            std::ifstream index_stream = std::ifstream(index_path, std::ios::binary | std::ios::in);
            if (!index_stream) {
                spdlog::error("error in reading index");
                exit(-1);
            }

            index_stream.read((char *) &n_data_item_, sizeof(size_t));
            index_stream.read((char *) &n_user_, sizeof(size_t));
            index_stream.read((char *) &n_sample_rank_, sizeof(int));

            sample_rank_l_ = std::make_unique<int[]>(n_sample_rank_);
            index_stream.read((char *) sample_rank_l_.get(), (int64_t) (sizeof(int) * n_sample_rank_));

            predict_para_l_ = std::make_unique<double[]>(n_user_ * n_predict_parameter_);
            index_stream.read((char *) predict_para_l_.get(),
                              (int64_t) (sizeof(double) * n_user_ * n_predict_parameter_));

            distribution_para_l_ = std::make_unique<double[]>(n_user_ * n_distribution_parameter_);
            index_stream.read((char *) distribution_para_l_.get(),
                              (int64_t) (sizeof(double) * n_user_ * n_distribution_parameter_));

            error_l_ = std::make_unique<int[]>(n_user_);
            index_stream.read((char *) error_l_.get(),
                              (int64_t) (sizeof(int) * n_user_));

            index_stream.close();
        }

        uint64_t IndexSizeByte() const {
            const uint64_t sample_rank_size = sizeof(int) * n_sample_rank_;
            const uint64_t para_size = sizeof(double) * n_user_ * (n_predict_parameter_ + n_distribution_parameter_);
            const uint64_t error_size = sizeof(int) * n_user_;
            return sample_rank_size + para_size + error_size;
        }

    };
}
#endif //REVERSE_K_RANKS_UNIFORMLINEARREGRESSION_HPP
