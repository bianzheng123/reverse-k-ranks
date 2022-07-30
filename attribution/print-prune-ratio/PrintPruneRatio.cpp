//
// Created by BianZheng on 2022/7/29.
//

#include "RankSamplePrintPruneRatio.hpp"
#include "ScoreSamplePrintPruneRatio.hpp"
#include "score_computation/ComputeScoreTable.hpp"

#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <string>

class Parameter {
public:
    std::string basic_dir, dataset_name;
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
             "dataset_name");

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

        char ss128_path[256];
        sprintf(ss128_path, "%s/%s_ScoreSearch128.index", index_basic_dir, dataset_name);
        char ss512_path[256];
        sprintf(ss512_path, "%s/%s_ScoreSearch512.index", index_basic_dir, dataset_name);
        char ss1024_path[256];
        sprintf(ss1024_path, "%s/%s_ScoreSearch1024.index", index_basic_dir, dataset_name);

        ScoreSearch ss_128(128, n_user, n_data_item);
        ScoreSearch ss_512(512, n_user, n_data_item);
        ScoreSearch ss_1024(1024, n_user, n_data_item);

        char rs128_path[256];
        sprintf(rs128_path, "%s/%s_RankSearch128.index", index_basic_dir, dataset_name);
        char rs512_path[256];
        sprintf(rs512_path, "%s/%s_RankSearch512.index", index_basic_dir, dataset_name);

        RankSearch rs_128(128, n_data_item, n_user);
        RankSearch rs_512(512, n_data_item, n_user);

        //Compute Score Table
        ComputeScoreTable cst(user, data_item);
        std::vector<DistancePair> distance_pair_l(n_data_item);

        TimeRecord record;
        record.reset();
        TimeRecord component_record;

        double score_search_time = 0;
        double rank_search_time = 0;

        for (int userID = 0; userID < n_user; userID++) {
            cst.ComputeSortItems(userID, distance_pair_l.data());

            component_record.reset();
            ss_128.LoopPreprocess(distance_pair_l.data(), userID);
            ss_512.LoopPreprocess(distance_pair_l.data(), userID);
            ss_1024.LoopPreprocess(distance_pair_l.data(), userID);
            score_search_time += component_record.get_elapsed_time_second();

            component_record.reset();
            rs_128.LoopPreprocess(distance_pair_l.data(), userID);
            rs_512.LoopPreprocess(distance_pair_l.data(), userID);
            rank_search_time += component_record.get_elapsed_time_second();

            if (userID != 0 && userID % cst.report_every_ == 0) {
                std::cout << "preprocessed " << userID / (0.01 * n_user) << " %, "
                          << record.get_elapsed_time_second() << " s/iter" << " Mem: "
                          << get_current_RSS() / 1000000 << " Mb \n";
                spdlog::info(
                        "Compute Score Time {}s, Sort Score Time {}s, Score Search Time {}s, Rank Search Time {}s",
                        cst.compute_time_, cst.sort_time_, score_search_time, rank_search_time);
                cst.compute_time_ = 0;
                cst.sort_time_ = 0;
                score_search_time = 0;
                rank_search_time = 0;
                record.reset();
            }
        }
        cst.FinishCompute();
        ss_128.SaveIndex(ss128_path);
        ss_512.SaveIndex(ss512_path);
        ss_1024.SaveIndex(ss1024_path);

        rs_128.SaveIndex(rs128_path);
        rs_512.SaveIndex(rs512_path);
    }

    {
        char rank_sample_128_path[256];
        sprintf(rank_sample_128_path, "%s/%s_RankSearch%d.index", index_basic_dir, dataset_name, 128);
        RankSamplePrintPruneRatio::MeasurePruneRatio(dataset_name, basic_dir, rank_sample_128_path, 128);

        char rank_sample_512_path[256];
        sprintf(rank_sample_512_path, "%s/%s_RankSearch%d.index", index_basic_dir, dataset_name, 512);
        RankSamplePrintPruneRatio::MeasurePruneRatio(dataset_name, basic_dir, rank_sample_512_path, 512);
    }

    {
        char score_sample_128_path[256];
        sprintf(score_sample_128_path, "%s/%s_ScoreSearch%d.index", index_basic_dir, dataset_name, 128);
        ScoreSamplePrintPruneRatio::MeasurePruneRatio(dataset_name, basic_dir, score_sample_128_path, 128);

        char score_sample_512_path[256];
        sprintf(score_sample_512_path, "%s/%s_ScoreSearch%d.index", index_basic_dir, dataset_name, 512);
        ScoreSamplePrintPruneRatio::MeasurePruneRatio(dataset_name, basic_dir, score_sample_512_path, 512);

        char score_sample_1024_path[256];
        sprintf(score_sample_1024_path, "%s/%s_ScoreSearch%d.index", index_basic_dir, dataset_name, 1024);
        ScoreSamplePrintPruneRatio::MeasurePruneRatio(dataset_name, basic_dir, score_sample_1024_path, 1024);
    }

    return 0;
}