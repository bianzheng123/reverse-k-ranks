#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/generate.h>
#include <thrust/sort.h>
#include <thrust/copy.h>
#include <thrust/random.h>

struct SortByDoubleDescending {
    __host__ __device__
    bool operator()(const thrust::pair<double, int> &o1, const thrust::pair<double, int> &o2) {
        return o1.first > o2.first;
    }
};

// Return a host vector with random values in the range [0,1)
thrust::host_vector<thrust::pair<double, int>> random_vector(const size_t N,
                                                             unsigned int seed = thrust::default_random_engine::default_seed) {
    thrust::default_random_engine rng(seed);
    thrust::uniform_real_distribution<double> u01(0.0f, 100.f);
    thrust::host_vector<thrust::pair<double, int>> temp(N);
    for (size_t i = 0; i < N; i++) {
        temp[i] = thrust::make_pair<double, int>(u01(rng), i);
    }
    return temp;
}

int main() {
    // Generate 32M random numbers serially.
    thrust::default_random_engine rng(1337);
    thrust::uniform_int_distribution<int> dist;

    thrust::host_vector<thrust::pair<double, int>> h_vec = random_vector(32);  // x components of the 'A' vectors

    // Transfer data to the device.
    thrust::device_vector<thrust::pair<double, int>> d_vec = h_vec;

    // Sort data on the device.
    thrust::sort(d_vec.begin(), d_vec.end(), SortByDoubleDescending());

    // Transfer data back to host.
    thrust::copy(d_vec.begin(), d_vec.end(), h_vec.begin());
    for (int i = 0; i < 32; i++) {
        printf("%.3f %d\n", h_vec[i].first, h_vec[i].second);
    }
    printf("\n");
}