#include <iostream>
#include <iterator>
#include <vector>
#include <cstdlib>

using namespace std;

struct Combinations {
    typedef vector<int> combination_t;

    // initialize status
    Combinations(int N, int R) :
            completed(N < 1 || R > N),
            generated(0),
            N(N), R(R) {
        for (int c = 0; c < R; ++c)
            curr.push_back(c);
    }

    // true while there are more solutions
    bool completed;

    // count how many generated
    size_t generated;

    // get current and compute next combination
    combination_t next() {
        combination_t ret = curr;

        // find what to increment
        completed = true;
        for (int i = R - 1; i >= 0; --i)
            if (curr[i] < N - R + i) {
                int j = curr[i] + 1;
                while (i <= R - 1)
                    curr[i++] = j++;
                completed = false;
                ++generated;
                break;
            }

        return ret;
    }

private:

    int N, R;
    combination_t curr;
};

int main(int argc, char **argv) {
    int N = argc >= 2 ? atoi(argv[1]) : 10;
    int R = argc >= 3 ? atoi(argv[2]) : 3;
    Combinations cs(N, R);
    while (!cs.completed) {
        Combinations::combination_t c = cs.next();
        copy(c.begin(), c.end(), ostream_iterator<int>(cout, ","));
        cout << endl;
    }
    return cs.generated;
}