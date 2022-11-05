//
// Created by BianZheng on 2022/10/21.
//

///given the sampled rank, build the QueryRankSample index

#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/VectorMatrix.hpp"

#include "alg/DiskIndex/ReadAll.hpp"
#include "alg/RankBoundRefinement/SampleSearch.hpp"

#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <vector>
#include <string>

class Parameter {
public:
    std::string dataset_dir, dataset_name, method_name, index_dir;
    int n_sample, n_sample_query, sample_topk;
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
            ("method_name, mn",
             po::value<std::string>(&para.method_name)->default_value("QueryRankSampleSearchKthRank"),
             "method_name")

            ("n_sample, ns", po::value<int>(&para.n_sample)->default_value(-1),
             "number of sample of a rank bound")
            ("n_sample_query, nsq", po::value<int>(&para.n_sample_query)->default_value(9000),
             "number of sampled query")
            ("sample_topk, st", po::value<int>(&para.sample_topk)->default_value(600),
             "topk of sampled query");

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

std::string IndexName(const std::string &method_name) {
    if (method_name == "QueryRankSampleLeastSquareIntLR" || method_name == "QueryRankSampleMinMaxIntLR") {
        return "QueryRankSampleIntLR";
    } else if (method_name == "QueryRankSampleScoreDistribution") {
        return "QueryRankSampleScoreDistribution";
    } else if (method_name == "QueryRankSampleSearchAllRank") {
        return "QueryRankSampleSearchAllRank";
    } else if (method_name == "QueryRankSampleSearchKthRank") {
        return "QueryRankSampleSearchKthRank";
    } else if (method_name == "RankSample") {
        return "RankSample";
    } else {
        spdlog::error("not find method name, program exit");
        exit(-1);
    }
}

void BuildIndex(const VectorMatrix &data_item, const VectorMatrix &user,
                const std::vector<int64_t> &n_sample_l, const int &n_sample_query, const int &sample_topk,
                const char *score_table_path, const char *dataset_name, const string &index_name,
                const char *basic_index_dir) {
    const int n_user = user.n_vector_;
    const int n_data_item = data_item.n_vector_;

    //rank search
    const int n_rs_ins = (int) n_sample_l.size();
    std::vector<SampleSearch> rank_search_l(n_rs_ins);
    const bool load_sample_score = false;
    const bool is_query_distribution = index_name != "RankSample";
    for (int rsID = 0; rsID < n_rs_ins; rsID++) {
        rank_search_l[rsID] = SampleSearch(
                basic_index_dir, dataset_name, index_name.c_str(),
                n_sample_l[rsID], load_sample_score, is_query_distribution, n_sample_query, sample_topk);
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

    const bool save_sample_score = true;
    for (int rsID = 0; rsID < n_rs_ins; rsID++) {
        rank_search_l[rsID].SaveIndex(basic_index_dir, dataset_name, index_name.c_str(),
                                      save_sample_score, is_query_distribution,
                                      n_sample_query, sample_topk);
    }
    read_ins.FinishRetrieval();
}

int64_t ComputeNSample(const std::string &index_name, const int64_t &memory_capacity,
                       const int64_t &n_user, const int64_t &n_data_item, const int64_t &vec_dim) {
    if (index_name == "QueryRankSampleIntLR") {
        return (memory_capacity * 1024 * 1024 * 1024 -
                n_user * 4 * sizeof(double) -
                n_user * vec_dim * sizeof(int) - n_data_item * vec_dim * sizeof(int)) /
               sizeof(double) / n_user;
    } else if (index_name == "QueryRankSampleScoreDistribution") {
        return (memory_capacity * 1024 * 1024 * 1024 + n_user * sizeof(char)) / (sizeof(double) + sizeof(char)) /
               n_user;
    } else if (index_name == "QueryRankSampleSearchAllRank" || index_name == "QueryRankSampleSearchKthRank" ||
               index_name == "RankSample") {
        return memory_capacity * 1024 * 1024 * 1024 / sizeof(double) / n_user;
    } else {
        spdlog::error("no such index name, program exit");
        exit(-1);
    }
}

int main(int argc, char **argv) {
    Parameter para;
    LoadOptions(argc, argv, para);
    const char *dataset_name = para.dataset_name.c_str();
    const char *dataset_dir = para.dataset_dir.c_str();
    string index_dir = para.index_dir;
    const char *method_name = para.method_name.c_str();
    const std::string index_name = IndexName(method_name);
    spdlog::info("BuildSampleIndexBySample dataset_name {}, method_name {}, dataset_dir {}",
                 dataset_name, method_name, dataset_dir);
    spdlog::info("index_name {}, index_dir {}", index_name, index_dir);

    int n_data_item, n_query_item, n_user, vec_dim;
    vector<VectorMatrix> data = readData(dataset_dir, dataset_name, n_data_item, n_query_item, n_user,
                                         vec_dim);
    VectorMatrix &user = data[0];
    VectorMatrix &data_item = data[1];
    VectorMatrix &query_item = data[2];
    spdlog::info("n_data_item {}, n_query_item {}, n_user {}, vec_dim {}", n_data_item, n_query_item, n_user, vec_dim);

    std::vector<int> memory_capacity_l = {2, 4, 8, 16, 32};
    std::vector<int64_t> n_sample_l(memory_capacity_l.size());
    int n_capacity = (int) memory_capacity_l.size();
    for (int capacityID = 0; capacityID < n_capacity; capacityID++) {
        const int64_t memory_capacity = memory_capacity_l[capacityID];
        const int64_t n_sample = ComputeNSample(index_name, memory_capacity,
                                                n_user, n_data_item, vec_dim);
        n_sample_l[capacityID] = n_sample;
    }

    if (para.n_sample != -1) {
        n_sample_l.clear();
        n_sample_l.push_back(para.n_sample);
        n_capacity = (int) memory_capacity_l.size();
    } else {
        for (int capacityID = 0; capacityID < n_capacity; capacityID++) {
            spdlog::info("memory_capacity {}, n_sample {}",
                         memory_capacity_l[capacityID], n_sample_l[capacityID]);
        }
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

    BuildIndex(data_item, user,
               n_sample_l, para.n_sample_query, para.sample_topk,
               score_table_path, dataset_name, index_name, index_dir.c_str());

    double build_index_time = record.get_elapsed_time_second();
    spdlog::info("finish preprocess and save the index");

    RetrievalResult config;

    spdlog::info("BuildSampleIndexBySample build index time: total {}s", build_index_time);

    char parameter_name[128];
    sprintf(parameter_name, "%s", method_name);
    config.AddInfo(n_sample_info);
    config.AddBuildIndexTime(build_index_time);
    config.WritePerformance(dataset_name, "BuildSampleIndexBySample", parameter_name);
    return 0;
}