//
// Created by BianZheng on 2022/10/20.
//

#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/VectorMatrix.hpp"
#include "alg/RankBoundRefinement/QueryRankSearchSearchKthRank.hpp"

#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <vector>
#include <random>
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
            // memory index parameter
            ("n_sample, ns", po::value<int>(&para.n_sample)->default_value(20),
             "number of sample of a rank bound")
            ("n_sample_query, nsq", po::value<int>(&para.n_sample_query)->default_value(150),
             "the numer of sample query in training query distribution")
            ("sample_topk, st", po::value<int>(&para.sample_topk)->default_value(60),
             "topk in training query distribution");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, opts), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << opts << std::endl;
        exit(0);
    }
}

void SampleUser(const int &n_user, const int64_t &n_sample, std::vector<int> &sample_userID_l) {
    assert(sample_userID_l.size() == n_sample);
    std::vector<int> shuffle_item_idx_l(n_user);
    std::iota(shuffle_item_idx_l.begin(), shuffle_item_idx_l.end(), 0);

//        std::random_device rd;
//        std::mt19937 g(rd());
//        std::shuffle(shuffle_item_idx_l.begin(), shuffle_item_idx_l.end(), g);

    std::random_device rd;
    std::seed_seq seed{rd(), rd(), rd(), rd(), rd(), rd(), rd()};
    std::mt19937 eng(0);
    std::shuffle(shuffle_item_idx_l.begin(), shuffle_item_idx_l.end(), eng);

    for (int sampleID = 0; sampleID < n_sample; sampleID++) {
        sample_userID_l[sampleID] = shuffle_item_idx_l[sampleID];
    }
}

void WriteSampleIP(const double *sample_score_l, const int &n_sample, const int &sampleID, const char *dataset_name) {

    char resPath[256];
    std::sprintf(resPath,
                 "../../result/attribution/SampleScoreDistribution/AdvancedSample-%s-n_sample_%d-sampleID_%d.txt",
                 dataset_name, n_sample, sampleID);
    std::ofstream file(resPath);
    if (!file) {
        spdlog::error("error in write result");
    }

    for (int item_sampleID = 0; item_sampleID < n_sample; item_sampleID++) {
        file << sample_score_l[item_sampleID] << std::endl;
    }
    file.close();
}

using namespace std;
using namespace ReverseMIPS;


int main(int argc, char **argv) {
    Parameter para;
    LoadOptions(argc, argv, para);
    const char *dataset_name = para.dataset_name.c_str();
    const char *dataset_dir = para.dataset_dir.c_str();
    const char *index_dir = para.index_dir.c_str();
    spdlog::info("SampleScoreDistribution dataset_name {}, dataset_dir {}", dataset_name, dataset_dir);
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

    //rank search
    const int n_sample = para.n_sample;
    const int n_sample_query = para.n_sample_query;
    const int sample_topk = para.sample_topk;
    QueryRankSearchSearchKthRank rank_ins(index_dir, dataset_name, n_sample, n_sample_query, sample_topk);

    const int n_sample_user = 50;
    std::vector<int> sample_userID_l(n_sample_user);
    SampleUser(n_user, n_sample_user, sample_userID_l);

    for (int sampleID = 0; sampleID < n_sample_user; sampleID++) {
        const int userID = sample_userID_l[sampleID];
        const double *sample_l = rank_ins.SampleData(userID);
        WriteSampleIP(sample_l, n_sample, sampleID, dataset_name);
    }

    double build_index_time = record.get_elapsed_time_second();
    spdlog::info("finish preprocess and save the index");
    return 0;
}