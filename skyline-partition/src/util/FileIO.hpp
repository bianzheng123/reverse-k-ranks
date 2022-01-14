//
// Created by BianZheng on 2021/12/20.
//

#ifndef FILEIO_HPP
#define FILEIO_HPP

#pragma once

#include <fstream>
#include <iostream>
#include <vector>
#include <ostream>
#include <iomanip>


namespace ReverseMIPS {

    void writeResultSkylineProportion(const char *dataset_name, const char *method_name, int n_eval_dim,
                                      float skyline_proportion,
                                      int n_skyline, int n_data) {
        char resPath[256];
        std::sprintf(resPath, "../result/%s-%s-%d-config.txt", dataset_name, method_name, n_eval_dim);
        std::ofstream file(resPath);
        if (!file) {
            std::printf("error in write result\n");
        }

        file << "skyline_proportion: " << std::fixed << std::setprecision(3) << skyline_proportion << std::endl;
        file << "n_skyline: " << n_skyline << ", n_data: " << n_data << std::endl;
        file << "n_eval_dim: " << n_eval_dim << std::endl;

        file.close();
    }
}

#endif //FILEIO_HPP
