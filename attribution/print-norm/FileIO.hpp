//
// Created by BianZheng on 2022/7/27.
//

#ifndef REVERSE_KRANKS_FILEIO_HPP
#define REVERSE_KRANKS_FILEIO_HPP

#include <spdlog/spdlog.h>
#include <vector>
#include <fstream>
#include <algorithm>
#include <random>
#include <iomanip>

namespace ReverseMIPS {

    void
    WriteNorm(const std::vector<double> &user_norm_l, const int n_user,
              const char *dataset_name) {

        char resPath[256];
        std::sprintf(resPath, "../../result/attribution/PrintNorm/%s.txt", dataset_name);
        std::ofstream file(resPath);
        if (!file) {
            spdlog::error("error in write result");
        }

        assert(user_norm_l.size() == n_user);

        for (int userID = 0; userID < n_user; userID++) {
            file << std::fixed << std::setprecision(5) << user_norm_l[userID] << std::endl;
        }

        file.close();
        spdlog::info("finish write norm");
    }

}
#endif //REVERSE_KRANKS_FILEIO_HPP
