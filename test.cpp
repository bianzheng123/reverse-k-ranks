//
// Created by BianZheng on 2022/2/25.
//

#include <iostream>
#include <vector>

using namespace std;

int main(int argc, char **argv) {
    int a = 283228;
    int b = 52889;
    int64_t res = (int64_t) sizeof(double) * a * b;
    std::cout << res << std::endl;

    int64_t res1 = (int64_t) a * b * sizeof(double);
    std::cout << res1 << std::endl;

    cout << sizeof(std::basic_istream<char>::off_type) << endl;
    cout << sizeof(long) << endl;
    return 0;
}