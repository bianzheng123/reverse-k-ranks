#pragma once

#include "struct/VectorMatrix.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>

namespace ReverseMIPS {

    template<typename T>
    std::unique_ptr<T[]> loadVector(const char *filename, int &n_data, int &dim) {
        std::ifstream in(filename, std::ios::binary);
        if (!in.is_open()) {
            std::cerr << "Open file error" << std::endl;
            exit(-1);
        }

        in.read((char *) &dim, 4);

        in.seekg(0, std::ios::end);
        std::ios::pos_type ss = in.tellg();
        auto fsize = (size_t) ss;
        n_data = (int) (fsize / (sizeof(T) / 4 * dim + 1) / 4);

        std::unique_ptr<T[]> data = std::make_unique<T[]>((size_t) n_data * (size_t) dim);
        in.seekg(0, std::ios::beg);
        for (int i = 0; i < n_data; i++) {
            in.seekg(4, std::ios::cur);
            in.read((char *) (data.get() + i * dim), dim * sizeof(T));
        }
        in.close();

        return data;
    }

    std::vector<VectorMatrix>
    readData(const char *basic_dir, const char *dataset_name, int &n_data_item, int &n_query_item, int &n_user,
             int &vec_dim) {
        n_data_item = 0;
        n_query_item = 0;
        n_user = 0;
        vec_dim = 0;

        char path[256];
        std::printf("load data item\n");
        std::sprintf(path, "%s/%s/%s_data_item.dvecs", basic_dir, dataset_name, dataset_name);
        std::unique_ptr<double[]> data_item_ptr = loadVector<double>(path, n_data_item, vec_dim);
//        std::printf("%.3f %.3f %.3f %.3f\n", data_item[0], data_item[1], data_item[2], data_item[3]);

        std::printf("load user\n");
        sprintf(path, "%s/%s/%s_user.dvecs", basic_dir, dataset_name, dataset_name);
        std::unique_ptr<double[]> user_ptr = loadVector<double>(path, n_user, vec_dim);
//        std::printf("%.3f %.3f %.3f %.3f\n", user[0], user[1], user[2], user[3]);

        std::printf("load query item\n");
        sprintf(path, "%s/%s/%s_query_item.dvecs", basic_dir, dataset_name, dataset_name);
        std::unique_ptr<double[]> query_item_ptr = loadVector<double>(path, n_query_item, vec_dim);
//        std::printf("%.3f %.3f %.3f %.3f\n", query_item[0], query_item[1], query_item[2], query_item[3]);

        static VectorMatrix user, data_item, query_item;
        user.init(user_ptr, n_user, vec_dim);
        data_item.init(data_item_ptr, n_data_item, vec_dim);
        query_item.init(query_item_ptr, n_query_item, vec_dim);

        std::vector<VectorMatrix> res(3);
        res[0] = std::move(user);
        res[1] = std::move(data_item);
        res[2] = std::move(query_item);
        return res;
    }
}
