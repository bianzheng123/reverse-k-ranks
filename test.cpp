#include <iostream>
#include <armadillo>
#include <vector>
#include <cfloat>
#include <cstdlib>

int BinarySearch(std::vector<double> arr, double queryIP) {
    auto iter_begin = arr.begin();
    auto iter_end = arr.begin() + 10;

    auto lb_ptr = std::lower_bound(iter_begin, iter_end, queryIP,
                                   [](const double &info, double IP) {
                                       return info > IP;
                                   });
    printf("%.3f %.3f %.3f %.3f\n", *lb_ptr, *iter_begin, *(iter_end - 1), *iter_end);
    return (int) (lb_ptr - iter_begin);
}

//test.cpp
int main(int argc, char **argv) {

    std::vector<double> arr(20);
    for (int i = 0; i < 10; i++) {
        arr[i] = 10 - i;
    }
    for (int i = 10; i < 20; i++) {
        arr[i] = 20;
    }
    BinarySearch(arr, 21);


    return 0;
}