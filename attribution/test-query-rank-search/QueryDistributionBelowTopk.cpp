//
// Created by BianZheng on 2022/8/11.
//

//对item进行采样, 计算每一个item, 返回reverse k-rank结果所在的userID, 以及返回这个item的topk userID

#include "ComputeItemIDScoreTable.hpp"
#include "FileIO.hpp"
#include "alg/SpaceInnerProduct.hpp"
#include "struct/VectorMatrix.hpp"
#include "util/TimeMemory.hpp"
#include "util/VectorIO.hpp"

#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <string>

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
    spdlog::info("QueryDistributionBelowTopk dataset_name {}, basic_dir {}", dataset_name, basic_dir);

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
    std::vector<int> topk_rank_l(n_query_item);
    ReadKthRank(n_query_item, topk, dataset_name, topk_rank_l);

    std::vector<int> sorted_topk_rank_l(n_query_item);
    sorted_topk_rank_l.assign(topk_rank_l.begin(), topk_rank_l.end());
    assert(std::is_sorted(sorted_topk_rank_l.begin(), sorted_topk_rank_l.end()));

    // every cell stores how many user falls in this rank
    std::vector<int> sample_rank_l(n_query_item * n_query_item);
    sample_rank_l.assign(n_query_item * n_query_item, 0);

    //compute the reverse k-rank result of each query
    {
        ComputeItemIDScoreTable cst(user, data_item);
        std::vector<double> distance_l(n_data_item);

        ComputeItemIDScoreTable cst_query(user, query_item);
        std::vector<double> distance_query_l(n_query_item);

        const int report_every = 10000;
        TimeRecord record;
        record.reset();
        for (int userID = 0; userID < n_user; userID++) {
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
                if (rank < topk_rank_l[queryID]) {
                    continue;
                }

                const int *topk_rank_offset_ptr = std::lower_bound(sorted_topk_rank_l.data(),
                                                                   sorted_topk_rank_l.data() + n_query_item,
                                                                   (int) rank,
                                                                   [](const int &arrIP, int queryIP) {
                                                                       return arrIP < queryIP;
                                                                   });
                const int topk_rank_offset = topk_rank_offset_ptr - sorted_topk_rank_l.data();
                assert(0 <= topk_rank_offset && topk_rank_offset <= n_query_item);
                if (topk_rank_offset == n_query_item) {
                    continue;
                }
                assert(0 <= rank && rank <= n_data_item);

                sample_rank_l[queryID * n_query_item + topk_rank_offset]++;
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

        for (int queryID = 0; queryID < n_query_item; queryID++) {
            for (int sample_rank = 1; sample_rank < n_query_item; sample_rank++) {
                sample_rank_l[queryID * n_query_item + sample_rank] +=
                        sample_rank_l[queryID * n_query_item + sample_rank - 1];
            }
        }

        for (int queryID = 0; queryID < n_query_item; queryID++) {
            assert(0 <= sample_rank_l[queryID * n_query_item] && sample_rank_l[queryID * n_query_item] <= n_user);
            for (int sample_rank = 1; sample_rank < n_query_item; sample_rank++) {
                assert(sample_rank_l[queryID * n_query_item + sample_rank - 1] <=
                       sample_rank_l[queryID * n_query_item + sample_rank] &&
                       sample_rank_l[queryID * n_query_item + sample_rank] <= n_user);
            }
        }

        WriteDistributionBelowTopk(sample_rank_l,
                                   n_query_item, topk, dataset_name);

    }


    return 0;
}