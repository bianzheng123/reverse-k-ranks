//
// Created by BianZheng on 2022/10/20.
//

#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/VectorMatrix.hpp"

#include "alg/DiskIndex/ReadAll.hpp"
#include "alg/RankBoundRefinement/RankSearch.hpp"

#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <vector>
#include <string>

class Parameter {
public:
    std::string dataset_dir, dataset_name, index_dir;
    int n_sample;
};

void LoadOptions(int argc, char **argv, Parameter &para) {
    namespace po = boost::program_options;

    po::options_description opts("Allowed options");
    opts.add_options()
            ("help,h", "help info")
            ("dataset_dir,dd",
             po::value<std::string>(&para.dataset_dir)->default_value("/home/bianzheng/Dataset/ReverseMIPS"),
             "the basic directory of dataset")
            ("dataset_name, ds", po::value<std::string>(&para.dataset_name)->default_value("fake-normal"),
             "dataset_name")
            ("index_dir, id",
             po::value<std::string>(&para.index_dir)->default_value("/home/bianzheng/reverse-k-ranks/index"),
             "the directory of the index")
            ("n_sample, ns", po::value<int>(&para.n_sample)->default_value(-1),
             "number of sample of a rank bound");

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

void BuildIndex(const VectorMatrix &data_item, const VectorMatrix &user,
                const std::vector<int64_t> &n_sample_l,
                const char *score_table_path, const char *dataset_name, const char *basic_index_dir) {
    const int n_user = user.n_vector_;
    const int n_data_item = data_item.n_vector_;

    //rank search
    const int n_rs_ins = (int) n_sample_l.size();
    std::vector<RankSearch> rank_search_l(n_rs_ins);
    for (int rsID = 0; rsID < n_rs_ins; rsID++) {
        rank_search_l[rsID] = RankSearch(n_sample_l[rsID], n_data_item, n_user);
    }

    ReadAll read_ins(n_user, n_data_item, score_table_path);
    read_ins.RetrievalPreprocess();

    const int report_every = 10000;
    TimeRecord record;
    record.reset();
    std::vector<double> distance_l(n_data_item);
    for (int userID = 0; userID < n_user; userID++) {
        read_ins.ReadDiskNoCache(userID, distance_l);

        for (int rsID = 0; rsID < n_rs_ins; rsID++) {
            rank_search_l[rsID].LoopPreprocess(distance_l.data(), userID);
        }

        if (userID % report_every == 0) {
            std::cout << "preprocessed " << userID / (0.01 * n_user) << " %, "
                      << record.get_elapsed_time_second() << " s/iter" << " Mem: "
                      << get_current_RSS() / 1000000 << " Mb \n";
            record.reset();
        }
    }

    for (int rsID = 0; rsID < n_rs_ins; rsID++) {
        char rank_search_path[256];
        sprintf(rank_search_path, "%s/memory_index/RankSample-%s-n_sample_%ld.index",
                basic_index_dir, dataset_name, n_sample_l[rsID]);
        rank_search_l[rsID].SaveIndex(rank_search_path);
    }
    read_ins.FinishRetrieval();
}

int main(int argc, char **argv) {
    Parameter para;
    LoadOptions(argc, argv, para);
    const char *dataset_name = para.dataset_name.c_str();
    const char *dataset_dir = para.dataset_dir.c_str();
    string index_dir = para.index_dir;
    spdlog::info("BuildRankSampleIndex dataset_name {}, dataset_dir {}", dataset_name, dataset_dir);
    spdlog::info("index_dir {}", index_dir);

    int n_data_item, n_user, vec_dim;
    vector<VectorMatrix> data = readIndexData(dataset_dir, dataset_name, n_data_item, n_user,
                                              vec_dim);
    VectorMatrix &user = data[0];
    VectorMatrix &data_item = data[1];
    VectorMatrix &query_item = data[2];
    spdlog::info("n_data_item {}, n_user {}, vec_dim {}", n_data_item, n_user, vec_dim);

    std::vector<int> memory_capacity_l = {2, 4, 8, 16, 32};
    std::vector<int64_t> n_sample_l(memory_capacity_l.size());
    const int n_capacity = (int) memory_capacity_l.size();
    for (int capacityID = 0; capacityID < n_capacity; capacityID++) {
        const int64_t memory_capacity = memory_capacity_l[capacityID];
        const int64_t n_sample = memory_capacity * 1024 * 1024 * 1024 / 8 / n_user;
        n_sample_l[capacityID] = n_sample;
        printf("memory_capacity %ld, n_sample %ld\n", memory_capacity, n_sample);
    }

    if (para.n_sample != -1) {
        n_sample_l.clear();
        n_sample_l.push_back(para.n_sample);
    }
    std::string n_sample_info = "n_sample: ";
    const int sample_length = (int) n_sample_l.size();
    for (int sampleID = 0; sampleID < sample_length; sampleID++) {
        n_sample_info += std::to_string(n_sample_l[sampleID]) + " ";
    }
    spdlog::info("{}", n_sample_info);

    char score_table_path[256];
    sprintf(score_table_path, "%s/%s.index", index_dir.c_str(), dataset_name);

    TimeRecord record;
    record.reset();

    BuildIndex(data_item, user, n_sample_l,
               score_table_path, dataset_name, index_dir.c_str());

    double build_index_time = record.get_elapsed_time_second();
    spdlog::info("finish preprocess and save the index");

    RetrievalResult config;

    spdlog::info("BuildRankSampleIndex build index time: total {}s", build_index_time);

    config.AddInfo(n_sample_info);
    config.AddBuildIndexTime(build_index_time);
    char parameter_name[256];
    sprintf(parameter_name, "n_sample_%d", para.n_sample);
    config.WritePerformance(dataset_name, "BuildRankSampleIndex", parameter_name);
    return 0;
}