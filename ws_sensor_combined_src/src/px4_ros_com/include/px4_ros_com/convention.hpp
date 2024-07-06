#ifndef CONVENTION_HPP
#define CONVENTION_HPP
#include <cmath>


namespace vtol {
    //숫자로 사용
    enum BodyFrame {NORTH, EAST, UP, YAW};  // enum 의도적 사용
  
    enum Mav {
        MAV_CMD_CONDITION_YAW = 115,
    };
  
    enum GlobalFrame {ALT, LAT, LON}; 

    // disarm 2^0, arm 2^1, takeoff 2^2, land 2^3 
    enum Bit {
        BIT_READY       =   0b10000000,
        BIT_ARMED       =	0b00000001,
        BIT_FLY         =	0b00000010,
        BIT_TAKEOFF     =	0b00000100,
        BIT_LAND        =	0b00001000,
        BIT_START       =	0b00010000,
        BIT_FIXED       =	0b00100000,
        BIT_TRANSITION  =	0b01000000,
    };

    enum Result {
        SUCCESS = 0,
        FAIL = 1
    };

    enum State {                     // enum class 타입 안정성 사용
        INIT,
        READY       =   BIT_READY,              // Before arming
        ARMED       =   READY   | BIT_ARMED,      // After arming
        FLY         =   ARMED   | BIT_FLY,    // After takeoff
        TAKEOFF     =   FLY    | BIT_TAKEOFF,    // After takeoff
        LAND        =   FLY    | BIT_LAND,       // After land
        START       =   FLY    | BIT_START,     // Before takeoff
                                                 //
        QUAD        =   FLY,
        FIXED       =   FLY   | BIT_FIXED,      // Before transition
        TO_FIXED    =   QUAD  | BIT_TRANSITION, // Before transition
        TO_QUAD     =   FIXED | BIT_TRANSITION, // Before transition
                                                  //
    };

    enum Position {
        INIT_NORTH  = 0,
        INIT_EAST   = 0,
        INIT_UP     = 20,
        INIT_YAW    = 0
    };

    //  enum GlobalPosition {
    //      INIT_ALT    = global_position_[0] + 30, // 35:초기위치, 30:상승고도
    //      INIT_LAT    = global_position_[1],
    //      INIT_LON    = global_position_[2],
    //  };

    enum Mode {
        MC = 3,
        FW = 4
    };

    struct GeographicCoordinate {
        float altitude;
        float latitude;
        float longitude;
        float min_pitch;
        float yaw;
    };

    constexpr size_t   ACTION_SIZE     = 20;
    constexpr size_t   STATE_SIZE      = 9;
    constexpr char  FCU_ARM[]       = "AUTO.RTL";
    constexpr char	FCU_HOLD[]      = "AUTO.LOITER";
    constexpr char	FCU_OFFBOARD[]  = "OFFBOARD";
    constexpr char	FCU_TAKEOFF[]   = "AUTO.TAKEOFF";
    constexpr char	FCU_LAND[]      = "AUTO.LAND";
    constexpr char  FCU_MISSION[]   = "AUTO.MISSION";
    constexpr double RAD_2_DEG = 180 / M_PI;
}


#endif

/*
namespace flight {
    enum State {
        INIT, ARMED, DISARMED, TAKEOFF, LAND
    };
}

// 사용 예
flight::State status = flight::ARMED;

*/

/*
namespace flight {
    enum class State {
        INIT, ARMED, DISARMED, TAKEOFF, LAND
    };
}

// 사용 예
flight::State status = flight::State::ARMED;
*/
