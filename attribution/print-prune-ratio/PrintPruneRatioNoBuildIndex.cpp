//
// Created by BianZheng on 2022/7/30.
//
//
// Created by BianZheng on 2022/7/29.
//

#include "RankSamplePrintPruneRatio.hpp"
#include "ScoreSamplePrintPruneRatio.hpp"

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
    spdlog::info("PrintPruneRatioNoBuildIndex dataset_name {}, basic_dir {}", dataset_name, basic_dir);

    char index_basic_dir[128];
    sprintf(index_basic_dir, "../../index/%s_constructed_index", dataset_name);

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