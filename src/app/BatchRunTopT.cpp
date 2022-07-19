//
// Created by BianZheng on 2022/7/15.
//

#include "BatchRunMethod/BatchBuildIndexTopT.hpp"
#include "BatchRunMethod/BatchRetrievalTopT.hpp"
#include "BatchRunMethod/ScoreSampleMeasurePruneRatio.hpp"
#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "struct/VectorMatrix.hpp"

#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <vector>
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
        //measure prune ratio
        int n_data_item, n_query_item, n_user, vec_dim;
        std::vector<VectorMatrix> data = readData(basic_dir, dataset_name,
                                                  n_data_item, n_query_item, n_user, vec_dim);
        VectorMatrix &user = data[0];
        VectorMatrix &data_item = data[1];
        VectorMatrix &query_item = data[2];
        user.vectorNormalize();

        char memory_path[256];
        sprintf(memory_path, "../index/%s_ScoreSearch128.index", dataset_name);
        std::unique_ptr<ScoreSampleMeasurePruneRatio::Index> index =
                ScoreSampleMeasurePruneRatio::BuildIndex(data_item, user, memory_path);

        std::string method_name = "TopTMeasurePruneRatio";
        char parameter_name[256];
        sprintf(parameter_name, "n_sample_%d", 128);

        ScoreSampleMeasurePruneRatio::RetrievalResultAttribution config;
        std::vector<int> topk_l = {10, 20, 30, 40, 50};
        TimeRecord record;
        for (const int topk: topk_l) {
            record.reset();
            index->Retrieval(query_item, topk);
            const double retrieval_time = record.get_elapsed_time_second();
            const double ms_per_query = retrieval_time / query_item.n_vector_;
            string performance_str = index->PerformanceStatistics(topk, retrieval_time, ms_per_query);
            std::cout << performance_str << std::endl;
            config.AddRetrievalInfo(performance_str, topk, retrieval_time, ms_per_query);
        }

        config.WritePerformance(dataset_name, method_name.c_str(), parameter_name);
    }

//    {
//        //search on TopTIP
//        int n_data_item, n_query_item, n_user, vec_dim;
//        std::vector<VectorMatrix> data = readData(basic_dir, dataset_name,
//                                                  n_data_item, n_query_item, n_user, vec_dim);
//        VectorMatrix &user = data[0];
//        VectorMatrix &data_item = data[1];
//        VectorMatrix &query_item = data[2];
//        user.vectorNormalize();
//
//        //search on TopTID
//        char disk_path[256];
//        sprintf(disk_path, "../index/%s_TopTID128.index", dataset_name);
//        char memory_path[256];
//        sprintf(memory_path, "../index/%s_ScoreSearch128.index", dataset_name);
//        RunRetrieval(user, data_item, query_item,
//                     disk_path, memory_path,
//                     128, 128,
//                     dataset_name, "CompressTopTIDBruteForce", build_index_time);
//    }
//
//    {
//        //search on TopTIP
//        int n_data_item, n_query_item, n_user, vec_dim;
//        std::vector<VectorMatrix> data = readData(basic_dir, dataset_name,
//                                                  n_data_item, n_query_item, n_user, vec_dim);
//        VectorMatrix &user = data[0];
//        VectorMatrix &data_item = data[1];
//        VectorMatrix &query_item = data[2];
//        user.vectorNormalize();
//
//        //search on TopTID
//        char disk_path[256];
//        sprintf(disk_path, "../index/%s_TopTIP128.index", dataset_name);
//        char memory_path[256];
//        sprintf(memory_path, "../index/%s_ScoreSearch128.index", dataset_name);
//        RunRetrieval(user, data_item, query_item,
//                     disk_path, memory_path,
//                     128, 128,
//                     dataset_name, "CompressTopTIPBruteForce", build_index_time);
//    }

//    const char *toptID128_path = "../index/Amazon_TopTID128.index";
//    const char *toptID256_path = "../index/Amazon_TopTID256.index";
//    const char *toptIP128_path = "../index/Amazon_TopTIP128.index";
//    const char *toptIP256_path = "../index/Amazon_TopTIP256.index";
//    const char *ss128_path = "../index/Amazon_ScoreSearch128.index";
//    const char *ss1024_path = "../index/Amazon_ScoreSearch1024.index";

    return 0;
}