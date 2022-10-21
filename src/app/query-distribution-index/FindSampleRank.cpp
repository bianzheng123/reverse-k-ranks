//
// Created by BianZheng on 2022/10/21.
//

///given the query distribution index, find the sampled rank

#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/VectorMatrix.hpp"

#include "alg/RankBoundRefinement/QueryRankSearchSearchKthRank.hpp"

#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <vector>
#include <string>

class Parameter {
public:
    std::string index_dir;
    int n_sample, n_sample_query, sample_topk;
    std::string dataset_name;
    int n_data_item, n_user;
};

void LoadOptions(int argc, char **argv, Parameter &para) {
    namespace po = boost::program_options;

    po::options_description opts("Allowed options");
    opts.add_options()
            ("help,h", "help info")
            ("index_dir, id",
             po::value<std::string>(&para.index_dir)->default_value("/home/bianzheng/reverse-k-ranks/index"),
             "the directory of the index")
            ("n_sample, ns", po::value<int>(&para.n_sample)->default_value(-1),
             "number of sample of a rank bound")
            ("n_sample_query, ns", po::value<int>(&para.n_sample_query)->default_value(150),
             "number of sample query")
            ("sample_topk, ns", po::value<int>(&para.sample_topk)->default_value(50),
             "topk in the sampled distribution")

            ("dataset_name, id",
             po::value<std::string>(&para.dataset_name)->default_value("fake-normal"),
             "dataset_name")
            ("n_data_item, ndi", po::value<int>(&para.n_data_item)->default_value(50),
             "number of data item")
            ("n_user, nu", po::value<int>(&para.n_user)->default_value(50),
             "number of user");

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
    const char *index_dir = para.index_dir.c_str();
    spdlog::info("FindSampleRank dataset_name {}, index_dir {}", dataset_name, index_dir);

    const int n_data_item = para.n_data_item;
    const int n_user = para.n_user;
    spdlog::info("n_data_item {}, n_user {}", n_data_item, n_user);

    const int n_sample = para.n_sample;
    const int n_sample_query = para.n_sample_query;
    const int sample_topk = para.sample_topk;
    spdlog::info("n_sample {}, n_sample_query {}, sample_topk {}", n_sample, n_sample_query, sample_topk);

    TimeRecord record;
    record.reset();

    //rank search
    QueryRankSearchSearchKthRank rank_ins(n_sample, n_data_item, n_user, dataset_name,
                                          n_sample_query, sample_topk, index_dir);
    rank_ins.SaveIndex(index_dir, dataset_name, false);

    double build_index_time = record.get_elapsed_time_second();
    spdlog::info("finish preprocess and save the index");

    RetrievalResult config;

    spdlog::info("FindSampleRank build index time: total {}s", build_index_time);

    config.AddBuildIndexTime(build_index_time);
    char parameter_name[256];
    sprintf(parameter_name, "n_sample_%d-n_sample_query_%d-sample_topk_%d", n_sample, n_sample_query, sample_topk);
    config.WritePerformance(dataset_name, "FindSampleRank", parameter_name);
    spdlog::info("FindSampleRank finish");
    return 0;
}