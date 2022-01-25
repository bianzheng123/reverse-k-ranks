#include <algorithm>
#include <iostream>
#include <vector>

class PriceInfo {
public:
    double price;

    inline bool operator==(const PriceInfo &other) const {
        if (this == &other)
            return true;
        return price == other.price;
    };

    inline bool operator!=(const PriceInfo &other) const {
        if (this == &other)
            return false;
        return price != other.price;
    };

    inline bool operator<(const PriceInfo &other) const {
        return price < other.price;
    }

    inline bool operator<=(const PriceInfo &other) const {
        return price <= other.price;
    }

    inline bool operator>(const PriceInfo &other) const {
        return price > other.price;
    }

    inline bool operator>=(const PriceInfo &other) const {
        return price >= other.price;
    }
};

int main() {
    printf("%d\n", 3.001 > 3.0);
}