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
        virtual std::vector<std::vector<UserRankElement>>
        Retrieval(const VectorMatrix &query_item, const int &topk) = 0;

        virtual std::string
        PerformanceStatistics(const int &topk, const double &retrieval_time, const double &second_per_query) = 0;

        virtual std::string BuildIndexStatistics() {
            return "Build Index Info:";
        };

        virtual std::string VariancePerformanceMetricName() {
            return "queryID, retrieval time, second per query";
        }

        virtual std::string VariancePerformanceStatistics(
                const double &retrieval_time, const double &second_per_query, const int &queryID) {
            char str[256];
            sprintf(str, "%d,%.3f,%.3f", queryID, retrieval_time, second_per_query);
            return str;
        };

        virtual ~BaseIndex() = default;

    };

}


#endif //REVERSE_KRANKS_METHODBASE_HPP
