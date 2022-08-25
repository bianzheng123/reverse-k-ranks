//
// Created by BianZheng on 2022/7/29.
//

#include "RankSamplePrintPruneRatio.hpp"
#include "QueryRankSamplePrintPruneRatio.hpp"
#include "score_computation/ComputeScoreTable.hpp"

#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <string>

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

class Parameter {
public:
    std::string basic_dir, dataset_name;
    int n_sample_query, sample_topk;
};

void LoadOptions(int argc, char **argv, Parameter &para) {
    namespace po = boost::program_options;

    po::options_description opts("Allowed options");
    opts.add_options()
            ("help,h", "help info")
            ("basic_dir,bd",
             po::value<std::string>(&para.basic_dir)->default_value("/home/bianzheng/Dataset/ReverseMIPS"),
             "basic directory")
            ("dataset_name, ds", po::value<std::string>(&para.dataset_name)->default_value("fake-normal"),
             "dataset_name")

            ("n_sample_query, nsq", po::value<int>(&para.n_sample_query)->default_value(150),
             "the numer of sample query in training query distribution")
            ("sample_topk, st", po::value<int>(&para.sample_topk)->default_value(50),
             "topk in training query distribution");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, opts), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << opts << std::endl;
        exit(0);
    }
}

using namespace std;
using namespace ReverseMIPS;

int main(int argc, char **argv) {
    Parameter para;
    LoadOptions(argc, argv, para);
    const char *dataset_name = para.dataset_name.c_str();
    const char *basic_dir = para.basic_dir.c_str();
    const int n_sample_query = para.n_sample_query;
    const int sample_topk = para.sample_topk;
    spdlog::info("PrintPruneRatio dataset_name {}, basic_dir {}", dataset_name, basic_dir);

    char index_basic_dir[128];
    sprintf(index_basic_dir, "../../index");

    {
        int n_data_item, n_query_item, n_user, vec_dim;
        std::vector<VectorMatrix> data = readData(basic_dir, dataset_name, n_data_item, n_query_item, n_user,
                                                  vec_dim);
        VectorMatrix &user = data[0];
        VectorMatrix &data_item = data[1];
        VectorMatrix &query_item = data[2];
        spdlog::info("n_data_item {}, n_query_item {}, n_user {}, vec_dim {}", n_data_item, n_query_item, n_user,
                     vec_dim);

        user.vectorNormalize();

        char qrs128_path[256];
        sprintf(qrs128_path, "%s/%s_QueryRankSearch128.index", index_basic_dir, dataset_name);
        char qrs512_path[256];
        sprintf(qrs512_path, "%s/%s_QueryRankSearch512.index", index_basic_dir, dataset_name);

        QueryRankSearch qrs_128(128, n_data_item, n_user,
                                dataset_name, n_sample_query, sample_topk, "../..");
        QueryRankSearch qrs_512(512, n_data_item, n_user,
                                dataset_name, n_sample_query, sample_topk, "../..");

        const int index_size_gb = 256;

        char rs128_path[256];
        sprintf(rs128_path, "%s/%s_RankSearch128-TopT-index_size_gb_%d.index", index_basic_dir, dataset_name,
                index_size_gb);
        char rs512_path[256];
        sprintf(rs512_path, "%s/%s_RankSearch512-TopT-index_size_gb_%d.index", index_basic_dir, dataset_name,
                index_size_gb);

        int topt;
        TopTIPParameter(n_data_item, n_user, index_size_gb, topt);

        RankSearch rs_128(128, n_data_item, n_user);
        RankSearch rs_512(512, n_data_item, n_user);

        //Compute Score Table
        ComputeScoreTable cst(user, data_item);
        std::vector<DistancePair> distance_pair_l(n_data_item);

        TimeRecord record;
        record.reset();
        TimeRecord component_record;

        double query_rank_search_time = 0;
        double rank_search_time = 0;

        for (int userID = 0; userID < n_user; userID++) {
            cst.ComputeSortItems(userID, distance_pair_l.data());

            component_record.reset();
            qrs_128.LoopPreprocess(distance_pair_l.data(), userID);
            qrs_512.LoopPreprocess(distance_pair_l.data(), userID);
            query_rank_search_time += component_record.get_elapsed_time_second();

            component_record.reset();
            rs_128.LoopPreprocess(distance_pair_l.data(), userID);
            rs_512.LoopPreprocess(distance_pair_l.data(), userID);
            rank_search_time += component_record.get_elapsed_time_second();

            if (userID != 0 && userID % cst.report_every_ == 0) {
                std::cout << "preprocessed " << userID / (0.01 * n_user) << " %, "
                          << record.get_elapsed_time_second() << " s/iter" << " Mem: "
                          << get_current_RSS() / 1000000 << " Mb \n";
                spdlog::info(
                        "Compute Score Time {}s, Sort Score Time {}s, Query Rank Search Time {}s, Rank Search Time {}s",
                        cst.compute_time_, cst.sort_time_, query_rank_search_time, rank_search_time);
                cst.compute_time_ = 0;
                cst.sort_time_ = 0;
                query_rank_search_time = 0;
                rank_search_time = 0;
                record.reset();
            }
        }
        cst.FinishCompute();
        qrs_128.SaveIndex(qrs128_path);
        qrs_512.SaveIndex(qrs512_path);

        rs_128.SaveIndex(rs128_path);
        rs_512.SaveIndex(rs512_path);
    }

    {
        const int index_size_gb = 256;
        char rank_sample_128_path[256];
        sprintf(rank_sample_128_path, "%s/%s_RankSearch%d-TopT-index_size_gb_%d.index", index_basic_dir, dataset_name,
                128, index_size_gb);
        RankSamplePrintPruneRatio::MeasurePruneRatio(dataset_name, basic_dir, rank_sample_128_path, 128);

        char rank_sample_512_path[256];
        sprintf(rank_sample_512_path, "%s/%s_RankSearch%d-TopT-index_size_gb_%d.index", index_basic_dir, dataset_name,
                512, index_size_gb);
        RankSamplePrintPruneRatio::MeasurePruneRatio(dataset_name, basic_dir, rank_sample_512_path, 512);
    }

    {
        char query_rank_search_128_path[256];
        sprintf(query_rank_search_128_path, "%s/%s_QueryRankSearch%d.index", index_basic_dir, dataset_name, 128);
        QueryRankSamplePrintPruneRatio::MeasurePruneRatio(dataset_name, basic_dir, query_rank_search_128_path, 128);

        char query_rank_search_512_path[256];
        sprintf(query_rank_search_512_path, "%s/%s_QueryRankSearch%d.index", index_basic_dir, dataset_name, 512);
        QueryRankSamplePrintPruneRatio::MeasurePruneRatio(dataset_name, basic_dir, query_rank_search_512_path, 512);
    }

    return 0;
}