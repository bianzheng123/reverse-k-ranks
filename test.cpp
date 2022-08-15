#include<iostream>
#include<queue>
#include <algorithm>

using namespace std;

int main() {
    std::vector<int> arr = {1, 2, 2, 3, 4, 5, 5, 5, 7};
    const int num = 2;

    const int *lb_ptr = std::lower_bound(arr.data(), arr.data() + arr.size(), num,
                                         [](const int &arrIP, int queryIP) {
                                             return arrIP < queryIP;
                                         });
    const int offset = lb_ptr - arr.data();

    const int *lb_ptr2 = std::lower_bound(arr.data(), arr.data() + arr.size(), num,
                                         [](const int &arrIP, int queryIP) {
                                             return arrIP <= queryIP;
                                         });
    const int offset2 = lb_ptr2 - arr.data();
    printf("offset %d, offset2 %d\n", offset, offset2);
    return 0;
}