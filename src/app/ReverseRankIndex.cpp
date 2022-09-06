//
// Created by bianzheng on 2022/4/29.
//

#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/VectorMatrix.hpp"

#include "BruteForce/QRSSDTopTIPRefineOrder.hpp"
#include "BruteForce/QRSTopTIP.hpp"
#include "BruteForce/QRSTopTIPRefineOrder.hpp"
#include "BruteForce/RSTopTIP.hpp"
#include "BruteForce/RSTopTIPRefineOrder.hpp"

#include "GridIndex.hpp"

#include "QueryRankSampleScoreDistribution.hpp"
#include "QueryRankSample.hpp"
#include "RankSample.hpp"
#include "RankSampleStoreID.hpp"

#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <vector>
#include <string>

class Parameter {
public:
    std::string basic_dir, dataset_name, method_name;
    int n_sample, n_sample_score_distribution, n_sample_query, sample_topk;
    uint64_t index_size_gb;
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

            ("n_sample, ns", po::value<int>(&para.n_sample)->default_value(20),
             "number of sample of a rank bound")
            ("n_sample_score_distribution, nssd", po::value<int>(&para.n_sample_score_distribution)->default_value(8),
             "number of sample of the score distribution in a rank bound")
            ("index_size_gb, tt", po::value<uint64_t>(&para.index_size_gb)->default_value(50),
             "index size, in unit of GB")
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
    if (method_name == "QRSSDTopTIPRefineOrder") {
        const int n_sample = para.n_sample;
        const int n_sample_score_distribution = para.n_sample_score_distribution;
        const int n_sample_query = para.n_sample_query;
        const int sample_topk = para.sample_topk;
        const uint64_t index_size_gb = para.index_size_gb;
        spdlog::info(
                "input parameter: n_sample {} index_size_gb {} n_sample_score_distribution {} n_sample_query {} sample_topk {}",
                n_sample, index_size_gb, n_sample_score_distribution, n_sample_query, sample_topk);
        index = QRSSDTopTIPRefineOrder::BuildIndex(data_item, user,
                                                   index_path, dataset_name,
                                                   n_sample, n_sample_score_distribution,
                                                   index_size_gb,
                                                   n_sample_query, sample_topk);
        sprintf(parameter_name, "n_sample_%d-index_size_gb_%ld-n_sample_score_distribution_%d",
                n_sample, index_size_gb, n_sample_score_distribution);

    } else if (method_name == "QRSTopTIP") {
        const int n_sample = para.n_sample;
        const uint64_t index_size_gb = para.index_size_gb;
        const int n_sample_query = para.n_sample_query;
        const int sample_topk = para.sample_topk;
        spdlog::info("input parameter: n_sample {} n_sample_query {} sample_topk {}",
                     n_sample, n_sample_query, sample_topk);
        index = QRSTopTIP::BuildIndex(data_item, user, index_path,
                                      n_sample, index_size_gb,
                                      dataset_name, n_sample_query, sample_topk);
        sprintf(parameter_name, "n_sample_%d-index_size_gb_%lu", n_sample, index_size_gb);

    } else if (method_name == "QRSTopTIPRefineOrder") {
        const int n_sample = para.n_sample;
        const uint64_t index_size_gb = para.index_size_gb;
        const int n_sample_query = para.n_sample_query;
        const int sample_topk = para.sample_topk;
        spdlog::info("input parameter: n_sample {} n_sample_query {} sample_topk {}",
                     n_sample, n_sample_query, sample_topk);
        index = QRSTopTIPRefineOrder::BuildIndex(data_item, user, index_path,
                                                 n_sample, index_size_gb,
                                                 dataset_name, n_sample_query, sample_topk);
        sprintf(parameter_name, "n_sample_%d-index_size_gb_%lu", n_sample, index_size_gb);

    } else if (method_name == "RSTopTIP") {
        const int n_sample = para.n_sample;
        const uint64_t index_size_gb = para.index_size_gb;
        spdlog::info("input parameter: n_sample {}, index_size_gb {}",
                     n_sample, index_size_gb);
        index = RSTopTIP::BuildIndex(data_item, user, index_path,
                                     n_sample, index_size_gb);
        sprintf(parameter_name, "n_sample_%d-index_size_gb_%lu",
                n_sample, index_size_gb);

    } else if (method_name == "RSTopTIPRefineOrder") {
        const int n_sample = para.n_sample;
        const uint64_t index_size_gb = para.index_size_gb;
        spdlog::info("input parameter: n_sample {}, index_size_gb {}",
                     n_sample, index_size_gb);
        index = RSTopTIPRefineOrder::BuildIndex(data_item, user, index_path,
                                                n_sample, index_size_gb);
        sprintf(parameter_name, "n_sample_%d-index_size_gb_%lu",
                n_sample, index_size_gb);

    } else if (method_name == "GridIndex") {
        ///Online
        spdlog::info("input parameter: none");
        index = GridIndex::BuildIndex(data_item, user);

    } else if (method_name == "QueryRankSample") {
        const int n_sample = para.n_sample;
        const int n_sample_query = para.n_sample_query;
        const int sample_topk = para.sample_topk;
        spdlog::info("input parameter: n_sample {} n_sample_query {} sample_topk {}",
                     n_sample, n_sample_query, sample_topk);
        index = QueryRankSample::BuildIndex(data_item, user, index_path, dataset_name,
                                            n_sample, n_sample_query, sample_topk);
        sprintf(parameter_name, "n_sample_%d", n_sample);

    } else if (method_name == "QueryRankSampleScoreDistribution") {
        const int n_sample = para.n_sample;
        const int n_sample_score_distribution = para.n_sample_score_distribution;
        const int n_sample_query = para.n_sample_query;
        const int sample_topk = para.sample_topk;
        spdlog::info("input parameter: n_sample {} n_sample_score_distribution {} n_sample_query {} sample_topk {}",
                     n_sample, n_sample_score_distribution, n_sample_query, sample_topk);
        index = QueryRankSampleScoreDistribution::BuildIndex(data_item, user, index_path, dataset_name,
                                                             n_sample, n_sample_score_distribution,
                                                             n_sample_query, sample_topk);
        sprintf(parameter_name, "n_sample_%d-n_sample_score_distribution_%d", n_sample, n_sample_score_distribution);

    } else if (method_name == "RankSample") {
        const int n_sample = para.n_sample;
        spdlog::info("input parameter: n_sample {}", n_sample);
        index = RankSample::BuildIndex(data_item, user, index_path, n_sample);
        sprintf(parameter_name, "n_sample_%d", n_sample);

    } else if (method_name == "RankSampleStoreID") {
        const int n_sample = para.n_sample;
        spdlog::info("input parameter: n_sample {}", n_sample);
        index = RankSampleStoreID::BuildIndex(data_item, user, index_path, n_sample);
        sprintf(parameter_name, "n_sample_%d", n_sample);

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

//    vector<int> topk_l{500, 400, 300, 200, 100, 50, 40, 30, 30, 20, 10};
//    vector<int> topk_l{50, 40, 30, 30, 20, 10};
    vector<int> topk_l{30, 20, 10};
//    vector<int> topk_l{10000, 8192, 4096, 2048, 1024, 512, 256, 128, 64, 32, 16, 8};
    RetrievalResult config;
    vector<vector<vector<UserRankElement>>> result_rank_l;
    vector<vector<SingleQueryPerformance>> query_performance_topk_l;
    const int n_execute_query = n_query_item;
    for (int topk: topk_l) {
        record.reset();
        vector<SingleQueryPerformance> query_performance_l(n_execute_query);
        vector<vector<UserRankElement>> result_rk = index->Retrieval(query_item, topk, n_execute_query,
                                                                     query_performance_l);
//        vector<vector<UserRankElement>> result_rk = index->Retrieval(query_item, topk, n_query_item);

        double retrieval_time = record.get_elapsed_time_second();
        double ms_per_query = retrieval_time / n_query_item * 1000;

        string performance_str = index->PerformanceStatistics(topk, retrieval_time, ms_per_query);
        config.AddRetrievalInfo(performance_str, topk, retrieval_time, ms_per_query);

        result_rank_l.emplace_back(result_rk);
        query_performance_topk_l.emplace_back(query_performance_l);
        spdlog::info("finish top-{}", topk);
        spdlog::info("{}", performance_str);
    }

    spdlog::info("build index time: total {}s", build_index_time);
    int n_topk = (int) topk_l.size();

    for (int i = 0; i < n_topk; i++) {
        cout << config.GetConfig(i) << endl;
        WriteRankResult(result_rank_l[i], dataset_name, method_name.c_str(), parameter_name);
        WriteQueryPerformance(query_performance_topk_l[i], dataset_name, method_name.c_str(), topk_l[i],
                              parameter_name);
    }

    config.AddBuildIndexInfo(index->BuildIndexStatistics());
    config.AddBuildIndexTime(build_index_time);
    config.AddExecuteQuery(n_execute_query);
    config.WritePerformance(dataset_name, method_name.c_str(), parameter_name);
    return 0;
}