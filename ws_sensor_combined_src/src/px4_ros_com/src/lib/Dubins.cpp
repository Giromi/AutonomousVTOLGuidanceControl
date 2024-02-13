
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
    : _q0(q0), _q1(q1), _rho(turning_radius) { }

Dubins::PathType Dubins::get_path_type(void) const { return _type; }
double Dubins::get_path_length(void) const {
    double length = _param[0] + _param[1] + _param[2];
    length = length * _rho;
    return length;

}

void Dubins::set_rho(const double rho) { _rho = rho; }
void Dubins::set_type(const Dubins::PathType type) { _type = type; }

void Dubins::set_qi(const std::array<double, 3>& qi) {
    _qi[0] = qi[0];
    _qi[1] = qi[1];
    _qi[2] = qi[2];
}

void Dubins::set_param(const std::array<double, 3>& param) {
    _param[0] = param[0];
    _param[1] = param[1];
    _param[2] = param[2];
}

int Dubins::_word(Dubins::PathType pathType, double param[3]) {
    int result;
    switch(pathType) {
    case Dubins::PathType::LSL:
        result = _cal_LSL(param);
        break;
    case Dubins::PathType::RSL:
        result = _cal_RSL(param);
        break;
    case Dubins::PathType::LSR:
        result = _cal_LSR(param);
        break;
    case Dubins::PathType::RSR:
        result = _cal_RSR(param);
        break;
    case Dubins::PathType::LRL:
        result = _cal_LRL(param);
        break;
    case Dubins::PathType::RLR:
        result = _cal_RLR(param);
        break;
    default:
        result = EDUBNOPATH;
    }
    return result;
}

int Dubins::_cal_LSL(double param[3]) {
    const double p_sq = 2 + _in.d_sq - (2*_in.c_ab) + (2 * _in.d * (_in.sa - _in.sb));
    if(p_sq >= 0) {
        const double tmp0 = _in.d + _in.sa - _in.sb;
        const double tmp1 = atan2((_in.cb - _in.ca), tmp0);
        param[0] = _mod2pi(tmp1 - _in.alpha);
        param[1] = sqrt(p_sq);
        param[2] = _mod2pi(_in.beta - tmp1);
        return EDUBOK;
    }
    return EDUBNOPATH;
}

int Dubins::_cal_RSR(double param[3]) {
    const double p_sq = 2 + _in.d_sq - (2 * _in.c_ab) + (2 * _in.d * (_in.sb - _in.sa));
    if( p_sq >= 0 ) {
        const double tmp0 = _in.d - _in.sa + _in.sb;
        const double tmp1 = atan2((_in.ca - _in.cb), tmp0);
        param[0] = _mod2pi(_in.alpha - tmp1);
        param[1] = sqrt(p_sq);
        param[2] = _mod2pi(tmp1 -_in.beta);
        return EDUBOK;
    }
    return EDUBNOPATH;
}

int Dubins::_cal_LSR(double param[3]) {
    const double p_sq = -2 + (_in.d_sq) + (2 * _in.c_ab) + (2 * _in.d * (_in.sa + _in.sb));
    if( p_sq >= 0 ) {
        const double p    = sqrt(p_sq);
        const double tmp0 = atan2( (-_in.ca - _in.cb), (_in.d + _in.sa + _in.sb) ) - atan2(-2.0, p);
        param[0] = _mod2pi(tmp0 - _in.alpha);
        param[1] = p;
        param[2] = _mod2pi(tmp0 - _mod2pi(_in.beta));
        return EDUBOK;
    }
    return EDUBNOPATH;
}

int Dubins::_cal_RSL(double param[3]) {
    const double p_sq = -2 + _in.d_sq + (2 * _in.c_ab) - (2 * _in.d * (_in.sa + _in.sb));
    if( p_sq >= 0 ) {
        double p    = sqrt(p_sq);
        double tmp0 = atan2( (_in.ca + _in.cb), (_in.d - _in.sa - _in.sb) ) - atan2(2.0, p);
        param[0] = _mod2pi(_in.alpha - tmp0);
        param[1] = p;
        param[2] = _mod2pi(_in.beta - tmp0);
        return EDUBOK;
    }
    return EDUBNOPATH;
}

int Dubins::_cal_RLR(double param[3]) {
    double tmp0 = (6. - _in.d_sq + 2*_in.c_ab + 2*_in.d*(_in.sa - _in.sb)) / 8.;
    double phi  = atan2( _in.ca - _in.cb, _in.d - _in.sa + _in.sb );
    if( fabs(tmp0) <= 1) {
        double p = _mod2pi((2*M_PI) - acos(tmp0) );
        double t = _mod2pi(_in.alpha - phi + _mod2pi(p/2.));
        param[0] = t;
        param[1] = p;
        param[2] = _mod2pi(_in.alpha - _in.beta - t + _mod2pi(p));
        return EDUBOK;
    }
    return EDUBNOPATH;
}

int Dubins::_cal_LRL(double param[3]) {
    double tmp0 = (6. - _in.d_sq + 2*_in.c_ab + 2*_in.d*(_in.sb - _in.sa)) / 8.;
    double phi = atan2( _in.ca - _in.cb, _in.d + _in.sa - _in.sb );
    if( fabs(tmp0) <= 1) {
        double p = _mod2pi( 2*M_PI - acos( tmp0) );
        double t = _mod2pi(-_in.alpha - phi + p/2.);
        param[0] = t;
        param[1] = p;
        param[2] = _mod2pi(_mod2pi(_in.beta) - _in.alpha - t + _mod2pi(p));
        return EDUBOK;
    }
    return EDUBNOPATH;
}

int Dubins::shortest_path() {

    double param[3];
    _errcode = _intermediate_results();
    if(_errcode != EDUBOK) {
        return _errcode;
    }
    _qi[0] = _q0[0];
    _qi[1] = _q0[1];
    _qi[2] = _q0[2];

    int     best_word = -1;
    double  best_cost = INFINITY;
    for (PathType cur_type : _allPathTypes) {
        _errcode = _word(cur_type, param);
        if(_errcode == EDUBOK) {
            const double cost = param[0] + param[1] + param[2];
            if(cost < best_cost) {
                best_word = static_cast<int>(cur_type);
                best_cost = cost;
                _param[0] = param[0];
                _param[1] = param[1];
                _param[2] = param[2];
                _type = cur_type;
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
    _errcode = _intermediate_results();
    if(_errcode == EDUBOK) {
        double param[3];
        _errcode = _word(pathType, param);
        if(_errcode == EDUBOK) {
            _param[0] = param[0];
            _param[1] = param[1];
            _param[2] = param[2];
            _qi[0] = q0[0];
            _qi[1] = q0[1];
            _qi[2] = q0[2];
            _rho = rho;
            _type = pathType;
        }
    }
    return _errcode;
}


double Dubins::segment_length(int i) {
    if((i < 0) || (i > 2)) {
        return INFINITY;
    }
    return _param[i] * _rho;
}

double Dubins::segment_length_normalized(int i) {
    if((i < 0) || (i > 2)) {
        return INFINITY;
    }
    return _param[i];
} 

void Dubins::_segment(double t, const double qi[3], double qt[3], const SegmentType& type) {
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

int Dubins::path_sample(double t)
{
    /* tprime is the normalised variant of the parameter t */
    double tprime = t / _rho;

    if( t < 0 || t > get_path_length() ) {
        return EDUBPARAM;
    }

    /* initial configuration */
    const double qi[] = {0.0, 0.0, _qi[2]};

    /* generate the target configuration */
    const double p1 = _param[0];
    const double p2 = _param[1];

    double q1[3]; /* end-of segment 1 */
    double q2[3]; /* end-of segment 2 */
    const SegmentType* types = DIRDATA[static_cast<int>(_type)];
    _segment(p1, qi, q1, types[0]);
    _segment(p2, q1, q2, types[1]);
    if(tprime < p1) {
        _segment( tprime, qi, _q, types[0] );
    } else if(tprime < (p1 + p2)) {
        _segment( tprime - p1, q1, _q,  types[1] );
    } else {
        _segment( tprime - p1 - p2, q2, _q,  types[2] );
    }
    /* scale the target configuration, translate back to the original starting point */
    _q[0] = _q[0] * _rho + _qi[0];
    _q[1] = _q[1] * _rho + _qi[1];
    _q[2] = _mod2pi(_q[2]);
    return EDUBOK;
}

int Dubins::path_sample_many(double step_size, DubinsPathSamplingCallback callback_func, void* user_data) {
    int retcode;
    double x = 0.0;
    double length = get_path_length();
    while( x <  length ) {
        path_sample(x);
        retcode = callback_func(_q, x, user_data);
        if( retcode != 0 ) {
            return retcode;
        }
        x += step_size;
    }
    return 0;
}


int Dubins::extract_subpath(double t, Dubins& newpath) {
    /* calculate the true parameter */
    double tprime = t / _rho;

    if((t < 0) || (t > get_path_length())) {
        return EDUBPARAM; 
    }

    /* copy most of the data */
    newpath.set_qi(_qi);
    newpath.set_rho(_rho);
    newpath.set_type(_type);

    /* fix the parameters */
    const std::array<double, 3> new_param = {fmin(_param[0], tprime ),
                                             fmin(_param[1], tprime - _param[0]),
                                             fmin(_param[2], tprime - _param[0] - _param[1])};
    newpath.set_param(new_param);
    return 0;
}

int Dubins::_intermediate_results(void)
{
    if( _rho <= 0.0 ) {
        return EDUBBADRHO;
    }
    const double dx = _q1[0] - _q0[0];
    const double dy = _q1[1] - _q0[1];
    const double D = sqrt( dx * dx + dy * dy );
    const double d = D / _rho;
    /* test required to prevent domain errors if dx=0 and dy=0 */
    const double theta = d > 0 ? _mod2pi(atan2( dy, dx )) : 0;
    const double alpha = _mod2pi(_q0[2] - theta);
    const double beta  = _mod2pi(_q1[2] - theta);

    _in.alpha = alpha;
    _in.beta  = beta;
    _in.d     = d;
    _in.sa    = sin(alpha);
    _in.sb    = sin(beta);
    _in.ca    = cos(alpha);
    _in.cb    = cos(beta);
    _in.c_ab  = cos(alpha - beta);
    _in.d_sq  = d * d;

    return EDUBOK;
}

int Dubins::path_endpoint(void) {
    return path_sample(get_path_length() - EPSILON);
}

double Dubins::_fmodr(double x, double y) {
    return x - y*floor(x/y);
}
double Dubins::_mod2pi(double theta) {
    return _fmodr( theta, 2 * M_PI );
}
/**
 * Floating point modulus suitable for rings
 *
 * fmod doesn't behave correctly for angular quantities, this function does
 */
