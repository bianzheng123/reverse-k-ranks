#include "src/util/VectorIO.hpp"
#include "src/util/TimeMemory.hpp"
#include "src/util/FileIO.hpp"
#include "src/struct/RankElement.hpp"
#include "src/struct/VectorMatrix.hpp"
#include "src/OnlineBruteForce.hpp"
#include <iostream>
#include <vector>

//预处理时不做任何动作, 在线计算全部的向量, 然后返回最大的k个rank

using namespace std;
using namespace ReverseMIPS;


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
    vector<float *> data = readData(basic_dir, dataset_name, n_data_item, n_query_item, n_user, vec_dim);
    float *data_item_ptr = data[0];
    float *user_ptr = data[1];
    float *query_item_ptr = data[2];

    VectorMatrix data_item, user, query_item;
    data_item.init(data_item_ptr, n_data_item, vec_dim);
    user.init(user_ptr, n_user, vec_dim);
    query_item.init(query_item_ptr, n_query_item, vec_dim);

    TimeRecord record;

    OnlineBruteForce obf(data_item, user);
    obf.Preprocess();
    float preprocessed_time = record.get_elapsed_time_micro() * 1e-6;
    record.reset();
    printf("finish preprocess\n");

    vector<vector<RankElement>> result = obf.Retrieval(query_item, topk);
    float retrieval_time = record.get_elapsed_time_micro() * 1e-6;

    printf("preprocessed time %.3fs, retrieval time %.3fs\n", preprocessed_time, retrieval_time);
    writeRank(result, dataset_name);

    return 0;
}