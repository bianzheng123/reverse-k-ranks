//
// Created by BianZheng on 2022/10/21.
//

///given the query distribution index, find the sampled rank

#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"

#include "alg/RankBoundRefinement/FindSampleRank/OptimalSampleRank.hpp"
#include "alg/RankBoundRefinement/FindSampleRank/UniformSample.hpp"
#include "alg/RankBoundRefinement/SampleSearch.hpp"

#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <string>

class Parameter {
public:
    int n_sample, n_data_item, n_user;
    std::string index_dir, dataset_name, sample_name, method_name;
    int n_sample_query, sample_topk;
};

void LoadOptions(int argc, char **argv, Parameter &para) {
    namespace po = boost::program_options;

    po::options_description opts("Allowed options");
    opts.add_options()
            ("help,h", "help info")
            ("index_dir, id",
             po::value<std::string>(&para.index_dir)->default_value("/home/bianzheng/reverse-k-ranks/index"),
             "the directory of the index")
            ("dataset_name, id",
             po::value<std::string>(&para.dataset_name)->default_value("fake-normal"),
             "dataset_name")
            ("sample_name, sn",
             po::value<std::string>(&para.sample_name)->default_value("Uniform"),
             "sample_name")
            ("method_name, mn",
             po::value<std::string>(&para.method_name)->default_value("QueryRankSearchKthRank"),
             "method_name")

            ("n_sample, ns", po::value<int>(&para.n_sample)->default_value(-1),
             "number of sample of a rank bound")
            ("n_data_item, ndi", po::value<int>(&para.n_data_item)->default_value(50),
             "number of data item")
            ("n_user, nu", po::value<int>(&para.n_user)->default_value(50),
             "number of user")

            ("n_sample_query, ns", po::value<int>(&para.n_sample_query)->default_value(150),
             "number of sample query")
            ("sample_topk, ns", po::value<int>(&para.sample_topk)->default_value(50),
             "topk in the sampled distribution");

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
    const char *index_dir = para.index_dir.c_str();
    const char *dataset_name = para.dataset_name.c_str();
    const string sample_name = para.sample_name;
    const char *method_name = para.method_name.c_str();
    spdlog::info("FindSampleRank index_dir {}, dataset_name {}, sample_name {}, method_name {}",
                 index_dir, dataset_name, sample_name, method_name);

    const int n_sample = para.n_sample;
    const int n_data_item = para.n_data_item;
    const int n_user = para.n_user;
    spdlog::info("n_sample {}, n_data_item {}, n_user {}", n_sample, n_data_item, n_user);

    const int n_sample_query = para.n_sample_query;
    const int sample_topk = para.sample_topk;
    spdlog::info("n_sample_query {}, sample_topk {}", n_sample_query, sample_topk);

    TimeRecord record;
    record.reset();

    std::vector<int> sample_rank_l(n_sample);
    if (sample_name == "OptimalPart" || sample_name == "OptimalAll") {
        FindOptimalSample(n_user, n_data_item, n_sample,
                          sample_rank_l,
                          n_sample_query, sample_topk,
                          dataset_name, sample_name.c_str(),
                          index_dir);

    } else if (sample_name == "Uniform") {
        FindUniformSample(n_data_item, n_sample, sample_rank_l);

    } else {
        spdlog::error("not such sample method");
        exit(-1);
    }

    //rank search
    SampleSearch rank_ins(n_data_item, n_user, sample_rank_l, n_sample);
    const bool save_sample_score = false;
    const bool is_query_distribution = sample_name != "Uniform";
    rank_ins.SaveIndex(index_dir, dataset_name, method_name,
                       save_sample_score, is_query_distribution,
                       n_sample_query, sample_topk);

    double build_index_time = record.get_elapsed_time_second();
    spdlog::info("finish preprocess and save the index");

    RetrievalResult config;

    spdlog::info("FindSampleRank build index time: total {}s", build_index_time);

    config.AddBuildIndexTime(build_index_time);
    char parameter_name[256];
    if (sample_name == "Uniform") {
        sprintf(parameter_name, "%s-n_sample_%d", method_name, n_sample);
    } else {
        sprintf(parameter_name, "%s-n_sample_%d-n_sample_query_%d-sample_topk_%d", method_name, n_sample, n_sample_query, sample_topk);
    }
    config.WritePerformance(dataset_name, "FindSampleRank", parameter_name);
    spdlog::info("FindSampleRank finish");
    return 0;
}