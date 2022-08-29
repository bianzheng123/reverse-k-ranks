//
// Created by BianZheng on 2022/8/11.
//

//对item进行采样, 计算每一个item, 返回reverse k-rank结果所在的userID, 以及返回这个item的topk userID

#include "ComputeItemIDScoreTable.hpp"
#include "FileIO.hpp"
#include "KthUserRank.hpp"
#include "alg/SpaceInnerProduct.hpp"
#include "struct/VectorMatrix.hpp"
#include "struct/UserRankElement.hpp"
#include "util/TimeMemory.hpp"
#include "util/VectorIO.hpp"

#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <random>

class Parameter {
public:
    std::string basic_dir, dataset_name;
    int n_sample_item;
    int sample_topk;
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

            ("n_sample_item, ns", po::value<int>(&para.n_sample_item)->default_value(500),
             "number of sample of a rank bound")
            ("sample_topk, ns", po::value<int>(&para.sample_topk)->default_value(10),
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
    const char *basic_dir = para.basic_dir.c_str();
    spdlog::info("DistributionBelowTopk dataset_name {}, basic_dir {}", dataset_name, basic_dir);

    int n_data_item, n_query_item, n_user, vec_dim;
    std::vector<VectorMatrix> data = readData(basic_dir, dataset_name, n_data_item, n_query_item, n_user,
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

    std::vector<int> sample_itemID_l(n_sample_item);
    SampleItem(n_data_item, n_sample_item, sample_itemID_l);

    //sort the rank in ascending sort, should also influence sample_itemID_l
    std::vector<int> sort_kth_rank_l(n_sample_item);
    std::vector<int> sort_sample_itemID_l(n_sample_item);
    ComputeKthRank(user, data_item,
                   n_sample_item, sample_topk,
                   sample_itemID_l,
                   sort_kth_rank_l, sort_sample_itemID_l);

    // every cell stores how many user falls in this rank
    std::vector<int> sample_rank_l(n_sample_item * n_sample_item);
    sample_rank_l.assign(n_sample_item * n_sample_item, 0);

    //compute the reverse k-rank result of each query
    {
        ComputeItemIDScoreTable cst(user, data_item);
        std::vector<double> distance_l(n_data_item);
        std::vector<double> sample_itemIP_l(n_sample_item);

        TimeRecord record;
        record.reset();
        for (int userID = 0; userID < n_user; userID++) {
            cst.ComputeItems(userID, distance_l.data());
            for (int sampleID = 0; sampleID < n_sample_item; sampleID++) {
                const int sampleItemID = sort_sample_itemID_l[sampleID];
                sample_itemIP_l[sampleID] = distance_l[sampleItemID];
            }
            cst.SortItems(userID, distance_l.data());
            for (int sampleID = 0; sampleID < n_sample_item; sampleID++) {
                const double itemIP = sample_itemIP_l[sampleID];
                const double *distance_ptr = distance_l.data();

                const double *lb_ptr = std::lower_bound(distance_ptr, distance_ptr + n_data_item, itemIP,
                                                        [](const double &arrIP, double queryIP) {
                                                            return arrIP > queryIP;
                                                        });
                const long rank = (lb_ptr - distance_ptr);
                if (rank < sort_kth_rank_l[sampleID]) {
                    continue;
                }

                const int *topk_rank_offset_ptr = std::lower_bound(sort_kth_rank_l.data(),
                                                                   sort_kth_rank_l.data() + n_sample_item,
                                                                   (int) rank,
                                                                   [](const int &arrIP, int queryIP) {
                                                                       return arrIP <= queryIP;
                                                                   });
                long topk_rank_offset = topk_rank_offset_ptr - sort_kth_rank_l.data();
                assert(0 < topk_rank_offset && topk_rank_offset <= n_sample_item);
                if (topk_rank_offset == n_sample_item && sort_kth_rank_l[topk_rank_offset - 1] < rank) {
                    continue;
                } else {
                    topk_rank_offset--;
                }
                assert(0 <= rank && rank <= n_data_item);

                sample_rank_l[sampleID * n_sample_item + topk_rank_offset]++;
            }

            if (userID % cst.report_every_ == 0 && userID != 0) {
                spdlog::info(
                        "Compute second score table {:.2f}%, {:.2f} s/iter, Mem: {} Mb, Compute Score Time {}s, Sort Score Time {}s",
                        userID / (0.01 * n_user), record.get_elapsed_time_second(), get_current_RSS() / 1000000,
                        cst.compute_time_, cst.sort_time_);
                cst.compute_time_ = 0;
                cst.sort_time_ = 0;
                record.reset();
            }
        }

        for (int sampleID = 0; sampleID < n_sample_item; sampleID++) {
            for (int sample_rank = 1; sample_rank < n_sample_item; sample_rank++) {
                sample_rank_l[sampleID * n_sample_item + sample_rank] +=
                        sample_rank_l[sampleID * n_sample_item + sample_rank - 1];
            }
        }

        for (int sampleID = 0; sampleID < n_sample_item; sampleID++) {
            assert(0 <= sample_rank_l[sampleID * n_sample_item] && sample_rank_l[sampleID * n_sample_item] <= n_user);
            for (int sample_rank = 1; sample_rank < n_sample_item; sample_rank++) {
                assert(sample_rank_l[sampleID * n_sample_item + sample_rank - 1] <=
                       sample_rank_l[sampleID * n_sample_item + sample_rank] &&
                       sample_rank_l[sampleID * n_sample_item + sample_rank] <= n_user);
            }
        }

        WriteDistributionBelowTopk(sample_rank_l, sort_kth_rank_l, sort_sample_itemID_l,
                                   n_sample_item, sample_topk, dataset_name);

    }


    return 0;
}