//
// Created by BianZheng on 2022/2/22.
//

#ifndef REVERSE_KRANKS_METHODBASE_HPP
#define REVERSE_KRANKS_METHODBASE_HPP

#include "struct/UserRankElement.hpp"
#include "struct/VectorMatrix.hpp"
#include <vector>

namespace ReverseMIPS {
    class BaseIndex {
    public:
        virtual std::vector<std::vector<UserRankElement>> Retrieval(VectorMatrix &query_item, const int &topk) {
            std::vector<std::vector<UserRankElement>> ins;
            return ins;
        }

        virtual std::string
        PerformanceStatistics(const int &topk, const double &retrieval_time, const double &second_per_query) {
            return "";
        }

        virtual ~BaseIndex() = default;

    };
}


#endif //REVERSE_KRANKS_METHODBASE_HPP
