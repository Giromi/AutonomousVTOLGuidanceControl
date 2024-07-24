#include <iostream>
class Parent {
public:
    Parent(void) {};

    virtual void print(void) {
        std::cout << "나는야 부모" << std::endl;
    }

};

class Child : public Parent {
public:
    Child(void) {};

    void print(void) {
        std::cout << "나는야 자식" << std::endl;
    }
};

int main(void) {
    Parent  a;

    a.print();


    // upcasting




    Parent c = Child(); // < -요놈이 문제
    c.print();          // 부모

    Child gg = Child();
    Parent& b = gg; //2줄?
    b.print();

    Parent* d = new Child();        // 1줄
    // point 동적할당

    std::cout << "d" << std::endl;
    d->print();


    delete d;               // 메모리 누수 방지 이거 안지움면 컴퓨터 끄기ㄱ전까지 남아있음




    return 0;


    
}