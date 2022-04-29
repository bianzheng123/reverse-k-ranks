//
// Created by BianZheng on 2022/2/25.
//

#include <vector>
#include <iostream>
#include <memory>

using namespace std;

struct Base {
    virtual void doWork() {
        // blah blah blah
        cout << "base" << std::endl;
    }

    virtual ~Base() {}
};

struct Derived : public Base {
    virtual void doWork() {
        // blah blah blah 2
        cout << "derived" << std::endl;
    }
};

struct Derived2 : public Base {
    virtual void doWork() {
        // blah blah blah 2
        cout << "derived2" << std::endl;
    }
};

int main(int argc, char **argv) {
    unique_ptr<Base> ptr = make_unique<Base>();
    ptr->doWork();
    ptr = make_unique<Derived>();
    ptr->doWork();
    ptr = make_unique<Derived2>();
    ptr->doWork();
    return 0;
}