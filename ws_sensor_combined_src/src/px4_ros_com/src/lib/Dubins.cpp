
#include "px4_ros_com/Dubins.hpp"

const Dubins::PathType Dubins::_allPathTypes[] = {
    Dubins::PathType::LSL, 
    Dubins::PathType::LSR, 
    Dubins::PathType::RSL, 
    Dubins::PathType::RSR, 
    Dubins::PathType::RLR, 
    Dubins::PathType::LRL 
};

Dubins::Dubins(std::array<double, 3> q0, std::array<double, 3> q1, double turning_radius)
    : q0(q0), q1(q1), rho(turning_radius) { }

Dubins::PathType Dubins::getPathType(void) const { return type; }
double Dubins::getPathLength(void) const {
    double length = param[0] + param[1] + param[2];
    length = length * rho;
    return length;

}

void Dubins::setRho(const double rho) { this->rho = rho; }
void Dubins::setType(const Dubins::PathType type) { this->type = type; }

void Dubins::setQi(const std::array<double, 3>& qi) {
    this->qi[0] = qi[0];
    this->qi[1] = qi[1];
    this->qi[2] = qi[2];
}

void Dubins::setParam(const std::array<double, 3>& param) {
    this->param[0] = param[0];
    this->param[1] = param[1];
    this->param[2] = param[2];
}

int Dubins::word(Dubins::PathType pathType, double param[3]) {
    int result;
    switch(pathType) {
    case Dubins::PathType::LSL:
        result = calLSL(param);
        break;
    case Dubins::PathType::RSL:
        result = calRSL(param);
        break;
    case Dubins::PathType::LSR:
        result = calLSR(param);
        break;
    case Dubins::PathType::RSR:
        result = calRSR(param);
        break;
    case Dubins::PathType::LRL:
        result = calLRL(param);
        break;
    case Dubins::PathType::RLR:
        result = calRLR(param);
        break;
    default:
        result = EDUBNOPATH;
    }
    return result;
}

int Dubins::calLSL(double param[3]) {
    const double p_sq = 2 + In.d_sq - (2*In.c_ab) + (2 * In.d * (In.sa - In.sb));
    if(p_sq >= 0) {
        const double tmp0 = In.d + In.sa - In.sb;
        const double tmp1 = atan2((In.cb - In.ca), tmp0);
        param[0] = _mod2Pi(tmp1 - In.alpha);
        param[1] = sqrt(p_sq);
        param[2] = _mod2Pi(In.beta - tmp1);
        return EDUBOK;
    }
    return EDUBNOPATH;
}

int Dubins::calRSR(double param[3]) {
    const double p_sq = 2 + In.d_sq - (2 * In.c_ab) + (2 * In.d * (In.sb - In.sa));
    if( p_sq >= 0 ) {
        const double tmp0 = In.d - In.sa + In.sb;
        const double tmp1 = atan2((In.ca - In.cb), tmp0);
        param[0] = _mod2Pi(In.alpha - tmp1);
        param[1] = sqrt(p_sq);
        param[2] = _mod2Pi(tmp1 -In.beta);
        return EDUBOK;
    }
    return EDUBNOPATH;
}

int Dubins::calLSR(double param[3]) {
    const double p_sq = -2 + (In.d_sq) + (2 * In.c_ab) + (2 * In.d * (In.sa + In.sb));
    if( p_sq >= 0 ) {
        const double p    = sqrt(p_sq);
        const double tmp0 = atan2( (-In.ca - In.cb), (In.d + In.sa + In.sb) ) - atan2(-2.0, p);
        param[0] = _mod2Pi(tmp0 - In.alpha);
        param[1] = p;
        param[2] = _mod2Pi(tmp0 - _mod2Pi(In.beta));
        return EDUBOK;
    }
    return EDUBNOPATH;
}

int Dubins::calRSL(double param[3]) {
    const double p_sq = -2 + In.d_sq + (2 * In.c_ab) - (2 * In.d * (In.sa + In.sb));
    if( p_sq >= 0 ) {
        double p    = sqrt(p_sq);
        double tmp0 = atan2( (In.ca + In.cb), (In.d - In.sa - In.sb) ) - atan2(2.0, p);
        param[0] = _mod2Pi(In.alpha - tmp0);
        param[1] = p;
        param[2] = _mod2Pi(In.beta - tmp0);
        return EDUBOK;
    }
    return EDUBNOPATH;
}

int Dubins::calRLR(double param[3]) {
    double tmp0 = (6. - In.d_sq + 2*In.c_ab + 2*In.d*(In.sa - In.sb)) / 8.;
    double phi  = atan2( In.ca - In.cb, In.d - In.sa + In.sb );
    if( fabs(tmp0) <= 1) {
        double p = _mod2Pi((2*M_PI) - acos(tmp0) );
        double t = _mod2Pi(In.alpha - phi + _mod2Pi(p/2.));
        param[0] = t;
        param[1] = p;
        param[2] = _mod2Pi(In.alpha - In.beta - t + _mod2Pi(p));
        return EDUBOK;
    }
    return EDUBNOPATH;
}

int Dubins::calLRL(double param[3]) {
    double tmp0 = (6. - In.d_sq + 2*In.c_ab + 2*In.d*(In.sb - In.sa)) / 8.;
    double phi = atan2( In.ca - In.cb, In.d + In.sa - In.sb );
    if( fabs(tmp0) <= 1) {
        double p = _mod2Pi( 2*M_PI - acos( tmp0) );
        double t = _mod2Pi(-In.alpha - phi + p/2.);
        param[0] = t;
        param[1] = p;
        param[2] = _mod2Pi(_mod2Pi(In.beta) - In.alpha - t + _mod2Pi(p));
        return EDUBOK;
    }
    return EDUBNOPATH;
}

int Dubins::shortestPath() {

    double param[3];
    errcode = intermediateResults();
    if(errcode != EDUBOK) {
        return errcode;
    }
    this->qi[0] = q0[0];
    this->qi[1] = q0[1];
    this->qi[2] = q0[2];

    int     best_word = -1;
    double  best_cost = INFINITY;
    for (PathType cur_type : _allPathTypes) {
        errcode = word(cur_type, param);
        if(errcode == EDUBOK) {
            const double cost = param[0] + param[1] + param[2];
            if(cost < best_cost) {
                best_word = static_cast<int>(cur_type);
                best_cost = cost;
                this->param[0] = param[0];
                this->param[1] = param[1];
                this->param[2] = param[2];
                type = cur_type;
            }
        }
    }
    if(best_word == -1) {
        return EDUBNOPATH;
    }
    return EDUBOK;
}

int Dubins::path(double q0[3], double q1[3], double rho, PathType pathType)
{
    static_cast<void>(q1); // unused
    errcode = intermediateResults();
    if(errcode == EDUBOK) {
        double param[3];
        errcode = word(pathType, param);
        if(errcode == EDUBOK) {
            this->param[0] = param[0];
            this->param[1] = param[1];
            this->param[2] = param[2];
            this->qi[0] = q0[0];
            this->qi[1] = q0[1];
            this->qi[2] = q0[2];
            this->rho = rho;
            this->type = pathType;
        }
    }
    return errcode;
}


double Dubins::segmentLength(int i) {
    if((i < 0) || (i > 2)) {
        return INFINITY;
    }
    return param[i] * rho;
}

double Dubins::segmentLengthNormalized(int i) {
    if((i < 0) || (i > 2)) {
        return INFINITY;
    }
    return param[i];
} 

void Dubins::segment(double t, const double qi[3], double qt[3], const SegmentType& type) {
    double st = sin(qi[2]);
    double ct = cos(qi[2]);
    if( type == L_SEG ) {
        qt[0] = +sin(qi[2]+t) - st;
        qt[1] = -cos(qi[2]+t) + ct;
        qt[2] = t;
    } else if( type == R_SEG ) {
        qt[0] = -sin(qi[2]-t) + st;
        qt[1] = +cos(qi[2]-t) - ct;
        qt[2] = -t;
    } else if( type == S_SEG ) {
        qt[0] = ct * t;
        qt[1] = st * t;
        qt[2] = 0.0;
    }
    qt[0] += qi[0];
    qt[1] += qi[1];
    qt[2] += qi[2];
}

int Dubins::pathSample(double t)
{
    /* tprime is the normalised variant of the parameter t */
    double tprime = t / rho;

    if( t < 0 || t > getPathLength() ) {
        return EDUBPARAM;
    }

    /* initial configuration */
    const double qi[] = {0.0, 0.0, qi[2]};

    /* generate the target configuration */
    const double p1 = param[0];
    const double p2 = param[1];

    double q1[3]; /* end-of segment 1 */
    double q2[3]; /* end-of segment 2 */
    const SegmentType* types = DIRDATA[static_cast<int>(type)];
    segment(p1, qi, q1, types[0]);
    segment(p2, q1, q2, types[1]);
    if(tprime < p1) {
        segment( tprime, qi, q, types[0] );
    } else if(tprime < (p1 + p2)) {
        segment( tprime - p1, q1, q,  types[1] );
    } else {
        segment( tprime - p1 - p2, q2, q,  types[2] );
    }
    /* scale the target configuration, translate back to the original starting point */
    q[0] = q[0] * rho + qi[0];
    q[1] = q[1] * rho + qi[1];
    q[2] = _mod2Pi(q[2]);
    return EDUBOK;
}

int Dubins::pathSampleMany(double step_size, DubinsPathSamplingCallback callback_func, void* user_data) {
    int retcode;
    double x = 0.0;
    double length = getPathLength();
    while( x <  length ) {
        pathSample(x);
        retcode = callback_func(q, x, user_data);
        if( retcode != 0 ) {
            return retcode;
        }
        x += step_size;
    }
    return 0;
}


int Dubins::extractSubpath(double t, Dubins& newpath) {
    /* calculate the true parameter */
    double tprime = t / rho;

    if((t < 0) || (t > getPathLength())) {
        return EDUBPARAM; 
    }

    /* copy most of the data */
    newpath.setQi(qi);
    newpath.setRho(rho);
    newpath.setType(type);

    /* fix the parameters */
    const std::array<double, 3> new_param = {fmin(param[0], tprime ),
                                             fmin(param[1], tprime - param[0]),
                                             fmin(param[2], tprime - param[0] - param[1])};
    newpath.setParam(new_param);
    return 0;
}

int Dubins::intermediateResults(void)
{
    if( rho <= 0.0 ) {
        return EDUBBADRHO;
    }
    const double dx = q1[0] - q0[0];
    const double dy = q1[1] - q0[1];
    const double D = sqrt( dx * dx + dy * dy );
    const double d = D / rho;
    /* test required to prevent domain errors if dx=0 and dy=0 */
    const double theta = d > 0 ? _mod2Pi(atan2( dy, dx )) : 0;
    const double alpha = _mod2Pi(q0[2] - theta);
    const double beta  = _mod2Pi(q1[2] - theta);

    In.alpha = alpha;
    In.beta  = beta;
    In.d     = d;
    In.sa    = sin(alpha);
    In.sb    = sin(beta);
    In.ca    = cos(alpha);
    In.cb    = cos(beta);
    In.c_ab  = cos(alpha - beta);
    In.d_sq  = d * d;

    return EDUBOK;
}

int Dubins::pathEndpoint(void) {
    return pathSample(getPathLength() - EPSILON);
}

double Dubins::_fModr(double x, double y) {
    return x - y*floor(x/y);
}
double Dubins::_mod2Pi(double theta) {
    return _fModr( theta, 2 * M_PI );
}
/**
 * Floating point modulus suitable for rings
 *
 * fmod doesn't behave correctly for angular quantities, this function does
 */
