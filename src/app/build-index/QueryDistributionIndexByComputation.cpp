//
// Created by BianZheng on 2022/11/24.
//

//对item进行采样, 计算每一个item, 返回reverse k-rank结果所在的userID, 以及返回这个item的topk userID

/*
 * 变成三个文件
 * 第一个文件存放每一个query的r^q_k的rank, 从小到大存
 * 第二个文件是一维数组, 每一个数代表存储的rank是什么
 * 第三个存放二维数组, 每一个位置代表从1到当前rank总共有多少个user
 */

#include "QueryRankByComputation.hpp"

#include "alg/SpaceInnerProduct.hpp"
#include "struct/VectorMatrix.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "util/VectorIO.hpp"

#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <string>

class Parameter {
public:
    std::string dataset_dir, dataset_name, index_dir;
    int n_sample_item;
    int sample_topk;
};

void LoadOptions(int argc, char **argv, Parameter &para) {
    namespace po = boost::program_options;

    po::options_description opts("Allowed options");
    opts.add_options()
            ("help,h", "help info")
            ("index_dir, ds",
             po::value<std::string>(&para.index_dir)->default_value("/home/bianzheng/reverse-k-ranks/index"),
             "the basic directory of index")
            ("dataset_dir,dd",
             po::value<std::string>(&para.dataset_dir)->default_value("/home/bianzheng/Dataset/ReverseMIPS"),
             "the basic directory of dataset")
            ("dataset_name, ds", po::value<std::string>(&para.dataset_name)->default_value("fake-normal"),
             "dataset_name")

            ("n_sample_item, ns", po::value<int>(&para.n_sample_item)->default_value(150),
             "number of sample of a rank bound")
            ("sample_topk, ns", po::value<int>(&para.sample_topk)->default_value(50),
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
    spdlog::info("QueryDistributionIndexByComputation dataset_name {}, dataset_dir {}", dataset_name, dataset_dir);
    spdlog::info("index_dir {}", index_dir);

    int n_data_item, n_query_item, n_user, vec_dim;
    std::vector<VectorMatrix> data = readData(dataset_dir, dataset_name, n_data_item, n_query_item, n_user,
                                              vec_dim);
    VectorMatrix &user = data[0];
    VectorMatrix &data_item = data[1];
    VectorMatrix &query_item = data[2];
    spdlog::info("n_data_item {}, n_query_item {}, n_user {}, vec_dim {}", n_data_item, n_query_item, n_user,
                 vec_dim);

    user.vectorNormalize();

    const int n_sample_item = para.n_sample_item;
    assert(n_sample_item <= n_data_item);
    const int sample_topk = para.sample_topk;
    spdlog::info("n_sample_item {}, sample_topk {}", n_sample_item, sample_topk);

    char index_path[256];
    sprintf(index_path, "%s/%s.index", index_dir, dataset_name);

    TimeRecord record;
    record.reset();
    double compute_rank_time, store_index_time;
    BuildIndex(user, data_item,
               index_path, dataset_name, index_dir,
               n_sample_item, sample_topk,
               compute_rank_time, store_index_time);
    const double build_index_time = record.get_elapsed_time_second();

    spdlog::info("build_index_time {}s, compute_rank_time {}s, store_index_time {}s",
                 build_index_time, compute_rank_time, store_index_time);

    RetrievalResult config;
    char build_index_info[256];
    sprintf(build_index_info, "compute_rank_time %.3fs, store_index_time %.3fs, build_index_time %.3fs",
            compute_rank_time, store_index_time, build_index_time);
    config.AddInfo(build_index_info);
    config.WritePerformance(dataset_name, "QueryDistributionIndex", "");
    spdlog::info("QueryDistributionIndex finish");

    return 0;
}