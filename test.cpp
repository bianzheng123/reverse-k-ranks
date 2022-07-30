#include <iostream>
#include <vector>
#include <numeric>      // std::iota
#include <algorithm>    // std::sort, std::stable_sort

using namespace std;

void sort_indexes(const int* arr, const size_t size, int* sort_idx) {

    // initialize original index locations
    iota(sort_idx, sort_idx + size, 0);

    sort(sort_idx, sort_idx + size,
                [&](int i1, int i2) {return arr[i1] > arr[i2];});
}

int main(){
    std::vector<int> arr = {1, 5, 3, 10, 4, 2};
    sort(arr.begin(), arr.end(), std::less());
    for(const int idx: arr){
        printf("%d ", idx);
    }
    printf("\n");
}