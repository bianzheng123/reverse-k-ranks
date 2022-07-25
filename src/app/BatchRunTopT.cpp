//
// Created by BianZheng on 2022/7/15.
//

#include "BatchRun/BatchMeasureRetrievalTopT.hpp"
#include "BatchRun/BatchBuildIndexTopTScoreSample.hpp"
#include "BatchRun/BatchRetrievalTopT.hpp"
#include "BatchRun/ScoreSampleMeasurePruneRatio.hpp"
#include "util/TimeMemory.hpp"

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
    spdlog::info("TopT dataset_name {}, basic_dir {}", dataset_name, basic_dir);

    double build_index_time;
    {
        TimeRecord record;
        record.reset();
        BuildIndex(basic_dir, dataset_name);

        build_index_time = record.get_elapsed_time_second();
        spdlog::info("finish preprocess and save the index, build index time {}s", build_index_time);
    }

    {
        ScoreSampleMeasurePruneRatio::MeasurePruneRatio(dataset_name, basic_dir, 128);
    }

    {
        //measure TopTID
        const int index_size_gb = 256;
        const int n_sample = 128;
        char disk_path[256];
        sprintf(disk_path, "../index/%s_TopTID%d.index", dataset_name, index_size_gb);
        char memory_path[256];
        sprintf(memory_path, "../index/%s_ScoreSearch%d.index", dataset_name, n_sample);
        BatchMeasureRetrievalTopT::MeasureTopT("MeasureScoreSampleTopTID",
                                               disk_path, memory_path,
                                               n_sample, index_size_gb,
                                               basic_dir, dataset_name);
    }

    {
        //search on TopTIP
        const int index_size_gb = 256;
        const int n_sample = 128;
        char disk_path[256];
        sprintf(disk_path, "../index/%s_TopTIP%d.index", dataset_name, index_size_gb);
        char memory_path[256];
        sprintf(memory_path, "../index/%s_ScoreSearch%d.index", dataset_name, n_sample);
        BatchMeasureRetrievalTopT::MeasureTopT("MeasureScoreSampleTopTIP",
                                               disk_path, memory_path,
                                               n_sample, index_size_gb,
                                               basic_dir, dataset_name);
    }

    {
        //search on TopTID
        const int index_size_gb = 256;
        const int n_sample = 128;
        char disk_path[256];
        sprintf(disk_path, "../index/%s_TopTID%d.index", dataset_name, index_size_gb);
        char memory_path[256];
        sprintf(memory_path, "../index/%s_ScoreSearch%d.index", dataset_name, n_sample);
        RunRetrieval(disk_path, memory_path,
                     n_sample, index_size_gb,
                     basic_dir, dataset_name, "CompressTopTIDBruteForceBatchRun");
    }

    {
        //search on TopTIP
        const int index_size_gb = 256;
        const int n_sample = 128;
        char disk_path[256];
        sprintf(disk_path, "../index/%s_TopTIP%d.index", dataset_name, index_size_gb);
        char memory_path[256];
        sprintf(memory_path, "../index/%s_ScoreSearch%d.index", dataset_name, n_sample);
        RunRetrieval(disk_path, memory_path,
                     n_sample, index_size_gb,
                     basic_dir, dataset_name, "CompressTopTIPBruteForceBatchRun");
    }

//    const char *toptID128_path = "../index/Amazon_TopTID128.index";
//    const char *toptID256_path = "../index/Amazon_TopTID256.index";
//    const char *toptIP128_path = "../index/Amazon_TopTIP128.index";
//    const char *toptIP256_path = "../index/Amazon_TopTIP256.index";
//    const char *ss128_path = "../index/Amazon_ScoreSearch128.index";
//    const char *ss1024_path = "../index/Amazon_ScoreSearch1024.index";

    return 0;
}