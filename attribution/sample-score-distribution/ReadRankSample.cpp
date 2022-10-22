//
// Created by BianZheng on 2022/10/22.
//

#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/VectorMatrix.hpp"
#include "alg/RankBoundRefinement/RankSearch.hpp"

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
            // memory index parameter
            ("n_sample, ns", po::value<int>(&para.n_sample)->default_value(20),
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

    //rank search
    char rank_sample_index_path[256];
    sprintf(rank_sample_index_path, "%s/memory_index/RankSample-%s-n_sample_%d.index",
            index_dir, dataset_name, para.n_sample);
    RankSearch rank_ins(rank_sample_index_path);
    printf("sample rank\n");
    for (int sampleID = 0; sampleID < para.n_sample; sampleID++) {
        printf("%d ", rank_ins.known_rank_idx_l_[sampleID]);
    }
    printf("\n");

    double build_index_time = record.get_elapsed_time_second();
    spdlog::info("finish preprocess and save the index");
    return 0;
}