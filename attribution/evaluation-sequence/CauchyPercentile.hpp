//
// Created by BianZheng on 2022/2/28.
//

#ifndef REVERSE_K_RANKS_GROUNDTRUTH_HPP
#define REVERSE_K_RANKS_GROUNDTRUTH_HPP

#include "alg/SpaceInnerProduct.hpp"
#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "struct/VectorMatrix.hpp"
#include "struct/MethodBase.hpp"
#include <fstream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <map>
#include <spdlog/spdlog.h>
#include <numeric>

namespace ReverseMIPS::CauchyPercentile {

    class Bound {
    public:
        double lower_bound_, upper_bound_;

        inline Bound() {
            lower_bound_ = -1;
            upper_bound_ = -1;
        }

        inline Bound(double lower_bound, double upper_bound) {
            this->lower_bound_ = lower_bound;
            this->upper_bound_ = upper_bound;
        }
    };

    int BinarySearch(double queryIP, std::vector<double> &distance_cache) {
        auto iter_begin = distance_cache.begin();
        auto iter_end = distance_cache.end();

        auto lb_ptr = std::lower_bound(iter_begin, iter_end, queryIP,
                                       [](const double &arrIP, double queryIP) {
                                           return arrIP > queryIP;
                                       });
        return (int) (lb_ptr - iter_begin) + 1;
    }

    std::vector<std::vector<UserRankElement>>
    GetGroundTruth(VectorMatrix &user, VectorMatrix &data_item, VectorMatrix &query_item, std::vector<Bound> &bound_l) {
        user.vectorNormalize();
        TimeRecord record;
        const int report_every_user_ = 100;
        const int n_user = user.n_vector_;
        const int n_data_item = data_item.n_vector_;
        const int n_query = query_item.n_vector_;
        const int vec_dim = data_item.vec_dim_;

        std::vector<std::vector<UserRankElement>> result(n_query, std::vector<UserRankElement>(n_data_item));

        record.reset();
#pragma omp parallel for default(none) shared(data_item, user, n_user, bound_l, n_query, query_item, result, n_data_item, vec_dim, std::cout, record)
        for (int userID = 0; userID < n_user; userID++) {
            std::vector<double> data_ip_l(n_data_item);
            for (int itemID = 0; itemID < n_data_item; itemID++) {
                double ip = InnerProduct(data_item.getVector(itemID), user.getVector(userID), vec_dim);
                data_ip_l[itemID] = ip;
            }
            std::sort(data_ip_l.begin(), data_ip_l.end(), std::greater<double>());
            bound_l[userID].lower_bound_ = data_ip_l[n_data_item - 1];
            bound_l[userID].upper_bound_ = data_ip_l[0];

            for (int queryID = 0; queryID < n_query; queryID++) {
                double queryIP = InnerProduct(query_item.getVector(queryID), user.getVector(userID), vec_dim);
                int rank = BinarySearch(queryIP, data_ip_l);
                result[queryID][userID] = UserRankElement(userID, rank, queryIP);
            }

            if (userID % report_every_user_ == 0) {
                std::cout << "get ground truth " << userID / (0.01 * n_user) << " %, "
                          << record.get_elapsed_time_second() << " s/iter" << " Mem: "
                          << get_current_RSS() / 1000000 << " Mb \n";
                record.reset();
            }

        }
        for (int queryID = 0; queryID < n_query; queryID++) {
            std::sort(result[queryID].begin(), result[queryID].end(), std::less<UserRankElement>());
        }
        printf("can sort\n");
        return result;
    }

    std::vector<int> Argsort(std::vector<double> arr){
        int n = arr.size();
        std::vector<int> arg_l(n);
        const auto function = [arr](int a, int b) noexcept -> bool{
            return arr[a] > arr[b];
        };
        std::iota(arg_l.begin(), arg_l.end(), 0);
        std::sort(arg_l.begin(), arg_l.end(), function);
        return arg_l;
    }

    std::vector<int> EvaluationSequence(std::vector<Bound> &bound_l) {
        int n_user = bound_l.size();
        std::vector<double> value_l(n_user);
        for (int userID = 0; userID < n_user; userID++) {
            value_l[userID] = (1.0f - bound_l[userID].lower_bound_) /
                                 (bound_l[userID].upper_bound_ - bound_l[userID].lower_bound_);
        }

        std::vector<int> eval_seq_l = Argsort(value_l);
        return eval_seq_l;
    }

}

#endif //REVERSE_K_RANKS_GROUNDTRUTH_HPP
