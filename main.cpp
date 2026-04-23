
#include <iostream>
#include "src.hpp"

using namespace sjtu;

int main() {
    map<int, int> mp;
    
    int n;
    std::cin >> n;
    
    for (int i = 0; i < n; i++) {
        int op, x, y;
        std::cin >> op;
        
        if (op == 1) {
            std::cin >> x >> y;
            mp[x] = y;
        } else if (op == 2) {
            std::cin >> x;
            auto it = mp.find(x);
            if (it != mp.end()) {
                std::cout << it->second << "\n";
            } else {
                std::cout << "0\n";
            }
        } else if (op == 3) {
            std::cin >> x;
            mp.erase(x);
        } else if (op == 4) {
            std::cout << mp.size() << "\n";
        }
    }
    
    return 0;
}
