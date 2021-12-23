#pragma once

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
        n_data = (int) (fsize / (dim + 1) / 4);

        std::unique_ptr<T[]> data = std::make_unique<T[]>((size_t) n_data * (size_t) dim);
        in.seekg(0, std::ios::beg);
        for (int i = 0; i < n_data; i++) {
            in.seekg(4, std::ios::cur);
            in.read((char *) (data.get() + i * dim), dim * sizeof(T));
        }
        in.close();

        return data;
    }

    auto readData(const char *basic_dir, const char *dataset_name, int &n_data_item, int &n_query_item, int &n_user,
                  int &data_dim) {
        n_data_item = 0;
        n_query_item = 0;
        n_user = 0;
        data_dim = 0;

        char path[256];
        std::printf("load data item\n");
        std::sprintf(path, "%s/%s/%s_data_item.fvecs", basic_dir, dataset_name, dataset_name);
        auto data_item = loadVector<float>(path, n_data_item, data_dim);
//        std::printf("%.3f %.3f %.3f %.3f\n", data_item[0], data_item[1], data_item[2], data_item[3]);

        std::printf("load user\n");
        sprintf(path, "%s/%s/%s_user.fvecs", basic_dir, dataset_name, dataset_name);
        auto user = loadVector<float>(path, n_user, data_dim);
//        std::printf("%.3f %.3f %.3f %.3f\n", user[0], user[1], user[2], user[3]);

        std::printf("load query item\n");
        sprintf(path, "%s/%s/%s_query_item.fvecs", basic_dir, dataset_name, dataset_name);
        auto query_item = loadVector<float>(path, n_query_item, data_dim);
//        std::printf("%.3f %.3f %.3f %.3f\n", query_item[0], query_item[1], query_item[2], query_item[3]);

        std::vector<std::unique_ptr<float[]>> res{};
        res.emplace_back(std::move(data_item));
        res.emplace_back(std::move(user));
        res.emplace_back(std::move(query_item));
        return res;
    }
}
