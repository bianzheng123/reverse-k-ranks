#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/VectorMatrix.hpp"
#include "alg/SpaceInnerProduct.hpp"
#include "score_computation/CPUScoreTable.hpp"

#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <Eigen/Dense>

#ifndef EIGEN_USE_MKL_ALL
#define EIGEN_USE_MKL_ALL
#endif

#ifndef EIGEN_VECTORIZE_SSE4_2
#define EIGEN_VECTORIZE_SSE4_2
#endif

using Eigen::MatrixXd;
using Eigen::VectorXd;

void f() {
    std::vector<double> arr = {1, 2, 3, 4, 5, 6};
    MatrixXd user_m = Eigen::Map<Eigen::VectorXd>(arr.data(), 6);
    user_m.resize(2, 3);

    printf("before\n");
    Eigen::VectorXd user_row = user_m.col(0);
    printf("after\n");

    printf("user_m\n");
    std::cout << user_m << std::endl;

    printf("user_row\n");
    std::cout << user_row << std::endl;

    printf("user_row row %ld, col %ld, user_m row %ld, col %ld\n",
           user_row.rows(), user_row.cols(), user_m.rows(), user_m.cols());
    Eigen::VectorXd ip_res = user_m.transpose() * user_row;
    std::cout << ip_res << std::endl;

}

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
            ("method_name, mn", po::value<std::string>(&para.method_name)->default_value("BatchDiskBruteForce"),
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
    f();

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

    //-----------------------------------------------------------------

    ReverseMIPS::CPUScoreTable cpu(user.getRawData(), data_item.getRawData(), n_user, n_data_item, vec_dim);

    std::vector<double> cpu_l(n_data_item);
    std::vector<double> normal_l(n_data_item);
    for (int userID = 0; userID < n_user; userID++) {
        cpu.ComputeList(userID, cpu_l.data());

        const double *user_vecs = user.getVector(userID);
        for (int itemID = 0; itemID < n_data_item; itemID++) {
            double ip = InnerProduct(user_vecs, data_item.getVector(itemID), vec_dim);
            normal_l[itemID] = ip;
        }

        for (int itemID = 0; itemID < n_data_item; itemID++) {
            if (std::abs(cpu_l[itemID] - normal_l[itemID]) > 0.01){
                printf("not equal, have bug\n");
                printf("itemID %d, eigen_l %.3f, normal_l %.3f\n", itemID, cpu_l[itemID], normal_l[itemID]);
                exit(-1);
            }
        }
    }
    printf("great!\n");
    return 0;
}