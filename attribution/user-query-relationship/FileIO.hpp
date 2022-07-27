//
// Created by BianZheng on 2022/7/27.
//

#ifndef REVERSE_KRANKS_FILEIO_HPP
#define REVERSE_KRANKS_FILEIO_HPP

#include "struct/VectorMatrix.hpp"
#include <vector>
#include <fstream>
#include <algorithm>
#include <random>

namespace ReverseMIPS {
    void
    WriteFrequency(const std::vector<int> &user_freq_l, const int n_user,
              const char *dataset_name, const char *method_name) {

        char resPath[256];
        std::sprintf(resPath, "../../result/attribution/UserQueryRelationship/%s-%s.csv", dataset_name, method_name);
        std::ofstream file(resPath);
        if (!file) {
            std::printf("error in write result\n");
        }

        assert(user_freq_l.size() == n_user);

        for (int userID = 0; userID < n_user; userID++) {
            file << user_freq_l[userID] << std::endl;
        }

        file.close();
    }

}
#endif //REVERSE_KRANKS_FILEIO_HPP
