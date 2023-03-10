//
// Created by BianZheng on 2022/12/8.
//

#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/VectorMatrix.hpp"

#include "SimpferFEXIPROTopkOnly.hpp"
#include "struct/MethodBase.hpp"

#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <vector>
#include <string>

class Parameter {
public:
    std::string dataset_dir, dataset_name, method_name, index_dir;
    bool test_topk;
    int simpfer_k_max, test_reverse_topk;
    size_t stop_time;
    int n_bit;
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
            ("test_topk, tt",
             po::value<bool>(&para.test_topk)->default_value(true),
             "whether is test topk")
            ("method_name, mn", po::value<std::string>(&para.method_name)->default_value("BatchDiskBruteForce"),
             "method_name")
            // reverse top-k adaption
            ("simpfer_k_max, skm", po::value<int>(&para.simpfer_k_max)->default_value(25),
             "k_max in simpfer")
            ("stop_time, st", po::value<size_t>(&para.stop_time)->default_value(60),
             "stop time, in unit of second")
            ("test_reverse_topk, st", po::value<int>(&para.test_reverse_topk)->default_value(60),
             "test_reverse_topk")
            // score distribution parameter
            ("n_bit, nb", po::value<int>(&para.n_bit)->default_value(8),
             "number of bit");

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
    vector <VectorMatrix> data = readData(dataset_dir, dataset_name, n_data_item, n_query_item, n_user,
                                          vec_dim);
    VectorMatrix &user = data[0];
    VectorMatrix &data_item = data[1];
    VectorMatrix &query_item = data[2];
    spdlog::info("n_data_item {}, n_query_item {}, n_user {}, vec_dim {}", n_data_item, n_query_item, n_user, vec_dim);

    char index_path[256];
    sprintf(index_path, "%s/%s.index", index_dir, dataset_name);

    TimeRecord record;
    record.reset();
    unique_ptr <BaseIndex> index;
    char parameter_name[256] = "";
    if (method_name == "SimpferFEXIPROTopkOnly") {
        const int simpfer_k_max = para.simpfer_k_max;
        const size_t stop_time = para.stop_time;
        const int test_reverse_topk = para.test_reverse_topk;
        spdlog::info("input parameter: simpfer_k_max {}, stop_time {}s, test_reverse_topk {}s",
                     simpfer_k_max, stop_time, test_reverse_topk);
        index = SimpferFEXIPROTopkOnly::BuildIndex(data_item, user, simpfer_k_max, stop_time, test_reverse_topk);
        sprintf(parameter_name, "simpfer_k_max_%d", simpfer_k_max);

    } else {
        spdlog::error("not such method");
    }

    double build_index_time = record.get_elapsed_time_second();
    spdlog::info("finish preprocess and save the index");

//    vector<int> topk_warm_up_l = {1, 1, 1, 1, 1, 1};
//    for (int topk: topk_warm_up_l) {
//        int n_query_item;
//        VectorMatrix &query_item = readQueryData(dataset_dir, dataset_name, vec_dim, n_query_item);
//        const int n_execute_query = n_query_item;
//
//        vector<SingleQueryPerformance> query_performance_l(n_execute_query);
//        vector<vector<UserRankElement>> result_rk = index->Retrieval(query_item, topk, n_execute_query,
//                                                                     query_performance_l);
//
//        string performance_str = index->PerformanceStatistics(topk);
//        spdlog::info("finish top-{}", topk);
//        spdlog::info("{}", performance_str);
//    }

    int n_execute_query = n_query_item;
    vector<int> topk_l;
    if (para.test_topk) {
        topk_l = {30, 20, 10};
//        topk_l = {10};
    } else if (method_name == "Simpfer" || method_name == "SimpferOnly" || method_name == "SimpferFEXIPROOnly" ||
               method_name == "SimpferFEXIPROTopkOnly") {
        topk_l = {50};
        n_execute_query = 100;
    } else {
        topk_l = {200, 150, 100, 50, 10};
    }

    RetrievalResult config;
    vector < vector < vector < UserRankElement>>> result_rank_l;
    vector <vector<SingleQueryPerformance>> query_performance_topk_l;

    for (int topk: topk_l) {
        vector <SingleQueryPerformance> query_performance_l(n_execute_query);
        vector <vector<UserRankElement>> result_rk = index->Retrieval(query_item, topk, n_execute_query,
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