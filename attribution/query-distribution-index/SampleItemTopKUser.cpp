//
// Created by BianZheng on 2022/7/27.
//

//对item进行采样, 计算每一个item, 返回reverse k-rank结果所在的userID, 以及返回这个item的topk userID

#include "ComputeItemIDScoreTable.hpp"
#include "FileIO.hpp"
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
             "dataset_name");

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
    spdlog::info("SampleItemTopKUser dataset_name {}, basic_dir {}", dataset_name, basic_dir);

    int n_data_item, n_query_item, n_user, vec_dim;
    std::vector<VectorMatrix> data = readData(basic_dir, dataset_name, n_data_item, n_query_item, n_user,
                                              vec_dim);
    VectorMatrix &user = data[0];
    VectorMatrix &data_item = data[1];
    VectorMatrix &query_item = data[2];
    spdlog::info("n_data_item {}, n_query_item {}, n_user {}, vec_dim {}", n_data_item, n_query_item, n_user,
                 vec_dim);

    user.vectorNormalize();

    const int n_sample_item = 1000;
    const int topk = 100;

    std::vector<int> shuffle_item_idx_l(n_data_item);
    std::iota(shuffle_item_idx_l.begin(), shuffle_item_idx_l.end(), 0);

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(shuffle_item_idx_l.begin(), shuffle_item_idx_l.end(), g);


    //compute the reverse k-rank result of each query
    {
        ComputeItemIDScoreTable cst(user, data_item);
        std::vector<double> distance_l(n_data_item);
        std::vector<double> sample_itemIP_l(n_sample_item);

        std::vector<UserRankElement> result_rank_l(n_sample_item * topk);

        for (int userID = 0; userID < topk; userID++) {
            cst.ComputeItems(userID, distance_l.data());
            for (int sampleID = 0; sampleID < n_sample_item; sampleID++) {
                const int sampleItemID = shuffle_item_idx_l[sampleID];
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
                const long rank = (lb_ptr - distance_ptr) + 1;
                result_rank_l[sampleID * topk + userID] = UserRankElement(userID, (int) rank, itemIP);
            }
        }

        for (int sampleID = 0; sampleID < n_sample_item; sampleID++) {
            std::make_heap(result_rank_l.begin() + sampleID * topk, result_rank_l.begin() + (sampleID + 1) * topk,
                           std::less());
        }

        const int report_every = 10000;
        TimeRecord record;
        record.reset();

        for (int userID = topk; userID < n_user; userID++) {
            cst.ComputeItems(userID, distance_l.data());
            for (int sampleID = 0; sampleID < n_sample_item; sampleID++) {
                const int sampleItemID = shuffle_item_idx_l[sampleID];
                sample_itemIP_l[sampleID] = distance_l[sampleItemID];
            }
            cst.SortItems(userID, distance_l.data());
            for (int sampleID = 0; sampleID < n_sample_item; sampleID++) {
                const double itemIP = sample_itemIP_l[sampleID];
                const double *distance_ptr = distance_l.data();

                const double *lb_ptr = std::lower_bound(distance_ptr, distance_ptr + n_data_item, itemIP,
                                                        [](const double &arrIP, const double &queryIP) {
                                                            return arrIP > queryIP;
                                                        });
                const long rank = (lb_ptr - distance_ptr) + 1;
                const int heap_max_rank = result_rank_l[sampleID * topk].rank_;

                if (heap_max_rank > rank) {
                    std::pop_heap(result_rank_l.begin() + sampleID * topk,
                                  result_rank_l.begin() + (sampleID + 1) * topk,
                                  std::less());
                    result_rank_l[sampleID * topk + topk - 1] = UserRankElement(userID, (int) rank, itemIP);
                    std::push_heap(result_rank_l.begin() + sampleID * topk,
                                   result_rank_l.begin() + (sampleID + 1) * topk,
                                   std::less());
                }
            }

            if (userID % report_every == 0) {
                std::cout << "preprocessed " << userID / (0.01 * n_user) << " %, "
                          << record.get_elapsed_time_second() << " s/iter" << " Mem: "
                          << get_current_RSS() / 1000000 << " Mb \n";
                spdlog::info(
                        "Compute Score Time {}s, Sort Score Time {}s",
                        cst.compute_time_, cst.sort_time_);
                cst.compute_time_ = 0;
                cst.sort_time_ = 0;
                record.reset();
            }
        }

        std::vector<int> user_freq_l(n_user);
        user_freq_l.assign(n_user, 0);
        for (int ID = 0; ID < n_sample_item * topk; ID++) {
            const int userID = result_rank_l[ID].userID_;
            user_freq_l[userID]++;
        }
        WriteFrequency(user_freq_l, n_user, dataset_name, "reverse-k-rank-userID-frequency");

        std::sort(user_freq_l.begin(), user_freq_l.end(), std::less());
        WriteFrequency(user_freq_l, n_user, dataset_name, "reverse-k-rank-sorted-frequency");

        assert(result_rank_l.size() == n_sample_item * topk);

        for (int sampleID = 0; sampleID < n_sample_item; sampleID++) {
            std::sort(result_rank_l.begin() + sampleID * topk,
                      result_rank_l.begin() + (sampleID + 1) * topk,
                      std::less());
        }
        WriteQueryDistribution(result_rank_l, shuffle_item_idx_l,
                               n_sample_item, topk, dataset_name);

    }

    {
        std::vector<DistancePair> item_distance_l(n_user);
        std::vector<int> user_topk_freq_l(n_user);
        user_topk_freq_l.assign(n_user, 0);

        const int report_every = 100;
        TimeRecord record;
        record.reset();

        for (int sampleID = 0; sampleID < n_sample_item; sampleID++) {
            const int itemID = shuffle_item_idx_l[sampleID];
            const double *item_vecs = data_item.getVector(itemID);

#pragma omp parallel for default(none) shared(n_user, user, item_vecs, vec_dim, item_distance_l)
            for (int userID = 0; userID < n_user; userID++) {
                const double *user_vecs = user.getVector(userID);
                const double ip = InnerProduct(item_vecs, user_vecs, vec_dim);
                item_distance_l[userID] = DistancePair(ip, userID);
            }
            boost::sort::block_indirect_sort(item_distance_l.data(), item_distance_l.data() + n_user, std::greater(),
                                             std::thread::hardware_concurrency());

            for (int topID = 0; topID < topk; topID++) {
                const int userID = item_distance_l[topID].ID_;
                user_topk_freq_l[userID]++;
            }

            if (sampleID != 0 && sampleID % report_every == 0) {
                std::cout << "preprocessed " << sampleID / (0.01 * n_sample_item) << " %, "
                          << record.get_elapsed_time_second() << " s/iter" << " Mem: "
                          << get_current_RSS() / 1000000 << " Mb \n";
                record.reset();
            }
        }
        WriteFrequency(user_topk_freq_l, n_user, dataset_name, "topk-userID-frequency");
        std::sort(user_topk_freq_l.begin(), user_topk_freq_l.end(), std::less());
        WriteFrequency(user_topk_freq_l, n_user, dataset_name, "topk-sort-frequency");

    }


    return 0;
}