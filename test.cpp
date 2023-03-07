
// CPP program to initialize a vector from
// an array.
#include <iostream>
#include <vector>
#include <random>

//#include <cub/cub.cuh>

using namespace std;

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

//
// Block-sorting CUDA kernel
//
//template<int BLOCK_THREADS, int ITEMS_PER_THREAD>
//__global__ void BlockSortKernel(int *d_in, int *d_out) {
//    // Specialize BlockLoad, BlockStore, and BlockRadixSort collective types
//    typedef cub::BlockLoad<
//            int *, BLOCK_THREADS, ITEMS_PER_THREAD, cub::BLOCK_LOAD_TRANSPOSE> BlockLoadT;
//    typedef cub::BlockStore<
//            int *, BLOCK_THREADS, ITEMS_PER_THREAD, cub::BLOCK_STORE_TRANSPOSE> BlockStoreT;
//    typedef cub::BlockRadixSort<
//            int, BLOCK_THREADS, ITEMS_PER_THREAD> BlockRadixSortT;
//    // Allocate type-safe, repurposable shared memory for collectives
//    __shared__ union {
//        typename BlockLoadT::TempStorage load;
//        typename BlockStoreT::TempStorage store;
//        typename BlockRadixSortT::TempStorage sort;
//    } temp_storage;
//    // Obtain this block's segment of consecutive keys (blocked across threads)
//    int thread_keys[ITEMS_PER_THREAD];
//    int block_offset = blockIdx.x * (BLOCK_THREADS * ITEMS_PER_THREAD);
//    BlockLoadT(temp_storage.load).Load(d_in + block_offset, thread_keys);
//
//    __syncthreads();    // Barrier for smem reuse
//    // Collectively sort the keys
//    BlockRadixSortT(temp_storage.sort).Sort(thread_keys);
//    __syncthreads();    // Barrier for smem reuse
//    // Store the sorted segment
//    BlockStoreT(temp_storage.store).Store(d_out + block_offset, thread_keys);
//}

int main() {
    std::default_random_engine generator(0);
    std::uniform_int_distribution<int> distribution(1, 10);
//    int dice_roll = distribution(generator);  // generates number in the range 1..6

    const int size = 16 * 128 * 16;
    std::vector<int> input_l(size);
    for (int i = 0; i < size; i++) {
        input_l[i] = distribution(generator);
    }

    std::vector<int> output_l(size);
    const int num_blocks = 16;

    // Elsewhere in the host program: parameterize and launch a block-sorting
// kernel in which blocks of 128 threads each sort segments of 2048 keys
//    BlockSortKernel<128, 16><<<num_blocks, 128>>>(input_l.data(), output_l.data());

    for (int i = 0; i < size; i++) {
        printf("%d ", output_l[i]);
    }
    printf("\n");


    return 0;
}