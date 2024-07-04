/*
 * Copyright (c) 2008-2018, Andrew Walker
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef DUBINS_HPP
#define DUBINS_HPP
#include <cmath>
#define EDUBOK        (0)   /* No error */
#define EDUBCOCONFIGS (1)   /* Colocated configurations */
#define EDUBPARAM     (2)   /* Path parameterisitation error */
#define EDUBBADRHO    (3)   /* the rho value is invalid */
#define EDUBNOPATH    (4)   /* no connection between configurations with this word */
#define EPSILON (10e-10)
#include <array>
#include "px4_ros_com/OffboardControl.hpp"

enum SegmentType { L_SEG, S_SEG, R_SEG };

/* The segment types for each of the Path types */
const SegmentType DIRDATA[][3] = {
    { L_SEG, S_SEG, L_SEG },
    { L_SEG, S_SEG, R_SEG },
    { R_SEG, S_SEG, L_SEG },
    { R_SEG, S_SEG, R_SEG },
    { R_SEG, L_SEG, R_SEG },
    { L_SEG, R_SEG, L_SEG }
};

  
class Dubins {
private:
    enum class PathType { LSL, LSR, RSL, RSR, RLR, LRL, };
    typedef int (*DubinsPathSamplingCallback)(double q[3], double t, void* user_data);
    typedef int (*CalculationPath)(void);
public:
    struct IntermediateResults { double alpha, beta, d, sa, sb, ca, cb, c_ab, d_sq; };

    Dubins(std::array<double, 3> q0, std::array<double, 3> q1, double turning_radius);

    Dubins::PathType    getPathType(void) const;
    double              getPathLength(void) const;
    double              getParam(int i) const { return param[i]; }
    double              getRho(void) const { return rho; }
    double              getQiOne(int i) const { return qi[i]; }

    void                setQi(const std::array<double, 3>& qi);
    void                setParam(const std::array<double, 3>& param);
    void                setRho(const double value);
    void                setType(const PathType value);


    /* 지정된 경로 유형에 대한 Dubins 경로를 계산 */
    int    path(double q0[3], double q1[3], double rho, PathType pathType);

    /* 시작지점 q0에서 목표지점 q1까지의 최단 Dubins경로를 계산
     * 이 함수는 가능한 모든 경로 유형을 시도하고 최소 비용(가장 짧은)가진 경로를 반환 */
    int    shortestPath(void);

    /* 각각 Dubins 경로의 특정 세그먼트 길이를 반환 */
    /* 실제 길이*/
    double segmentLength(int i);
    /* 정규화된 길이(회전 반경을 고려하지 않은 길이)를 반환 */
    double segmentLengthNormalized(int i );

    /* 경로의 유형을 반환 */

    /* 경로를 따라 특정 지점을 샘플링 */
    /* 경로 상의 단일 지점을 샘플링 */
    int pathSample(double t);
    /* 경로를 따라 여러 지점을 샘플링 */
    int pathSampleMany(double step_size, DubinsPathSamplingCallback cb, void* user_data);

    /* 경로의 경로의 끝점을 계산합니다. */
    int pathEndpoint(void);


    /* Dubins 경로의 특정 부분을 추출 */
    int extractSubpath(double t, Dubins& new_path);



    // Utility functions

private:
    /* Dubins 경로의 총 길이를 반환합니다. */
    static double                           _mod2Pi(double theta);
    static double                           _fModr(double x, double y);

    int intermediateResults(void);
    void segment(double t, const double qi[3], double qt[3], const SegmentType& type);
    int word(PathType pathType, double param[3]);

    int calLSL(double param[3]);
    int calRSR(double param[3]);
    int calLSR(double param[3]);
    int calRSL(double param[3]);
    int calRLR(double param[3]);
    int calLRL(double param[3]);

    // Essential data
    static const Dubins::PathType           _allPathTypes[];
    int errcode;
    PathType type; /* the path type described */
    IntermediateResults In;

    // Input
    std::array<double, 3> q0;
    std::array<double, 3> q1;
    double rho;              /* model forward velocity / model angular velocity */

    // Calculated
    std::array<double, 3> qi;          /* the initial configuration */
    std::array<double, 3> param;       /* the lengths of the three segments */
    double q[3];       /* the lengths of the three segments */
};

#endif /* DUBINS_HPP */

