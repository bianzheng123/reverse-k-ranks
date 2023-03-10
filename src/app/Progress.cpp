//
// Created by bianzheng on 2022/5/3.
//

#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/VectorMatrix.hpp"

#include "Progress/BatchDiskBruteForce.hpp"
#include "Progress/DiskBruteForce.hpp"
#include "Progress/MemoryBruteForce.hpp"
#include "Progress/QueryRankSampleSearchBruteForce.hpp"

#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <vector>
#include <string>

class Parameter {
public:
    std::string basic_dir, dataset_name, method_name, index_dir;
    int n_sample, n_sample_query, sample_topk;
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
            ("method_name, mn", po::value<std::string>(&para.method_name)->default_value("BatchDiskBruteForce"),
             "method_name")
            ("index_dir, id",
             po::value<std::string>(&para.index_dir)->default_value("/home/bianzheng/reverse-k-ranks/index"),
             "the directory of the index")

            ("n_sample, ns", po::value<int>(&para.n_sample)->default_value(20),
             "number of sample of a rank bound")
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
    string method_name = para.method_name;
    const char* index_dir = para.index_dir.c_str();
    spdlog::info("{} dataset_name {}, basic_dir {}", method_name, dataset_name, basic_dir);

    int n_data_item, n_query_item, n_user, vec_dim;
    vector<VectorMatrix> data = readData(basic_dir, dataset_name, n_data_item, n_query_item, n_user,
                                         vec_dim);
    VectorMatrix &user = data[0];
    VectorMatrix &data_item = data[1];
    VectorMatrix &query_item = data[2];
    spdlog::info("n_data_item {}, n_query_item {}, n_user {}, vec_dim {}", n_data_item, n_query_item, n_user, vec_dim);

    char index_path[256];
    sprintf(index_path, "../index/index");

    TimeRecord record;
    record.reset();
    unique_ptr<BaseIndex> index;
    char parameter_name[256] = "";
    if (method_name == "BatchDiskBruteForce") {
        ///BruteForce
        spdlog::info("input parameter: none");
        index = BatchDiskBruteForce::BuildIndex(data_item, user, index_path);

    } else if (method_name == "DiskBruteForce") {
        spdlog::info("input parameter: none");
        index = DiskBruteForce::BuildIndex(data_item, user, index_path);

    } else if (method_name == "MemoryBruteForce") {
        spdlog::info("input parameter: none");
        index = MemoryBruteForce::BuildIndex(data_item, user);

    } else if (method_name == "QueryRankSampleSearchBruteForce") {
        const int n_sample = para.n_sample;
        const int n_sample_query = para.n_sample_query;
        const int sample_topk = para.sample_topk;
        spdlog::info("input parameter: n_sample {} n_sample_query {} sample_topk {}",
                     n_sample, n_sample_query, sample_topk);
        index = QueryRankSampleSearchBruteForce::BuildIndex(data_item, user, index_path, dataset_name,
                                                            n_sample, n_sample_query, sample_topk, index_dir);
        sprintf(parameter_name, "n_sample_%d", n_sample);

    } else {
        spdlog::error("not such method");
    }

    double build_index_time = record.get_elapsed_time_second();
    spdlog::info("finish preprocess and save the index");

    vector<int> topk_l{30, 20, 10};
//    vector<int> topk_l{10};
    RetrievalResult config;
    vector<vector<vector<UserRankElement>>> result_rank_l;
    vector<vector<SingleQueryPerformance>> query_performance_topk_l;
    for (int topk: topk_l) {
        record.reset();
        vector<SingleQueryPerformance> query_performance_l(n_query_item);
        vector<vector<UserRankElement>> result_rk = index->Retrieval(query_item, topk, n_query_item,
                                                                     query_performance_l);

        string performance_str = index->PerformanceStatistics(topk);
        config.AddRetrievalInfo(performance_str);

        query_performance_topk_l.emplace_back(query_performance_l);

        result_rank_l.emplace_back(result_rk);
        spdlog::info("finish top-{}", topk);
    }

    spdlog::info("build index time: total {}s", build_index_time);
    int n_topk = (int) topk_l.size();

    for (int i = 0; i < n_topk; i++) {
        cout << config.GetConfig(i) << endl;
        const int topk = topk_l[i];
        WriteRankResult(result_rank_l[i], topk, dataset_name, method_name.c_str(), parameter_name);
        WriteQueryPerformance(query_performance_topk_l[i], dataset_name, method_name.c_str(), topk_l[i],
                              parameter_name);
    }

    config.AddMemoryInfo(index->IndexSizeByte());
    config.AddBuildIndexTime(build_index_time);
    config.WritePerformance(dataset_name, method_name.c_str(), parameter_name);
    return 0;
}