#include <iostream>
#include <vector>
#include <numeric>      // std::iota
#include <algorithm>    // std::sort, std::stable_sort

using namespace std;

int main() {
    std::vector<int> arr = {1, 2, 3, 4, 5, 5, 5, 6};

    const int topk = 5;

    const int *lb_ptr = std::lower_bound(arr.data(), arr.data() + arr.size(), topk,
                                         [](const double &arrIP, double queryIP) {
                                             return arrIP < queryIP;
                                         });
    const long itemID = lb_ptr - arr.data();
    printf("%d\n", itemID);
}