//
// Created by BianZheng on 2022/7/9.
//

#ifndef REVERSE_KRANKS_GPUSCORETABLE_HPP
#define REVERSE_KRANKS_GPUSCORETABLE_HPP

#include <cublas_v2.h>
#include <algorithm>
#include <vector>
#include <iostream>

namespace ReverseMIPS {

// error check macros
#define cudaCheckErrors(msg) \
    do { \
        cudaError_t __err = cudaGetLastError(); \
        if (__err != cudaSuccess) { \
            fprintf(stderr, "Fatal error: %s (%s at %s:%d)\n", \
                msg, cudaGetErrorString(__err), \
                __FILE__, __LINE__); \
            fprintf(stderr, "*** FAILED - ABORTING\n"); \
            exit(1); \
        } \
    } while (0)

// for CUBLAS V2 API
#define cublasCheckErrors(fn) \
    do { \
        cublasStatus_t __err = fn; \
        if (__err != CUBLAS_STATUS_SUCCESS) { \
            fprintf(stderr, "Fatal cublas error: %d (at %s:%d)\n", \
                (int)(__err), \
                __FILE__, __LINE__); \
            fprintf(stderr, "*** FAILED - ABORTING\n"); \
            exit(1); \
        } \
    } while (0)

    class GPUScoreTable {

        int n_user_, n_data_item_, vec_dim_;
        double *user_gpu_ptr_;
        double *data_item_gpu_ptr_;
        double *ip_cache_gpu_ptr_;
        std::vector<double> ip_cache_;
        cublasHandle_t handle_;
    public:
        GPUScoreTable() = default;

        inline GPUScoreTable(const double *user, const double *data_item,
                             const int n_user, const int n_data_item, const int vec_dim) {
            n_user_ = n_user;
            n_data_item_ = n_data_item;
            vec_dim_ = vec_dim;
            ip_cache_.resize(n_data_item_);

            cudaMalloc((void **) &user_gpu_ptr_, n_user_ * vec_dim_ * sizeof(double));
            cudaMalloc((void **) &data_item_gpu_ptr_, n_data_item_ * vec_dim_ * sizeof(double));
            cudaMalloc((void **) &ip_cache_gpu_ptr_, n_data_item_ * sizeof(double));
            cudaCheckErrors("cuda malloc fail");

            cublasCheckErrors(cublasSetVector(vec_dim_ * n_user_, sizeof(double), (void *) user, 1, user_gpu_ptr_, 1));

            cublasCheckErrors(
                    cublasSetMatrix(vec_dim, n_data_item_, sizeof(double), (void *) data_item, vec_dim,
                                    data_item_gpu_ptr_,
                                    vec_dim));
//            cublasCheckErrors(
//                    cublasSetMatrix(vec_dim, n_user, sizeof(double), (void *) user, vec_dim, user_gpu_ptr_,
//                                    vec_dim));

            cublasCheckErrors(cublasCreate(&handle_));
//            cublasCheckErrors(cublasSetVector(vec_dim, sizeof(double), &(vector[0]), 1, vector_gpu, 1));

//            cudaCheckErrors(cudaMemcpy(user_gpu_ptr_, user, n_user_ * vec_dim_ * sizeof(double),
//                                       cudaMemcpyHostToDevice));
//            cudaCheckErrors(cudaMemcpy(data_item_gpu_ptr_, data_item, n_data_item_ * vec_dim_ * sizeof(double),
//                                       cudaMemcpyHostToDevice));
//            cudaCheckErrors(cudaMemset(ip_cache_gpu_ptr_, 0, n_data_item_ * sizeof(double)));

        }

        void ComputeList(const int &userID, double *distance_l) {
            const double *tmp_user_gpu_ptr = user_gpu_ptr_ + userID * vec_dim_;

            double alpha = 1.0;
            double beta = 0.0;
            cublasCheckErrors(
                    cublasDgemv(handle_, CUBLAS_OP_T, vec_dim_, n_data_item_, &alpha, data_item_gpu_ptr_, vec_dim_,
                                tmp_user_gpu_ptr, 1, &beta,
                                ip_cache_gpu_ptr_, 1));

            cublasCheckErrors(cublasGetVector(n_data_item_, sizeof(double), ip_cache_gpu_ptr_, 1, distance_l, 1));

//            cudaCheckErrors(cudaMemcpy(distance_l, ip_cache_gpu_ptr_, n_data_item_ * sizeof(double), cudaMemcpyDeviceToHost));
        }

        void FinishCompute() {
            if (user_gpu_ptr_ != nullptr) {
                cudaFree(user_gpu_ptr_);
            }
            if (data_item_gpu_ptr_ != nullptr) {
                cudaFree(data_item_gpu_ptr_);
            }
            if (ip_cache_gpu_ptr_ != nullptr) {
                cudaFree(ip_cache_gpu_ptr_);
            }
        }
    };

}
#endif //REVERSE_KRANKS_GPUSCORETABLE_HPP
