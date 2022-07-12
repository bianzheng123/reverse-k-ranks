//
// Created by bianzheng on 2022/5/3.
//

#include "src/gpu/GPUScoreTable.hpp"
#include "alg/SpaceInnerProduct.hpp"
#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "struct/VectorMatrix.hpp"
#include <boost/sort/sort.hpp>

#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <vector>
#include <string>

class Parameter {
public:
    std::string basic_dir, dataset_name, method_name;
    int cache_bound_every, n_sample, index_size_gb;
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
            ("method_name, mn", po::value<std::string>(&para.method_name)->default_value("TestMemoryBruteForce"),
             "method_name")

            ("cache_bound_every, cbe", po::value<int>(&para.cache_bound_every)->default_value(512),
             "how many numbers would cache a value")
            ("n_sample, ns", po::value<int>(&para.n_sample)->default_value(20),
             "number of sample of a rank bound")
            ("index_size_gb, tt", po::value<int>(&para.index_size_gb)->default_value(50),
             "index size, in unit of GB");

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
    string method_name = para.method_name;
    spdlog::info("{} dataset_name {}, basic_dir {}", method_name, dataset_name, basic_dir);

    int n_data_item, n_query_item, n_user, vec_dim;
    vector<VectorMatrix> data = readData(basic_dir, dataset_name, n_data_item, n_query_item, n_user,
                                         vec_dim);
    VectorMatrix &user = data[0];
    VectorMatrix &data_item = data[1];
    VectorMatrix &query_item = data[2];
    spdlog::info("n_data_item {}, n_query_item {}, n_user {}, vec_dim {}", n_data_item, n_query_item, n_user, vec_dim);

    //compare which is better
    TimeRecord record;
    record.reset();
    std::vector<double> vecs(n_data_item);

#pragma omp parallel for default(none) shared(n_user, data_item, user, n_data_item, vecs, vec_dim)
    for (int userID = 0; userID < n_user; userID++) {
        const double *user_vecs = user.getVector(userID);
        for (int itemID = 0; itemID < n_data_item; itemID++) {
            double ip = InnerProduct(user_vecs, data_item.getVector(itemID), vec_dim);
            vecs[itemID] = ip;
        }

        std::sort(vecs.begin(), vecs.end(), std::greater());
    }
    const double cpu_time = record.get_elapsed_time_second();

    record.reset();
    GPU::GPUScoreTable gpu(user.getRawData(), data_item.getRawData(), n_user, n_data_item, vec_dim);
    for (int userID = 0; userID < n_user; userID++) {
        gpu.ComputeList(userID, vecs.data());
//        std::sort(vecs.begin(), vecs.end(), std::greater());
        boost::sort::parallel_stable_sort(vecs.begin(), vecs.end(), std::greater());
    }
    const double gpu_time = record.get_elapsed_time_second();

    std::vector<double> parallel_arr(n_data_item);
    std::vector<double> verify_arr(n_data_item);

    for (int userID = 0; userID < n_user; userID++) {
        const double *user_vecs = user.getVector(userID);
        for (int itemID = 0; itemID < n_data_item; itemID++) {
            double ip = InnerProduct(user_vecs, data_item.getVector(itemID), vec_dim);
            parallel_arr[itemID] = ip;
            verify_arr[itemID] = ip;
        }

        boost::sort::parallel_stable_sort(parallel_arr.begin(), parallel_arr.end(), std::greater());
        std::sort(verify_arr.begin(), verify_arr.end(), std::greater());
        for (int itemID = 0; itemID < n_data_item; itemID++) {
            if (parallel_arr[itemID] != verify_arr[itemID]) {
                printf("not equal, program exit\n");
                exit(-1);
            }
        }
    }
    printf("equal, good sorting algorithm\n");

    printf("cpu_time %.3f, gpu_time %.3f\n", cpu_time, gpu_time);

    return 0;
}