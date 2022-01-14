//
// Created by BianZheng on 2021/12/20.
// 计算skyline中item的数量和原来的数据集数量的占比
//

#include <iostream>
#include "src/util/VectorIO.hpp"
#include "src/util/FileIO.hpp"

using namespace std;
using namespace ReverseMIPS;

//查看idx_i是否被idx_j dominate
bool isDominated(float *item_l, int idx_i, int idx_j, int n_dim) {
    float *data1 = item_l + n_dim * idx_i;
    float *data2 = item_l + n_dim * idx_j;
    for (int i = 0; i < n_dim; i++) {
        if (data1[i] >= data2[i]) {
            return false;
        }
    }
    return true;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        cout << argv[0] << " dataset_name filename_prefix n_eval_dim" << endl;
        return 0;
    }
    char *dataset_name = argv[1];
    char *filename_prefix = argv[2];
    int n_eval_dim = atoi(argv[3]);
    char item_path[256];
    sprintf(item_path, "../data/%s/%s_%s.fvecs", dataset_name, dataset_name, filename_prefix);
    int n_data, n_dim;
    float *item_l = loadVector<float>(item_path, n_data, n_dim);
    if(n_eval_dim == -1){
        n_eval_dim = n_dim;
    }
    int n_skyline = 0;
    for (int i = 0; i < n_data; i++) {
        bool isDomined = false;
        for (int j = 0; j < n_data; j++) {
            if (i == j) {
                continue;
            }
            if (isDominated(item_l, i, j, n_eval_dim)) {
                isDomined = true;
                break;
            }

        }
        if (!isDomined) {
            n_skyline++;
        }
    }
    printf("n_skyline %d, n_data %d\n", n_skyline, n_data);
    float proportion = n_skyline * 1.0 / n_data;
    printf("%s %s n_eval_dim %d skyline_proportion: %.3f\n", dataset_name, filename_prefix, n_eval_dim, proportion);
    writeResultSkylineProportion(dataset_name, filename_prefix, n_eval_dim, proportion, n_skyline, n_data);
    return 0;
}