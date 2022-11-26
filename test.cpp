#include <iostream>
#include <iterator>
#include <vector>
#include <cstdlib>

using namespace std;

int main(int argc, char **argv) {
    std::vector<int> v;
    cout << v.max_size() << endl;
    return 0;
}