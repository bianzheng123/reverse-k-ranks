//
// Created by BianZheng on 2022/11/4.
//

#include "util/TimeMemory.hpp"

#include "alg/RankBoundRefinement/QueryRankSearchSearchKthRank.hpp"
#include "alg/RankBoundRefinement/RankSearch.hpp"

#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <string>

class Parameter {
public:
    std::string index_dir, dataset_name, old_method_name, new_method_name, index_type;
    int n_sample, n_sample_query, sample_topk;
    bool has_sample_score;
};

void LoadOptions(int argc, char **argv, Parameter &para) {
    namespace po = boost::program_options;

    po::options_description opts("Allowed options");
    opts.add_options()
            ("help,h", "help info")
            ("index_dir, id",
             po::value<std::string>(&para.index_dir)->default_value("/home/bianzheng/reverse-k-ranks/index"),
             "index directory")
            ("dataset_name, dn",
             po::value<std::string>(&para.dataset_name)->default_value("fake-normal"),
             "dataset_name")
            ("old_method_name, omn",
             po::value<std::string>(&para.old_method_name)->default_value("QueryRankSearchKthRank"),
             "old method name")
            ("new_method_name, dn",
             po::value<std::string>(&para.new_method_name)->default_value("QueryRankSearchLinearRegression"),
             "new method name")
            ("index_type, it",
             po::value<std::string>(&para.index_type)->default_value("QueryRankSearchKthRank"),
             "the new index path")

            ("n_sample, ns",
             po::value<int>(&para.n_sample)->default_value(20),
             "new method name")
            ("n_sample_query, nsq",
             po::value<int>(&para.n_sample_query)->default_value(150),
             "new method name")
            ("sample_topk, st",
             po::value<int>(&para.sample_topk)->default_value(60),
             "new method name")

            ("has_sample_score, hss",
             po::value<bool>(&para.has_sample_score)->default_value(true),
             "has sample score");

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
    const char *old_method_name = para.old_method_name.c_str();
    const char *new_method_name = para.new_method_name.c_str();
    const std::string index_type = para.index_type;
    spdlog::info("TransformIndex index_dir {}", index_dir);
    spdlog::info("dataset_name {}, old_method_name {}, new_method_name {}, index_type {}",
                 dataset_name, old_method_name, new_method_name, index_type);

    const bool has_sample_score = para.has_sample_score;
    const int n_sample = para.n_sample;
    const int n_sample_query = para.n_sample_query;
    const int sample_topk = para.sample_topk;
    spdlog::info("has_sample_score {}, n_sample {}, n_sample_query {}, sample_topk {}",
                 has_sample_score, n_sample, n_sample_query, sample_topk);

    TimeRecord record;
    record.reset();

    if (index_type == "QueryRankSearchKthRank") {
        QueryRankSearchSearchKthRank rank_ins(
                index_dir, dataset_name, old_method_name,
                n_sample, n_sample_query, sample_topk,
                has_sample_score
        );
        rank_ins.SaveNewIndex(index_dir, dataset_name, new_method_name, has_sample_score);

    } else if (index_type == "RankSample") {
        char index_path[256];
        if (has_sample_score) {
            sprintf(index_path,
                    "%s/memory_index/%s-%s-n_sample_%d.index",
                    index_dir, old_method_name, dataset_name, n_sample);
        } else {
            sprintf(index_path,
                    "%s/qrs_to_sample_index/%s-%s-n_sample_%d.index",
                    index_dir, old_method_name, dataset_name, n_sample);
        }
        RankSearch rank_ins(index_path);

        char new_index_path[256];
        if (has_sample_score) {
            sprintf(new_index_path,
                    "%s/new_memory_index/%s-%s-n_sample_%d.index",
                    index_dir, new_method_name, dataset_name, n_sample);
        } else {
            sprintf(new_index_path,
                    "%s/new_qrs_to_sample_index/%s-%s-n_sample_%d.index",
                    index_dir, new_method_name, dataset_name, n_sample);
        }
        rank_ins.SaveNewIndex(new_index_path, has_sample_score);


    } else {
        spdlog::error("not find index type");
        exit(-1);
    }

    spdlog::info("finish preprocess and save the index");
    const double time = record.get_elapsed_time_second();
    spdlog::info("TransformIndex finish time: total {}s", time);
    return 0;
}