//
// Created by bianzheng on 2023/3/6.
//

#ifndef REVERSE_KRANKS_GPUCOMPUTESORT_HAVE_BUG_HPP
#define REVERSE_KRANKS_GPUCOMPUTESORT_HAVE_BUG_HPP

#include <cublas_v2.h>
#include <algorithm>
#include <vector>
#include <iostream>

#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/generate.h>
#include <thrust/device_ptr.h>
#include <thrust/sort.h>
#include <thrust/copy.h>
#include <thrust/random.h>

#include "alg/SpaceInnerProduct.hpp"

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

#define CHECK(call)\
{\
  const cudaError_t error=call;\
  if(error!=cudaSuccess)\
  {\
      printf("ERROR: %s:%d,",__FILE__,__LINE__);\
      printf("code:%d,reason:%s\n",error,cudaGetErrorString(error));\
      exit(1);\
  }\
}

    class GPUComputeSort_have_bug {

        uint64_t n_user_, n_data_item_, vec_dim_;
        int batch_n_user_;

        double *user_device_ptr_;
        double *item_device_ptr_;
        int *resID_device_ptr_;
        double *resIP_device_ptr_;

        std::vector<int> resID_host_l_;
        std::vector<double> resIP_host_l_;

        std::vector<int> asyncID_host_l_;
        std::vector<double> asyncIP_host_l_;

        cublasHandle_t handle_;

        int NUM_STREAMS_;
        std::vector<cudaStream_t> streams_;
    public:
        GPUComputeSort_have_bug() = default;

        inline GPUComputeSort_have_bug(const double *user_ptr, const double *data_item_ptr,
                                       const uint64_t n_user, const uint64_t n_data_item, const uint64_t vec_dim,
                                       const int &batch_n_user) {
            n_user_ = n_user;
            n_data_item_ = n_data_item;
            vec_dim_ = vec_dim;
            batch_n_user_ = batch_n_user;

            cudaMalloc((void **) &item_device_ptr_, n_data_item_ * vec_dim_ * sizeof(double));
            cudaMalloc((void **) &user_device_ptr_, n_user_ * vec_dim_ * sizeof(double));
            cudaMalloc((void **) &resID_device_ptr_, batch_n_user_ * n_data_item_ * sizeof(int));
            cudaMalloc((void **) &resIP_device_ptr_, batch_n_user_ * n_data_item_ * sizeof(double));

            cudaCheckErrors("cuda malloc fail");

            resID_host_l_.resize(batch_n_user_ * n_data_item_);
            resIP_host_l_.resize(batch_n_user_ * n_data_item_);

            asyncID_host_l_.resize(n_data_item_);
            asyncIP_host_l_.resize(n_data_item_);

            CHECK(cudaHostRegister(asyncID_host_l_.data(), n_data_item_ * sizeof(int),
                                   cudaHostRegisterPortable));
            CHECK(cudaHostRegister(asyncIP_host_l_.data(), n_data_item_ * sizeof(double),
                                   cudaHostRegisterPortable));


            cublasCheckErrors(
                    cublasSetMatrix(vec_dim, n_data_item_, sizeof(double), (void *) data_item_ptr, vec_dim,
                                    (void *) item_device_ptr_,
                                    vec_dim));
            cublasCheckErrors(
                    cublasSetMatrix(vec_dim_, n_user_, sizeof(double), (void *) user_ptr, vec_dim,
                                    (void *) user_device_ptr_,
                                    vec_dim_));

            NUM_STREAMS_ = batch_n_user_;
            streams_.resize(NUM_STREAMS_);
            // --- Create CUDA streams
            spdlog::info("use GPU stream");
            for (int i = 0; i < NUM_STREAMS_; i++) {
                CHECK(cudaStreamCreate(&streams_[i]));
            }

//            std::vector<double> user_cpu_tran_vecs(n_user * vec_dim);
//            CHECK(cudaMemcpy(user_cpu_tran_vecs.data(), user_device_ptr_, n_user * vec_dim * sizeof(double),
//                             cudaMemcpyDeviceToHost));
//            printf("after memcpy in the user cpu tran vecs\n");
//            for (int i = 0; i < n_user * vec_dim; i++) {
//                assert(abs(user_cpu_tran_vecs[i] - user_ptr[i]) <= 0.01);
//            }

            cublasCheckErrors(cublasCreate(&handle_));
//            cublasCheckErrors(cublasSetVector(vec_dim, sizeof(double), &(vector[0]), 1, vector_gpu, 1));

        }

        void ComputeList(const int &start_userID, const int &n_compute_user) {
            const double *tmp_user_device_ptr = user_device_ptr_ + start_userID * vec_dim_;
            double alpha = 1.0;
            double beta = 0.0;
            cublasCheckErrors(
                    cublasDgemm(handle_, CUBLAS_OP_T, CUBLAS_OP_N,
                                n_data_item_, n_compute_user, vec_dim_,
                                &alpha, item_device_ptr_, vec_dim_, tmp_user_device_ptr, vec_dim_, &beta,
                                resIP_device_ptr_, n_data_item_
                    ));
//            cublasCheckErrors(
//                    cublasDgemm(handle_, CUBLAS_OP_T, CUBLAS_OP_N,
//                                n_compute_user, n_data_item_, vec_dim_,
//                                &alpha, tmp_user_device_ptr, vec_dim_, item_device_ptr_, vec_dim_, &beta,
//                                resIP_device_ptr_, n_data_item_
//                    ));


//            std::vector<double> vecs(n_compute_user * n_data_item_);
//            for (int userID = start_userID; userID < start_userID + n_compute_user; userID++) {
//                const double *tmp_user_host_ptr = tmp_user_ptr_ + userID * vec_dim_;
//                for (int itemID = 0; itemID < n_data_item_; itemID++) {
//                    const double *tmp_item_ptr = tmp_item_ptr_ + itemID * vec_dim_;
//                    vecs[itemID + (userID - start_userID) * n_data_item_] = InnerProduct(tmp_user_host_ptr,
//                                                                                         tmp_item_ptr, (int) vec_dim_);
//                }
//            }
//
//            std::vector<double> gpu_vecs(n_compute_user * n_data_item_);
//            CHECK(cudaMemcpy(gpu_vecs.data(), resIP_device_ptr_, n_compute_user * n_data_item_ * sizeof(double),
//                             cudaMemcpyDeviceToHost));
//            for (int cpt_userID = 0; cpt_userID < n_compute_user; cpt_userID++) {
//                for (int itemID = 0; itemID < n_data_item_; itemID++) {
//                    const int i = cpt_userID * n_data_item_ + itemID;
//                    if (abs(vecs[i] - gpu_vecs[i]) > 0.01) {
//                        printf("i %d, cpt_userID %d, itemID %d, vecs[i] %.3f, gpu_vecs[i] %.3f\n",
//                               i, cpt_userID, itemID, vecs[i], gpu_vecs[i]);
//                    }
//                    assert(abs(vecs[i] - gpu_vecs[i]) <= 0.01);
//                }
//            }


        }

        void ComputeSortListBatch(const int &start_userID, const int &n_compute_user, double *distance_l) {
            TimeRecord record;
            record.reset();
            ComputeList(start_userID, n_compute_user);
            const double compute_time = record.get_elapsed_time_second();

            record.reset();
            assert(NUM_STREAMS_ > n_compute_user);
            for (int comp_userID = 0; comp_userID < n_compute_user; comp_userID++) {
                thrust::device_ptr<double> sort_device_ptr = thrust::device_pointer_cast<double>(
                        resIP_device_ptr_ + comp_userID * n_data_item_);
                thrust::sort(thrust::cuda::par.on(streams_[comp_userID % NUM_STREAMS_]),
                             sort_device_ptr,
                             sort_device_ptr + n_data_item_,
                             thrust::greater<double>());
                CHECK(cudaMemcpyAsync(asyncIP_host_l_.data(),
                                      resIP_device_ptr_ + comp_userID * n_data_item_,
                                      n_data_item_ * sizeof(double),
                                      cudaMemcpyDeviceToHost, streams_[comp_userID % NUM_STREAMS_]));
                CHECK(cudaMemcpyAsync(distance_l + comp_userID * n_data_item_,
                                      asyncIP_host_l_.data(),
                                      n_data_item_ * sizeof(double),
                                      cudaMemcpyHostToHost, streams_[comp_userID % NUM_STREAMS_]));

            }

//            thrust::copy(resIP_device_ptr_, resIP_device_ptr_ + n_compute_user * n_data_item_,
//                         distance_l);

            for (int i = 0; i < NUM_STREAMS_; i++) {
                CHECK(cudaStreamSynchronize(streams_[i]));
            }
            CHECK(cudaDeviceSynchronize());
            const double sort_memcpy_time = record.get_elapsed_time_second();

            memcpy(distance_l, resIP_host_l_.data(), n_compute_user * n_data_item_ * sizeof(double));

            spdlog::info("compute batch time {}s, sort memcpy time {}s", compute_time, sort_memcpy_time);


        }

        void ComputeSortListBatch(const int &start_userID, const int &n_compute_user, DistancePair *distance_l) {
            assert(0 < n_compute_user && n_compute_user <= batch_n_user_);
            ComputeList(start_userID, n_compute_user);

            for (int comp_userID = 0; comp_userID < n_compute_user; comp_userID++) {
//                int *assign_device_ptr = resID_device_ptr_ + comp_userID * n_data_item_;
                thrust::device_ptr<int> assign_device_ptr = thrust::device_pointer_cast<int>(
                        resID_device_ptr_ + comp_userID * n_data_item_);
                thrust::sequence(thrust::device, assign_device_ptr, assign_device_ptr + n_data_item_, 0);
            }

            for (int comp_userID = 0; comp_userID < n_compute_user; comp_userID++) {
                thrust::device_ptr<int> assign_device_ptr = thrust::device_pointer_cast<int>(
                        resID_device_ptr_ + comp_userID * n_data_item_);
                thrust::device_ptr<double> sort_device_ptr = thrust::device_pointer_cast<double>(
                        resIP_device_ptr_ + comp_userID * n_data_item_);
                thrust::sort_by_key(thrust::device, sort_device_ptr, sort_device_ptr + n_data_item_, assign_device_ptr,
                                    thrust::greater<double>());
            }

            CHECK(cudaMemcpy(resIP_host_l_.data(), resIP_device_ptr_, n_compute_user * n_data_item_ * sizeof(double),
                             cudaMemcpyDeviceToHost));
            CHECK(cudaMemcpy(resID_host_l_.data(), resID_device_ptr_, n_compute_user * n_data_item_ * sizeof(int),
                             cudaMemcpyDeviceToHost));
//            thrust::copy(resIP_device_ptr_, resIP_device_ptr_ + n_compute_user * n_data_item_,
//                         resIP_host_l_.data());
//            thrust::copy(resID_device_ptr_, resID_device_ptr_ + n_compute_user * n_data_item_,
//                         resID_host_l_.data());

            for (int comp_userID = 0; comp_userID < n_compute_user; comp_userID++) {
                for (int itemID = 0; itemID < n_data_item_; itemID++) {
                    const uint32_t offset = itemID + comp_userID * n_data_item_;
                    distance_l[offset] = DistancePair(resIP_host_l_[offset], resID_host_l_[offset]);
                }
            }


        }

        void FinishCompute() {
            if (user_device_ptr_ != nullptr) {
                cudaFree(user_device_ptr_);
            }
            if (item_device_ptr_ != nullptr) {
                cudaFree(item_device_ptr_);
            }
            if (resID_device_ptr_ != nullptr) {
                cudaFree(resID_device_ptr_);
            }
            if (resIP_device_ptr_ != nullptr) {
                cudaFree(resIP_device_ptr_);
            }
            for (int i = 0; i < NUM_STREAMS_; i++) {
                CHECK(cudaStreamDestroy(streams_[i]));
            }
            CHECK(cudaHostUnregister(asyncIP_host_l_.data()));
            CHECK(cudaHostUnregister(asyncID_host_l_.data()));
            cublasCheckErrors(cublasDestroy(handle_));
        }
    };

}
#endif //REVERSE_KRANKS_GPUCOMPUTESORT_HAVE_BUG_HPP
