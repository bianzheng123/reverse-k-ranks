#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

int main(int argc, char **argv) {

    vector<vector<int>> arr(2, vector<int>());
    arr[0].emplace_back(1);
    arr[1].emplace_back(2);
    vector<int> &tmp = arr[1];
    tmp.emplace_back(12312);
    for (int i = 0; i < arr[1].size(); i++) {
        cout << arr[i][i] << endl;
    }

    return 0;
}