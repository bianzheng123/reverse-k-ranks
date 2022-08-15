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
    spdlog::info("QuerySampleRankTopKUser dataset_name {}, basic_dir {}", dataset_name, basic_dir);

    int n_data_item, n_query_item, n_user, vec_dim;
    std::vector<VectorMatrix> data = readData(basic_dir, dataset_name, n_data_item, n_query_item, n_user,
                                              vec_dim);
    VectorMatrix &user = data[0];
    VectorMatrix &data_item = data[1];
    VectorMatrix &query_item = data[2];
    spdlog::info("n_data_item {}, n_query_item {}, n_user {}, vec_dim {}", n_data_item, n_query_item, n_user,
                 vec_dim);

    user.vectorNormalize();

    const int topk = 10;

    std::vector<int> sample_queryID_l(n_query_item);
    std::iota(sample_queryID_l.begin(), sample_queryID_l.end(), 0);

    //compute the k-th top rank of each query
    {
        ComputeItemIDScoreTable cst(user, data_item);
        std::vector<double> distance_l(n_data_item);

        ComputeItemIDScoreTable cst_query(user, query_item);
        std::vector<double> distance_query_l(n_query_item);

        std::vector<priority_queue<int, vector<int>, std::less<int>>> result_rank_l(n_query_item);

        for (int userID = 0; userID < topk; userID++) {
            cst.ComputeItems(userID, distance_l.data());
            cst.SortItems(userID, distance_l.data());

            cst_query.ComputeItems(userID, distance_query_l.data());
            for (int queryID = 0; queryID < n_query_item; queryID++) {
                const double queryIP = distance_query_l[queryID];
                const double *distance_ptr = distance_l.data();

                const double *lb_ptr = std::lower_bound(distance_ptr, distance_ptr + n_data_item, queryIP,
                                                        [](const double &arrIP, double queryIP) {
                                                            return arrIP > queryIP;
                                                        });
                const long rank = lb_ptr - distance_ptr;
                result_rank_l[queryID].push((int) rank);
            }
        }

        const int report_every = 10000;
        TimeRecord record;
        record.reset();

        for (int userID = topk; userID < n_user; userID++) {
            cst.ComputeItems(userID, distance_l.data());
            cst.SortItems(userID, distance_l.data());

            cst_query.ComputeItems(userID, distance_query_l.data());
            for (int queryID = 0; queryID < n_query_item; queryID++) {
                const double queryIP = distance_query_l[queryID];
                const double *distance_ptr = distance_l.data();

                const double *lb_ptr = std::lower_bound(distance_ptr, distance_ptr + n_data_item, queryIP,
                                                        [](const double &arrIP, const double &queryIP) {
                                                            return arrIP > queryIP;
                                                        });
                const long rank = lb_ptr - distance_ptr;
                const int heap_max_rank = result_rank_l[queryID].top();

                if (heap_max_rank > rank) {
                    result_rank_l[queryID].pop();
                    result_rank_l[queryID].push((int) rank);
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

        std::vector<int> topk_rank_l(n_query_item);
        for (int queryID = 0; queryID < n_query_item; queryID++) {
            assert(result_rank_l[queryID].size() == topk);
            topk_rank_l[queryID] = result_rank_l[queryID].top();
        }

        std::vector<int> topk_rank_idx_l(n_query_item);
        std::iota(topk_rank_idx_l.begin(), topk_rank_idx_l.end(), 0);
        std::sort(topk_rank_idx_l.begin(), topk_rank_idx_l.end(),
                  [&topk_rank_l](int i1, int i2) { return topk_rank_l[i1] < topk_rank_l[i2]; });

        std::vector<int> sort_topk_rank_l(n_query_item);
        for (int queryID = 0; queryID < n_query_item; queryID++) {
            sort_topk_rank_l[queryID] = topk_rank_l[topk_rank_idx_l[queryID]];
        }
        assert(std::is_sorted(sort_topk_rank_l.begin(), sort_topk_rank_l.end()));

        std::vector<int> sort_sample_queryID_l(n_query_item);
        for (int queryID = 0; queryID < n_query_item; queryID++) {
            sort_sample_queryID_l[queryID] = sample_queryID_l[topk_rank_idx_l[queryID]];
        }

        WriteQueryDistribution(sort_topk_rank_l, sort_sample_queryID_l,
                               n_query_item, topk, dataset_name);
    }

    return 0;
}