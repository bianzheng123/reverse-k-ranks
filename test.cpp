#include <string>
#include <iostream>

using namespace std;

class Base {
public:
    virtual string ToString() { return "base"; };

    virtual string SaySomething() { return ToString() + "base"; }
};

class Impl1 : public Base {
    string tmp;
public:
    inline Impl1(string tmp1) {
        this->tmp = tmp1;
    }

    string ToString() override {
        return tmp;
    }
};

class Impl2 : public Base {
    string tmp, tmp2;
public:
    inline Impl2(string tmp1, string tmp2) {
        this->tmp = tmp1;
        this->tmp2 = tmp2;
    }

    string ToString() override {
        return tmp + tmp2;
    }
};

void test(Base &b) {
    cout << b.SaySomething() << endl;
}

int main() {
    Base a;
    Impl1 b("b");
    Impl2 c("c", "d");
    test(a);
    test(b);
    test(c);
}