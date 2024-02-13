#include <stdio.h>
#include <iomanip>
#include <iostream>
#include "include/px4_ros_com/Dubins.hpp"

int printConfiguration(double q[3], double x, void* user_data) {
    // x를 소수점 아래 한 자리까지 출력
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "[" << x << "] q: ";

    // q의 각 요소를 기본 정밀도로 출력
    std::cout << std::fixed << std::setprecision(6);
    // std::cout.unsetf(std::ios::fixed); // fixed 속성 해제
    // std::cout.precision(6); // 기본 정밀도로 되돌림 (C++ 표준에서 기본 정밀도는 6)

    // q 배열 출력
    std::cout << q[0] << " " << q[1] << " " << q[2] << std::endl;

    return 0;
}


int main()
{
    const std::array<double, 3> q0 = {0, 0, 0};
    const std::array<double, 3> q1 = {4, 4, 3.142};
    Dubins dubins = Dubins(q0, q1, 1.0);

    dubins.shortest_path();
    dubins.path_sample_many(0.1, printConfiguration, NULL);
    return 0;
}
