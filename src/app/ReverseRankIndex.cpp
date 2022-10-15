//
// Created by bianzheng on 2022/4/29.
//

#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/VectorMatrix.hpp"

#include "GridIndex.hpp"
#include "LinearModel.hpp"
#include "QueryRankSampleSearchAllRank.hpp"
#include "QueryRankSampleSearchKthRank.hpp"
#include "RankSample.hpp"
#include "RankSampleIntLR.hpp"
#include "RankSampleIntPGM.hpp"
#include "Simpfer.hpp"

#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <vector>
#include <string>

class Parameter {
public:
    std::string dataset_dir, dataset_name, method_name, index_dir;
    int n_sample, n_sample_query, sample_topk;
    int simpfer_k_max;
};

void LoadOptions(int argc, char **argv, Parameter &para) {
    namespace po = boost::program_options;

    po::options_description opts("Allowed options");
    opts.add_options()
            ("help,h", "help info")
            ("dataset_dir,dd",
             po::value<std::string>(&para.dataset_dir)->default_value("/home/bianzheng/Dataset/ReverseMIPS"),
             "the basic directory of dataset")
            ("dataset_name, dn", po::value<std::string>(&para.dataset_name)->default_value("fake-normal"),
             "dataset_name")
            ("index_dir, id",
             po::value<std::string>(&para.index_dir)->default_value("/home/bianzheng/reverse-k-ranks/index"),
             "the directory of the index")
            ("method_name, mn", po::value<std::string>(&para.method_name)->default_value("BatchDiskBruteForce"),
             "method_name")
            // memory index parameter
            ("n_sample, ns", po::value<int>(&para.n_sample)->default_value(20),
             "number of sample of a rank bound")
            ("n_sample_query, nsq", po::value<int>(&para.n_sample_query)->default_value(150),
             "the numer of sample query in training query distribution")
            ("sample_topk, st", po::value<int>(&para.sample_topk)->default_value(50),
             "topk in training query distribution")
            // reverse top-k adaption
            ("simpfer_k_max, skm", po::value<int>(&para.simpfer_k_max)->default_value(25),
             "k_max in simpfer");

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
    const char *dataset_dir = para.dataset_dir.c_str();
    const char *index_dir = para.index_dir.c_str();
    string method_name = para.method_name;
    spdlog::info("{} dataset_name {}, dataset_dir {}", method_name, dataset_name, dataset_dir);
    spdlog::info("index_dir {}", index_dir);

    int n_data_item, n_query_item, n_user, vec_dim;
    vector<VectorMatrix> data = readData(dataset_dir, dataset_name, n_data_item, n_query_item, n_user,
                                         vec_dim);
    VectorMatrix &user = data[0];
    VectorMatrix &data_item = data[1];
    VectorMatrix &query_item = data[2];
    spdlog::info("n_data_item {}, n_query_item {}, n_user {}, vec_dim {}", n_data_item, n_query_item, n_user, vec_dim);

    char index_path[256];
    sprintf(index_path, "%s/%s.index", index_dir, dataset_name);

    TimeRecord record;
    record.reset();
    unique_ptr<BaseIndex> index;
    char parameter_name[256] = "";
    if (method_name == "GridIndex") {
        ///Online
        spdlog::info("input parameter: none");
        index = GridIndex::BuildIndex(data_item, user);

    } else if (method_name == "LinearModel") {
        spdlog::info("input parameter: none");
        index = LinearModel::BuildIndex(data_item, user, index_path);

    } else if (method_name == "QueryRankSampleSearchAllRank") {
        const int n_sample = para.n_sample;
        const int n_sample_query = para.n_sample_query;
        const int sample_topk = para.sample_topk;
        spdlog::info("input parameter: n_sample {} n_sample_query {} sample_topk {}",
                     n_sample, n_sample_query, sample_topk);
        index = QueryRankSampleSearchAllRank::BuildIndex(data_item, user, index_path, dataset_name,
                                                         n_sample, n_sample_query, sample_topk, index_dir);
        sprintf(parameter_name, "n_sample_%d", n_sample);

    } else if (method_name == "QueryRankSampleSearchKthRank") {
        const int n_sample = para.n_sample;
        const int n_sample_query = para.n_sample_query;
        const int sample_topk = para.sample_topk;
        spdlog::info("input parameter: n_sample {} n_sample_query {} sample_topk {}",
                     n_sample, n_sample_query, sample_topk);
        index = QueryRankSampleSearchKthRank::BuildIndex(data_item, user, index_path, dataset_name,
                                                         n_sample, n_sample_query, sample_topk, index_dir);
        sprintf(parameter_name, "n_sample_%d", n_sample);

    } else if (method_name == "RankSample") {
        const int n_sample = para.n_sample;
        spdlog::info("input parameter: n_sample {}", n_sample);
        index = RankSample::BuildIndex(data_item, user, index_path, n_sample);
        sprintf(parameter_name, "n_sample_%d", n_sample);

    }else if (method_name == "RankSampleIntLR") {
        const int n_sample = para.n_sample;
        spdlog::info("input parameter: n_sample {}", n_sample);
        index = RankSampleIntLR::BuildIndex(data_item, user, index_path, n_sample);
        sprintf(parameter_name, "n_sample_%d", n_sample);

    }  else if (method_name == "RankSampleIntPGM") {
        const int n_sample = para.n_sample;
        spdlog::info("input parameter: n_sample {}", n_sample);
        index = RankSampleIntPGM::BuildIndex(data_item, user, index_path, n_sample);
        sprintf(parameter_name, "n_sample_%d", n_sample);

    } else if (method_name == "Simpfer") {
        const int simpfer_k_max = para.simpfer_k_max;
        spdlog::info("input parameter: simpfer_k_max {}",
                     simpfer_k_max);
        index = Simpfer::BuildIndex(data_item, user, simpfer_k_max);
        sprintf(parameter_name, "simpfer_k_max_%d", simpfer_k_max);

    } else {
        spdlog::error("not such method");
    }

    double build_index_time = record.get_elapsed_time_second();
    spdlog::info("finish preprocess and save the index");

//    vector<int> topk_warm_up_l{1};
//    for (int topk: topk_warm_up_l) {
//        record.reset();
//        vector<vector<UserRankElement>> result_rk = index->Retrieval(query_item, topk, 50);
//
//        double retrieval_time = record.get_elapsed_time_second();
//        double ms_per_query = retrieval_time / n_query_item * 1000;
//
//        string performance_str = index->PerformanceStatistics(topk, retrieval_time, ms_per_query);
//        spdlog::info("finish warm up top-{}", topk);
//        spdlog::info("{}", performance_str);
//    }

    vector<int> topk_l{600, 500, 100, 50, 30, 10, 1};
//    vector<int> topk_l{60, 50, 40, 30, 20, 10};
//    vector<int> topk_l{30, 20, 10};
//    vector<int> topk_l{10};
//    vector<int> topk_l{10000, 8192, 4096, 2048, 1024, 512, 256, 128, 64, 32, 16, 8};
    RetrievalResult config;
    vector<vector<vector<UserRankElement>>> result_rank_l;
    vector<vector<SingleQueryPerformance>> query_performance_topk_l;
    const int n_execute_query = n_query_item;
    for (int topk: topk_l) {
        vector<SingleQueryPerformance> query_performance_l(n_execute_query);
        vector<vector<UserRankElement>> result_rk = index->Retrieval(query_item, topk, n_execute_query,
                                                                     query_performance_l);

        string performance_str = index->PerformanceStatistics(topk);
        config.AddRetrievalInfo(performance_str);

        result_rank_l.emplace_back(result_rk);
        query_performance_topk_l.emplace_back(query_performance_l);
        spdlog::info("finish top-{}", topk);
        spdlog::info("{}", performance_str);
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
    config.AddExecuteQuery(n_execute_query);
    config.WritePerformance(dataset_name, method_name.c_str(), parameter_name);
    return 0;
}