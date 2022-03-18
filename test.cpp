#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

using namespace std;

int main() {
    int *arr = new int[10];
    arr[0] = 1;
    arr[1] = 10;
    arr[2] = -5;
    arr[3] = 3;
    arr[4] = 8;
    arr[5] = -4;
    sort(arr, arr + 6, std::greater());
    printf("%d %d %d %d %d %d\n", arr[0], arr[1], arr[2], arr[3], arr[4], arr[5]);

    int *ptr = lower_bound(arr, arr + 6, 4, [](const double &arrIP, double queryIP) {
        return arrIP > queryIP;
    });
    printf("%d\n", ptr - arr);


}