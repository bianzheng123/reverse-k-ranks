
// CPP program to initialize a vector from
// an array.
#include <iostream>
#include <vector>

using namespace std;

int main() {
    int arr[] = {10, 20, 30};
    int n = sizeof(arr) / sizeof(arr[0]);

    vector<int> vect(arr, arr + n);

    arr[0] = 0;
    cout << "arr: ";
    for (int i = 0; i < n; i++) {
        cout << arr[i] << " ";
    }
    cout << endl;

    cout << "vect: ";
    for (int x: vect)
        cout << x << " ";
    cout << endl;


    return 0;
}