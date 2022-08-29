#include <vector>
#include <iostream>
#include <iterator>
#include <algorithm>

using namespace std;

int main() {

    int n = 5, k = 3;

    // vector<vector<int> > combinations;
    vector<int> selected(k);
    vector<int> selector(n);
    std::fill(selector.begin(), selector.begin() + k, 1);
    do {
        int size = 0;
        for (int i = 0; i < n; i++) {
            if (selector[i]) {
                selected[size] = i;
                size++;
            }
        }
        //     combinations.push_back(selected);
//        do_sth(selected);
        copy(selected.begin(), selected.end(), ostream_iterator<int>(cout, " "));
        cout << endl;
    } while (prev_permutation(selector.begin(), selector.end()));

    return 0;
}