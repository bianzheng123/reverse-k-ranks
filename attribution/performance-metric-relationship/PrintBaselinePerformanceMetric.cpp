//
// Created by bianzheng on 2022/4/29.
//

#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "FileIO.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/VectorMatrix.hpp"

#include "RSCompressTopTIPBruteForce.hpp"

#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <vector>
#include <string>

class Parameter {
public:
    std::string basic_dir, dataset_name, method_name;
    int n_sample;
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
            ("method_name, mn", po::value<std::string>(&para.method_name)->default_value("RSCompressTopTIPBruteForce"),
             "method_name")

            ("n_sample, ns", po::value<int>(&para.n_sample)->default_value(2048),
             "the numer of sample")
            ("index_size_gb, tt", po::value<uint64_t>(&para.index_size_gb)->default_value(256),
             "index size, in unit of GB");

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
    spdlog::info("PrintBaselinePerformanceMetric dataset_name {}, basic_dir {}", dataset_name, basic_dir);

    int n_data_item, n_query_item, n_user, vec_dim;
    vector <VectorMatrix> data = readData(basic_dir, dataset_name, n_data_item, n_query_item, n_user,
                                          vec_dim);
    VectorMatrix &user = data[0];
    VectorMatrix &data_item = data[1];
    VectorMatrix &query_item = data[2];
    spdlog::info("n_data_item {}, n_query_item {}, n_user {}, vec_dim {}", n_data_item, n_query_item, n_user, vec_dim);

    char index_path[256];
    sprintf(index_path, "../../index/index");

    TimeRecord record;
    record.reset();
    char parameter_name[256] = "";

    const int n_sample = para.n_sample;
    const uint64_t index_size_gb = para.index_size_gb;
    spdlog::info("input parameter: n_sample {}, index_size_gb {}",
                 n_sample, index_size_gb);
    unique_ptr <RSCompressTopTIPBruteForce::Index> index =
            RSCompressTopTIPBruteForce::BuildIndex(data_item, user,
                                                   index_path,
                                                   n_sample,
                                                   index_size_gb);
    sprintf(parameter_name, "n_sample_%d-index_size_gb_%lu",
            n_sample, index_size_gb);


    double build_index_time = record.get_elapsed_time_second();
    spdlog::info("finish preprocess and save the index");

    const int topk = 10;
    RetrievalResult config;
    record.reset();
    std::vector<size_t> n_remain_candidate_l(n_query_item);
    std::vector<size_t> io_cost_l(n_query_item);
    std::vector<size_t> ip_cost_l(n_query_item);
    index->Retrieval(query_item, topk, n_query_item,
                     n_remain_candidate_l, io_cost_l, ip_cost_l);

    double retrieval_time = record.get_elapsed_time_second();
    double ms_per_query = retrieval_time / n_query_item * 1000;

    string performance_str = index->PerformanceStatistics(topk, retrieval_time, ms_per_query);
    config.AddRetrievalInfo(performance_str, topk, retrieval_time, ms_per_query);

    spdlog::info("finish top-{}", topk);
    spdlog::info("{}", performance_str);

    spdlog::info("build index time: total {}s", build_index_time);
    config.AddBuildIndexInfo(index->BuildIndexStatistics());
    config.AddBuildIndexTime(build_index_time);
    config.AddExecuteQuery(n_query_item);
    config.WritePerformance(dataset_name, method_name.c_str(), parameter_name);
    WritePerformanceMetric(n_remain_candidate_l, io_cost_l, ip_cost_l,
                           dataset_name, method_name.c_str(),
                           topk, n_sample, (int) index_size_gb, n_query_item);
    return 0;
}