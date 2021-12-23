#include "src/util/VectorIO.hpp"
#include "src/util/TimeMemory.hpp"
#include "src/util/FileIO.hpp"
#include "src/struct/RankElement.hpp"
#include "src/struct/VectorMatrix.hpp"
#include "src/MemoryIndexBruteForce.hpp"
#include <iostream>
#include <vector>
#include <string>

//预处理时不做任何动作, 在线计算全部的向量, 然后返回最大的k个rank

using namespace std;
using namespace ReverseMIPS;


void writeConfig(const char *dataset_name, const char *method_name, double preprocess_time, double retrieval_time,
                 double ip_calc_time, double binary_search_time) {
    char resPath[256];
    std::sprintf(resPath, "../result/%s-%s-config.txt", dataset_name, method_name);
    std::ofstream file(resPath);
    if (!file) {
        std::printf("error in write result\n");
    }

    file << "preprocess time: " << std::fixed << std::setprecision(5) << preprocess_time << "s" << std::endl;
    file << "retrieval time: " << std::fixed << std::setprecision(5) << retrieval_time << "s" << std::endl;
    file << "inner product calculation time: " << std::fixed << std::setprecision(5) << ip_calc_time << "s"
         << std::endl;
    file << "binary search time: " << std::fixed << std::setprecision(5) << binary_search_time << "s" << std::endl;

    file.close();
}

int main(int argc, char **argv) {
    if (!(argc == 3 or argc == 4)) {
        cout << argv[0] << " dataset_name top-k [basic_dir]" << endl;
        return 0;
    }
    const char *dataset_name = argv[1];
    int topk = atoi(argv[2]);
    const char *basic_dir = "/home/bianzheng/Dataset/MIPS/Reverse-kRanks";
    if (argc == 4) {
        basic_dir = argv[3];
    }
    printf("dataset_name %s, basic_dir %s\n", dataset_name, basic_dir);

    int n_data_item, n_query_item, n_user, vec_dim;
    auto data = readData(basic_dir, dataset_name, n_data_item, n_query_item, n_user, vec_dim);
    auto data_item_ptr = data[0].get();
    auto user_ptr = data[1].get();
    auto query_item_ptr = data[2].get();

    printf("%.3f %.3f\n", data_item_ptr[0], data_item_ptr[1]);

    VectorMatrix data_item, user, query_item;
    data_item.init(data_item_ptr, n_data_item, vec_dim);
    user.init(user_ptr, n_user, vec_dim);
    query_item.init(query_item_ptr, n_query_item, vec_dim);

    TimeRecord record;

    MemoryIndexBruteForce mibf(data_item, user);
    mibf.Preprocess();
    mibf.ResetTime();
    double preprocessed_time = record.get_elapsed_time_micro() * 1e-6;
    record.reset();
    printf("finish preprocess\n");

    vector<vector<RankElement>> result = mibf.Retrieval(query_item, topk);

    double ip_calc_time = mibf.inner_product_calculation_time_;
    double binary_search_time = mibf.binary_search_time_;
    double retrieval_time = record.get_elapsed_time_micro() * 1e-6;

    printf("preprocessed time %.3fs, retrieval time %.3fs\n", preprocessed_time, retrieval_time);
    printf("inner product calculation time %.3fs, binary search time %.3fs\n", ip_calc_time, binary_search_time);
    writeRank(result, dataset_name, "MemoryIndexBruteForce");
    writeConfig(dataset_name, "MemoryIndexBruteForce", preprocessed_time, retrieval_time, ip_calc_time,
                binary_search_time);

    return 0;
}