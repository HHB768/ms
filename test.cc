#include <iostream>

union stu {
    int a;
    char b[12];
    double c;
};

int main() {
    std::cout << sizeof(stu) << "\n";    
}