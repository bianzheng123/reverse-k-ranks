//
// Created by BianZheng on 2022/8/11.
//

//对item进行采样, 计算每一个item, 返回reverse k-rank结果所在的userID, 以及返回这个item的topk userID

/*
 * 修改, 变成三个文件
 * 第一个文件存放每一个query的r^q_k的rank, 从小到大存
 * 第二个文件是一维数组, 每一个数代表存储的rank是什么
 * 第三个存放二维数组, 每一个位置代表从1到当前rank总共有多少个user
 */

#include "FileIO.hpp"
#include "QueryRank.hpp"

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
            ("dataset_dir,dd",
             po::value<std::string>(&para.dataset_dir)->default_value("/home/bianzheng/Dataset/ReverseMIPS"),
             "the basic directory of dataset")
            ("dataset_name, ds", po::value<std::string>(&para.dataset_name)->default_value("fake-normal"),
             "dataset_name")
            ("index_dir, ds",
             po::value<std::string>(&para.index_dir)->default_value("/home/bianzheng/reverse-k-ranks/index"),
             "the basic directory of index")

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
    spdlog::info("QueryDistributionIndex dataset_name {}, dataset_dir {}", dataset_name, dataset_dir);
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
    std::vector<int> sample_itemID_l(n_sample_item);
    SampleItem(n_data_item, n_sample_item, sample_itemID_l);

    std::vector<int> accu_n_user_rank_l(n_sample_item * (n_data_item + 1));
    ComputeQueryRank(user, data_item,
                     sample_itemID_l, n_sample_item,
                     accu_n_user_rank_l, index_path);

    //sort the rank in ascending sort, should also influence sample_itemID_l
    std::vector<int> sort_kth_rank_l(n_sample_item);
    std::vector<int> sort_sampleID_l(n_sample_item);
    ComputeSortKthRank(accu_n_user_rank_l, n_data_item,
                       n_sample_item, sample_topk,
                       sort_kth_rank_l, sort_sampleID_l);
    const double compute_rank_time = record.get_elapsed_time_second();

//    for (int sampleID = 0; sampleID < n_sample_item; sampleID++) {
//        printf("%d ", sort_kth_rank_l[sampleID]);
//    }
//    printf("\n");
//
//    for (int sampleID = 0; sampleID < n_sample_item; sampleID++) {
//        const int sort_sampleID = sort_sampleID_l[sampleID];
//        for (int i = 0; i < 30; i++) {
//            printf("%5d ", accu_n_user_rank_l[sort_sampleID * (n_data_item + 1) + i]);
//        }
//        printf("\n");
//    }
//
//    {
//        ReadScoreTable rst(n_user, n_data_item, index_path);
//        rst.ReadPreprocess();
//
//        ComputeItemScore cis(user, data_item);
//
//        std::vector<double> itemIP_l(n_data_item);
//        std::vector<int> this_sample_itemID_l = {2160};
//        const int this_n_sample_item = this_sample_itemID_l.size();
//        std::vector<double> sample_item_score_l(this_n_sample_item);
//
//        std::vector<int> rank_count_l(n_data_item + 1);
//        rank_count_l.assign(n_data_item + 1, 0);
//        for (int userID = 0; userID < n_user; userID++) {
//            rst.ReadDisk(userID, itemIP_l);
//            cis.ComputeItems(this_sample_itemID_l.data(), (int) this_n_sample_item, userID, sample_item_score_l.data());
//
//            //compute the rank of each sampled item
//            for (int64_t sampleID = 0; sampleID < this_n_sample_item; sampleID++) {
//                const double sampleIP = sample_item_score_l[sampleID];
//                double *rank_ptr = std::lower_bound(itemIP_l.data(), itemIP_l.data() + n_data_item, sampleIP,
//                                                    [](const double &arrIP, double queryIP) {
//                                                        return arrIP > queryIP;
//                                                    });
//                const int64_t rank = rank_ptr - itemIP_l.data();
//                assert(0 <= rank && rank <= n_data_item);
//                rank_count_l[rank]++;
//            }
//        }
//
//        const int n_rank = n_data_item + 1;
//        for (int rank = 1; rank < n_rank; rank++) {
//            rank_count_l[rank] += rank_count_l[rank - 1];
//        }
//        printf("test rank count\n");
//        for (int i = 0; i < 20; i++) {
//            printf("%5d ", rank_count_l[i]);
//        }
//        printf("\n");
//        assert(rank_count_l[11] == 5 && rank_count_l[10] < 5);
//    }

    record.reset();
    {
        const int n_sample_rank = n_data_item;
        std::vector<int> sample_rankID_l(n_sample_rank);
        std::iota(sample_rankID_l.begin(), sample_rankID_l.end(), 0);
        assert(std::is_sorted(sample_rankID_l.begin(), sample_rankID_l.end()));
        spdlog::info("FullSample n_sample_item {}, n_sample_rank {}", n_sample_item, n_sample_rank);

        WriteDistributionBelowTopk(sample_itemID_l, sort_kth_rank_l,
                                   sort_sampleID_l, accu_n_user_rank_l,
                                   n_data_item, n_sample_item, sample_topk,
                                   dataset_name, index_dir);
    }
    const double store_index_time = record.get_elapsed_time_second();

    spdlog::info("compute_rank_time {}s, store_index_time {}s",
                 compute_rank_time, store_index_time);

    RetrievalResult config;
    char build_index_info[256];
    sprintf(build_index_info, "compute_rank_time %.3f s, store_index_time %.3f s",
            compute_rank_time, store_index_time);
    config.WritePerformance(dataset_name, "QueryDistributionIndex", "");

    return 0;
}