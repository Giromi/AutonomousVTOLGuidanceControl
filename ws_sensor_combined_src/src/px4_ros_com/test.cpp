#include <iostream>

class A
{   
public:
    A(void) : x(0), y(0), z(0) {}
    A(int x, int y, int z) : x(x), y(y), z(z) {}



    int x;
    int y;
    int z;
};


struct B
{
    int b;
};


#define X 1
#define Y 2
#define Z 3

int main() {

    // std::array<int, 3> a = {1003, 1004, 1005}; 

    int     a[3] = {1000, 1001, 1002};//  (1000)(1001)(1002)()()
    int*    a_ptr;//                               ()()()()()(1000)()()()

    a_ptr = a;


    std::cout << "a[0] = " << a[0] << std::endl;
    std::cout << "*a = " << *(a + 0) << std::endl;
    std::cout << "a[1] = " << a[1] << std::endl;
    std::cout << "*a = " << *(a + 1) << std::endl;
    std::cout << "a[2] = " << a[2] << std::endl;
    std::cout << "*a = " << *(a + 2) << std::endl;


    std::cout << "*a_ptr = " << *a_ptr << std::endl;
    std::cout << "*a_ptr = " << *a_ptr + 1<< std::endl;
    std::cout << "*a_ptr = " << *(a_ptr + 2)<< std::endl;
    
    return 0;
}

