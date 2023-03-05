//
// Created by BianZheng on 2022/11/7.
//

#ifndef REVERSE_K_RANKS_NAMETRANSLATION_HPP
#define REVERSE_K_RANKS_NAMETRANSLATION_HPP

#include <string>
#include <spdlog/spdlog.h>

namespace ReverseMIPS {
    std::string IndexName(const std::string &method_name) {
        if (method_name == "QueryRankSampleSearchKthRank" || method_name == "QueryRankSampleMinMaxIntLR" ||
            method_name == "QueryRankSampleDirectIntLR" || method_name == "QueryRankSampleGlobalIntLR" ||
            method_name == "QueryRankSampleUniformIntLR") {
            return "QueryRankSampleSearchKthRank";
        } else if (method_name == "QueryRankSampleSearchUniformRank" ||
                   method_name == "QueryRankSampleSearchUniformRankMinMaxIntLR" ||
                   method_name == "QueryRankSampleSearchUniformRankUniformIntLR") {
            return "QueryRankSampleSearchUniformRank";
        } else if (method_name == "QueryRankSampleScoreDistribution") {
            return "QueryRankSampleScoreDistribution";
        } else if (method_name == "QueryRankSampleSearchAllRank") {
            return "QueryRankSampleSearchAllRank";
        } else if (method_name == "RankSample") {
            return "RankSample";
        } else if (method_name == "QueryRankSampleSearchBruteForce") {
            return "QueryRankSampleSearchBruteForce";
        } else {
            spdlog::error("not find method name, program exit");
            exit(-1);
        }
    }

    std::string SampleName(const std::string &method_name) {
        if (method_name == "QueryRankSampleMinMaxIntLR" ||
            method_name == "QueryRankSampleScoreDistribution" || method_name == "QueryRankSampleSearchKthRank" ||
            method_name == "QueryRankSampleDirectIntLR" || method_name == "QueryRankSampleGlobalIntLR" ||
            method_name == "QueryRankSampleUniformIntLR") {
            return "OptimalPart";
        } else if (method_name == "QueryRankSampleSearchAllRank") {
            return "OptimalAll";
        } else if (method_name == "QueryRankSampleSearchUniformRank" ||
                   method_name == "QueryRankSampleSearchUniformRankMinMaxIntLR" ||
                   method_name == "QueryRankSampleSearchUniformRankUniformIntLR") {
            return "OptimalUniform";
        } else if (method_name == "RankSample") {
            return "Uniform";
        } else if (method_name == "QueryRankSampleSearchBruteForce") {
            return "OptimalBruteForce";
        } else {
            spdlog::error("not find method name, program exit");
            exit(-1);
        }
    }
}
#endif //REVERSE_K_RANKS_NAMETRANSLATION_HPP
