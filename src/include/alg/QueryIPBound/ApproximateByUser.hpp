//
// Created by BianZheng on 2022/10/8.
//

#ifndef REVERSE_K_RANKS_APPROXIMATEBYUSER_HPP
#define REVERSE_K_RANKS_APPROXIMATEBYUSER_HPP

#include <cassert>
#include <memory>
#include <vector>

#include "alg/QueryIPBound/BaseQueryIPBound.hpp"
#include "alg/SpaceInnerProduct.hpp"
#include "alg/SpaceEuclidean.hpp"
#include "struct/VectorMatrix.hpp"

namespace ReverseMIPS {

    class ApprUser {
        int n_user_, n_data_item_, vec_dim_, n_partition_;
        double itv_dist_; // distance of an interval, 1.0 / n_partition_
        std::unique_ptr<double[]> user_grid_;// vec_dim * (n_partition_ + 1), each cell stores times of residual and approximation of user
        std::unique_ptr<unsigned char[]> user_codeword_; //n_user_ * vec_dim_
        std::vector<bool> user_negativity_l_; //n_user_ * vec_dim_, true means positive, false means negative
        std::vector<bool> query_negativity_l_; //vec_dim_

        void EncodeUser(const VectorMatrix &user) {
            for (int userID = 0; userID < n_user_; userID++) {
                const double *user_vecs = user.getVector(userID);
                for (int dim = 0; dim < vec_dim_; dim++) {
                    user_negativity_l_[userID * vec_dim_ + dim] = user_vecs[dim] > 0;
                    const int code = std::floor(std::abs(user_vecs[dim] - 0) / itv_dist_);
                    assert(0 <= code && code < n_partition_);
                    user_codeword_[userID * vec_dim_ + dim] = code;
                }
            }

        }

    public:
        inline ApprUser() {
            this->n_user_ = -1;
            this->n_data_item_ = -1;
            this->vec_dim_ = -1;
            this->n_partition_ = -1;
            this->itv_dist_ = 0;
        };

        inline ApprUser(const int &n_user, const int &n_data_item, const int &vec_dim, const int &n_partition) {
            this->n_user_ = n_user;
            this->n_data_item_ = n_data_item;
            this->vec_dim_ = vec_dim;
            this->n_partition_ = n_partition;
            const int lshift = sizeof(unsigned char) * 8;
            if (n_partition > (1 << lshift)) {
                spdlog::error("n_partition too large, program exit");
                exit(-1);
            }
            this->itv_dist_ = 1.0 / n_partition_;

            user_grid_ = std::make_unique<double[]>(vec_dim_ * (n_partition_ + 1));
            user_codeword_ = std::make_unique<unsigned char[]>(n_user_ * vec_dim_);
            user_negativity_l_.resize(n_user_ * vec_dim_);
            query_negativity_l_.resize(vec_dim_);
        }

        void Preprocess(VectorMatrix &user, VectorMatrix &data_item) {
            for (int userID = 0; userID < n_user_; userID++) {
                const double *user_vecs = user.getVector(userID);
                for (int dim = 0; dim < vec_dim_; dim++) {
                    assert(-1 <= user_vecs[dim] && user_vecs[dim] <= 1);
                }
            }
            EncodeUser(user);
        }

        void
        IPBound(const double *residual_vecs, std::vector<std::pair<double, double>> &queryIP_l) {
            assert(queryIP_l.size() == n_user_);
            for (int dim = 0; dim < vec_dim_; dim++) {
                query_negativity_l_[dim] = residual_vecs[dim] > 0;
            }

            const int total_n_partition = n_partition_ + 1;
            for (int dim = 0; dim < vec_dim_; dim++) {
                const double query_itv_dist = itv_dist_ * std::abs(residual_vecs[dim]);
                double *tmp_user_grid = user_grid_.get() + dim * total_n_partition;
                tmp_user_grid[0] = 0;
                for (int partitionID = 1; partitionID < total_n_partition; partitionID++) {
                    tmp_user_grid[partitionID] = tmp_user_grid[partitionID - 1] + query_itv_dist;
                }
            }

            for (int userID = 0; userID < n_user_; userID++) {
                const int offset = userID * vec_dim_;
                const unsigned char *user_codeword_ptr = user_codeword_.get() + offset;

                double IP_lb = 0;
                double IP_ub = 0;
                for (int dim = 0; dim < vec_dim_; dim++) {
                    const unsigned char code_user = user_codeword_ptr[dim];
                    assert(0 <= code_user && code_user < n_partition_);
                    const bool sign_user = user_negativity_l_[offset + dim];
                    const bool sign_query = query_negativity_l_[dim];

                    const int64_t user_grid_idx = dim * total_n_partition + code_user;
                    const std::pair<double, double> bound_pair = make_pair(user_grid_[user_grid_idx],
                                                                           user_grid_[user_grid_idx + 1]);
                    if (sign_user ^ sign_query) { //multiply result is negative
                        IP_lb -= bound_pair.second;
                        IP_ub -= bound_pair.first;
                    } else {
                        IP_lb += bound_pair.first;
                        IP_ub += bound_pair.second;
                    }
                }
                queryIP_l[userID] = std::make_pair(IP_lb, IP_ub);

            }

        }

    };

    class ApproximateByUser : public BaseQueryIPBound {
        int n_user_, n_data_item_, vec_dim_;
        int n_sample_item_;
        std::unique_ptr<double[]> sample_ip_l_; //n_sample_item_ * n_user_, record the sampled ip
        std::unique_ptr<double[]> sample_item_vecs_; //n_sample_item * vec_dim_, record the sampled item vectors
        std::unique_ptr<double[]> residual_vecs_; //vec_dim_
        std::vector<std::pair<double, double>> residualIP_l_; //n_user_
        ApprUser appr_ins_;

    public:
        ApproximateByUser() {
            this->n_user_ = -1;
            this->n_data_item_ = -1;
            this->vec_dim_ = -1;
        };

        ApproximateByUser(const int &n_user, const int &n_data_item, const int &vec_dim,
                          const int &n_sample_item) {
            this->n_user_ = n_user;
            this->n_data_item_ = n_data_item;
            this->vec_dim_ = vec_dim;
            this->n_sample_item_ = n_sample_item;
            sample_ip_l_ = std::make_unique<double[]>(n_sample_item_ * n_user_);
            sample_item_vecs_ = std::make_unique<double[]>(n_sample_item_ * vec_dim_);
            residual_vecs_ = std::make_unique<double[]>(vec_dim_);
            residualIP_l_.resize(n_user_);
            appr_ins_ = ApprUser(n_user, n_data_item, vec_dim, 256);
        }

        void SelectItemCandidate(const VectorMatrix &data_item, std::vector<int> &itemID_l) {
            assert(itemID_l.size() == n_sample_item_);
            //random select the item
            itemID_l[0] = 0;
            std::vector<bool> is_cand_l(n_data_item_, false);
            is_cand_l[0] = true;
            //select the nect item
            for (int item_candID = 1; item_candID < n_sample_item_; item_candID++) {

                int max_dist_itemID = -1;
                double max_dist = 0;
#pragma omp parallel for default(none) shared(is_cand_l, max_dist, max_dist_itemID, data_item, item_candID, itemID_l)
                for (int itemID = 0; itemID < n_data_item_; itemID++) {
                    if (is_cand_l[itemID]) {
                        continue;
                    }
                    double cand_item_min_dist = DBL_MAX;
                    const double *tmp_item_vecs = data_item.getVector(itemID);
                    for (int prev_item_candID = 0; prev_item_candID < item_candID; prev_item_candID++) {
                        assert(itemID != itemID_l[prev_item_candID]);
                        const double *cand_item_vecs = data_item.getVector(itemID_l[prev_item_candID]);
                        const double cand_item_dist = EuclideanDistance(tmp_item_vecs, cand_item_vecs, vec_dim_);
                        cand_item_min_dist = cand_item_min_dist < cand_item_dist ? cand_item_min_dist : cand_item_dist;
                    }
#pragma omp critical
                    {
                        if (max_dist < cand_item_min_dist) {
                            max_dist = cand_item_min_dist;
                            max_dist_itemID = itemID;
                        }
                    };

                }
                assert(max_dist_itemID != -1);
                itemID_l[item_candID] = max_dist_itemID;
                is_cand_l[max_dist_itemID] = true;
            }

        }

        void
        AssignItemCandidate(const VectorMatrix &user, const VectorMatrix &data_item, const std::vector<int> &itemID_l) {
            assert(itemID_l.size() == n_sample_item_);
#pragma omp parallel for default(none) shared(itemID_l, data_item, user)
            for (int sampleID = 0; sampleID < n_sample_item_; sampleID++) {
                const int itemID = itemID_l[sampleID];
                const double *item_vecs = data_item.getVector(itemID);
                memcpy(sample_item_vecs_.get() + sampleID * vec_dim_, item_vecs, vec_dim_ * sizeof(double));

                std::vector<double> itemIP_l(n_user_);
                for (int userID = 0; userID < n_user_; userID++) {
                    const double *user_vecs = user.getVector(userID);
                    const double itemIP = InnerProduct(item_vecs, user_vecs, vec_dim_);
                    itemIP_l[userID] = itemIP;
                }
                memcpy(sample_ip_l_.get() + sampleID * n_user_, itemIP_l.data(), n_user_ * sizeof(double));
            }
        }

        void Preprocess(VectorMatrix &user, VectorMatrix &data_item) override {

            //select item with a large distance
            //first random select an item, then select the item with the maximum of minimum of all the other candidates, until all the elements has selected
            std::vector<int> itemID_l(n_sample_item_);
            SelectItemCandidate(data_item, itemID_l);
            //record their ip and vecs information
            AssignItemCandidate(user, data_item, itemID_l);

            appr_ins_.Preprocess(user, data_item);

        }

        void
        IPBound(const double *query_vecs, const VectorMatrix &user,
                std::vector<std::pair<double, double>> &ip_bound_l) {
            assert(ip_bound_l.size() == n_user_);

            double min_appr_dist = DBL_MAX;
            int min_sampleID = -1;
            for (int sampleID = 0; sampleID < n_sample_item_; sampleID++) {
                const double *item_vecs = sample_item_vecs_.get() + sampleID * vec_dim_;
                const double appr_dist = EuclideanDistance(item_vecs, query_vecs, vec_dim_);
                if (min_appr_dist > appr_dist) {
                    min_appr_dist = appr_dist;
                    min_sampleID = sampleID;
                }
            }
            assert(min_sampleID != -1 && min_appr_dist >= 0);

            const double *appr_item_vecs = sample_item_vecs_.get() + min_sampleID * vec_dim_;
            for (int dim = 0; dim < vec_dim_; dim++) {
                residual_vecs_[dim] = query_vecs[dim] - appr_item_vecs[dim];
            }
            //calc bound of grid index
            const double *sample_ip_l = sample_ip_l_.get() + min_sampleID * n_user_;

            appr_ins_.IPBound(residual_vecs_.get(), residualIP_l_);

            for (int userID = 0; userID < n_user_; userID++) {
                const double appr_queryIP = sample_ip_l[userID];
                const std::pair<double, double> residual_ip = residualIP_l_[userID];
                const double queryIP_lb = appr_queryIP + residual_ip.first;
                const double queryIP_ub = appr_queryIP + residual_ip.second;

                ip_bound_l[userID] = make_pair(queryIP_lb, queryIP_ub);

                // for test only
//                const double queryIP = InnerProduct(query_vecs, user.getVector(userID), vec_dim_);
//                assert(queryIP_lb <= queryIP && queryIP <= queryIP_ub);

            }

        }

    };

}
#endif //REVERSE_K_RANKS_APPROXIMATEBYUSER_HPP
