//
// Created by BianZheng on 2022/7/15.
//

#ifndef REVERSE_KRANKS_BATBUILDINDEXTOPT_HPP
#define REVERSE_KRANKS_BATBUILDINDEXTOPT_HPP

#include "alg/DiskIndex/TopTID.hpp"
#include "alg/DiskIndex/TopTIP.hpp"
#include "alg/RankBoundRefinement/ScoreSearch.hpp"
#include "score_computation/ComputeScoreTable.hpp"
#include "util/TimeMemory.hpp"
#include "util/VectorIO.hpp"

namespace ReverseMIPS {
    void TopTIDParameter(const int &n_data_item, const int &n_user, const uint64_t &index_size_gb,
                         int &topt) {
        //disk index
        const uint64_t index_size_byte = (uint64_t) index_size_gb * 1024 * 1024 * 1024;
        const uint64_t predict_index_size_byte = (uint64_t) sizeof(int) * n_data_item * n_user;
        const uint64_t topt_big_size = index_size_byte / sizeof(int) / n_user;
        topt = int(topt_big_size);
        spdlog::info("TopTID index size byte: {}, predict index size byte: {}", index_size_byte,
                     predict_index_size_byte);
        if (index_size_byte >= predict_index_size_byte) {
            spdlog::info("index size larger than the whole score table, use whole table setting");
            topt = n_data_item;
        }
    }

    void TopTIPParameter(const int &n_data_item, const int &n_user, const uint64_t &index_size_gb,
                         int &topt) {
        //disk index
        const uint64_t index_size_byte = (uint64_t) index_size_gb * 1024 * 1024 * 1024;
        const uint64_t predict_index_size_byte = (uint64_t) sizeof(double) * n_data_item * n_user;
        const uint64_t topt_big_size = index_size_byte / sizeof(double) / n_user;
        topt = int(topt_big_size);
        spdlog::info("TopTIP index size byte: {}, predict index size byte: {}", index_size_byte,
                     predict_index_size_byte);
        if (index_size_byte >= predict_index_size_byte) {
            spdlog::info("index size larger than the whole score table, use whole table setting");
            topt = n_data_item;
        }
    }

    /*
     * bruteforce index
     * shape: n_user * n_data_item, type: double, the distance pair for each user
     */

    void BuildIndex(const char *basic_dir, const char *dataset_name) {
        int n_data_item, n_query_item, n_user, vec_dim;
        std::vector<VectorMatrix> data = readData(basic_dir, dataset_name, n_data_item, n_query_item, n_user,
                                                  vec_dim);
        VectorMatrix &user = data[0];
        VectorMatrix &data_item = data[1];
        VectorMatrix &query_item = data[2];
        spdlog::info("n_data_item {}, n_query_item {}, n_user {}, vec_dim {}", n_data_item, n_query_item, n_user,
                     vec_dim);

        user.vectorNormalize();

        spdlog::info("input parameter: n_sample: {128, 1024}, index_size_gb {256}, method_name TopTID, TopTIP");

        const int index_size_gb_256 = 256;
        char toptID256_path[256];
        sprintf(toptID256_path, "../index/%s_TopTID256.index", dataset_name);
        char toptIP256_path[256];
        sprintf(toptIP256_path, "../index/%s_TopTIP256.index", dataset_name);

        user.vectorNormalize();

        int toptID256;
        TopTIDParameter(n_data_item, n_user, index_size_gb_256, toptID256);
        TopTID toptID256_ins(n_user, n_data_item, vec_dim, toptID256_path, toptID256);
        toptID256_ins.BuildIndexPreprocess();
        //do not have PreprocessData since only applicable for BaseIPBound

        int toptIP256;
        TopTIPParameter(n_data_item, n_user, index_size_gb_256, toptIP256);
        TopTIP toptIP256_ins(n_user, n_data_item, vec_dim, toptIP256_path, toptIP256);
        toptIP256_ins.BuildIndexPreprocess();

        if (toptID256_ins.exact_rank_ins_.method_name != "BaseIPBound" ||
            toptIP256_ins.exact_rank_ins_.method_name != "BaseIPBound") {
            spdlog::error("TopTID and TopTIP, its exact_rank_ins_ should all be BaseIPBound");
            exit(-1);
        }

        const int n_interval_128 = 128;
        const int n_interval_1024 = 1024;
        char ss128_path[256];
        sprintf(ss128_path, "../index/%s_ScoreSearch128.index", dataset_name);
        char ss1024_path[256];
        sprintf(ss1024_path, "../index/%s_ScoreSearch1024.index", dataset_name);

        //rank search
        ScoreSearch ss_128(n_interval_128, n_user, n_data_item);
        ScoreSearch ss_1024(n_interval_1024, n_user, n_data_item);

        //Compute Score Table
        ComputeScoreTable cst(user, data_item);
        std::vector<DistancePair> distance_pair_l(n_data_item);

        TimeRecord record;
        record.reset();
        TimeRecord component_record;

        double score_search_time = 0;
        double toptID_time = 0;
        double toptIP_time = 0;

        for (int userID = 0; userID < n_user; userID++) {
            cst.ComputeSortItems(userID, distance_pair_l.data());

            component_record.reset();
            ss_128.LoopPreprocess(distance_pair_l.data(), userID);
            ss_1024.LoopPreprocess(distance_pair_l.data(), userID);
            score_search_time += component_record.get_elapsed_time_second();

            component_record.reset();
            toptID256_ins.BuildIndexLoop(distance_pair_l.data());
            toptID_time += component_record.get_elapsed_time_second();

            component_record.reset();
            toptIP256_ins.BuildIndexLoop(distance_pair_l.data());
            toptIP_time += component_record.get_elapsed_time_second();

            if (userID != 0 && userID % cst.report_every_ == 0) {
                std::cout << "preprocessed " << userID / (0.01 * n_user) << " %, "
                          << record.get_elapsed_time_second() << " s/iter" << " Mem: "
                          << get_current_RSS() / 1000000 << " Mb \n";
                spdlog::info(
                        "Compute Score Time {}s, Sort Score Time {}s, Score Search Time {}s, TopTID Time {}s, TopTIP Time {}s",
                        cst.compute_time_, cst.sort_time_, score_search_time, toptID_time, toptIP_time);
                cst.compute_time_ = 0;
                cst.sort_time_ = 0;
                score_search_time = 0;
                toptID_time = 0;
                toptIP_time = 0;
                record.reset();
            }
        }
        cst.FinishCompute();
        toptID256_ins.FinishBuildIndex();
        toptIP256_ins.FinishBuildIndex();

        ss_128.SaveIndex(ss128_path);
        ss_1024.SaveIndex(ss1024_path);
    }
}
#endif //REVERSE_KRANKS_BATBUILDINDEXTOPT_HPP
