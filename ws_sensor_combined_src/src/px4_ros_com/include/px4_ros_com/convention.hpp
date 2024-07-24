#ifndef CONVENTION_HPP
#define CONVENTION_HPP
# include <cmath>
# include <rclcpp/rclcpp.hpp>
# include <mavlink/v2.0/common/common.hpp>


namespace vtol {
    //숫자로 사용
    enum BodyFrame {EAST, NORTH, UP, YAW};  // enum 의도적 사용
  
    enum Mav {
        MAV_CMD_CONDITION_YAW   = 115,
    };
  
    enum GlobalFrame {ALT, LAT, LON}; 

    // disarm 2^0, arm 2^1, takeoff 2^2, land 2^3 
    enum Bit {
        BIT_READY       =   0b100000000,
        BIT_ARMED       =	0b000000001,
        BIT_FLY         =	0b000000010,
        BIT_TAKEOFF     =	0b000000100,
        BIT_LAND        =	0b000001000,
        BIT_START       =	0b000010000,
        BIT_FIXED       =	0b000100000,
        BIT_TRANSITION  =	0b001000000,
        BIT_MISSION     =	0b010000000,
   };

    enum Result {
        SUCCESS = 0,
        FAIL = 1
    };

    enum State {                     // enum class 타입 안정성 사용
        INIT,
        READY           =   INIT    | BIT_READY,              // Before arming
        MISSION         =   READY   | BIT_MISSION,     // Before takeoff
        ARMED           =   READY   | BIT_ARMED,      // After arming
        FLY             =   ARMED   | BIT_FLY,    // After takeoff
        TAKEOFF         =   FLY     | BIT_TAKEOFF,    // After takeoff
        LAND            =   FLY     | BIT_LAND,       // After land
        QUAD            =   FLY,
        FIXED           =   FLY     | BIT_FIXED,      // Before transition
        TO_FIXED        =   QUAD    | BIT_TRANSITION, // Before transition
        TO_QUAD         =   FIXED   | BIT_TRANSITION, // Before transition
        MC_START        =   QUAD    | BIT_START,     // Before takeoff
        FW_START        =   FIXED   | BIT_START,      // Before transition
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

    constexpr uint16_t MAV_CMD_DO_CHANGE_SPEED = \
        static_cast<uint16_t>(mavlink::common::MAV_CMD::DO_CHANGE_SPEED);
    constexpr float     NaN             = std::numeric_limits<float>::quiet_NaN();
    constexpr size_t    ACTION_SIZE     = 21;
    constexpr size_t    STATE_SIZE      = 12;
    constexpr char      FCU_ARM[]       = "AUTO.RTL";
    constexpr char	    FCU_HOLD[]      = "AUTO.LOITER";
    constexpr char	    FCU_OFFBOARD[]  = "OFFBOARD";
    constexpr char	    FCU_TAKEOFF[]   = "AUTO.TAKEOFF";
    constexpr char	    FCU_LAND[]      = "AUTO.LAND";
    constexpr char      FCU_MISSION[]   = "AUTO.MISSION";
    constexpr char      FCU_POSITION[]   = "POSCTL";
    constexpr double RAD_2_DEG = 180 / M_PI;


    struct ReferenceWaypoint {
        float x;
        float y;
        float z;
        float yaw;

        ReferenceWaypoint(float x, float y, float z, float yaw=0.0f) 
            : x(x), y(y), z(z), yaw(yaw) { }
    };

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
