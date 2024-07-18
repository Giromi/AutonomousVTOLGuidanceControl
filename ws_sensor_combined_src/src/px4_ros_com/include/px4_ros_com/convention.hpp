#ifndef CONVENTION_HPP
#define CONVENTION_HPP
# include <cmath>
# include <rclcpp/rclcpp.hpp>
# include <mavlink/v2.0/common/common.hpp>


namespace vtol {
    //숫자로 사용
    enum BodyFrame {EAST, NORTH, UP, YAW};  // enum 의도적 사용
  
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

    // enum State {                     // enum class 타입 안정성 사용
    //     INIT,
    //     READY           =   INIT    | BIT_READY,              // Before arming
    //     MISSION         =   READY   | BIT_MISSION,     // Before takeoff
    //     ARMED           =   READY   | BIT_ARMED,      // After arming
    //     FLY             =   ARMED   | BIT_FLY,    // After takeoff
    //     TAKEOFF         =   FLY     | BIT_TAKEOFF,    // After takeoff
    //     LAND            =   FLY     | BIT_LAND,       // After land
    //     QUAD            =   FLY,
    //     FIXED           =   FLY     | BIT_FIXED,      // Before transition
    //     TO_FIXED        =   QUAD    | BIT_TRANSITION, // Before transition
    //     TO_QUAD         =   FIXED   | BIT_TRANSITION, // Before transition
    //     MC_START        =   QUAD    | BIT_START,     // Before takeoff
    //     FW_START        =   FIXED   | BIT_START,      // Before transition
    // };

    enum CommandBit {   // 토글
        CMD_CHECK           =   0b00000001,
        CMD_ARM_DISARM      =	0b00000010,
        CMD_TAKEOFF_LAND    =	0b00000100,
        CMD_TRANSITION      =	0b00001000,
        CMD_START_STOP      =	0b00010000, 
        CMD_BITS            =   0b11111111,
   };

    enum StatusBit {
        STT_READY           =   0b00000001 << 8,
        STT_ARMED           =	0b00000010 << 8,
        STT_FLY             =	0b00000100 << 8,
        STT_FIXED           =	0b00001000 << 8,
        STT_MISSION         =	0b00010000 << 8,
        STT_BITS            =   0b11111111 << 8,
   };

    enum State { 
        STAND_BY,
        CHECK         =   STAND_BY     |  CMD_CHECK,            // 00000000 00000001
        READY         =   STAND_BY     |  STT_READY,            // 00000001 00000000  

        ARMING        =   READY        |  CMD_ARM_DISARM,       // 00000001 00000010
        ARMED         =   READY        |  STT_ARMED,            // 00000011 00000000

        TAKING_OFF    =   ARMED        |  CMD_TAKEOFF_LAND,     // 00000011 00000100
        MC            =   ARMED        |  STT_FLY,              // 00000111 00000000
        // MC_TAKING_OFF =   MC           |  CMD_TAKEOFF_LAND,     // 00000111 00000100


        MC_TO_FIXED   =   MC           |  CMD_TRANSITION,       // 00000111 00001000
        FW            =   MC           |  STT_FIXED,            // 00001111 00000000

        FW_STARTING   =   FW           |  CMD_START_STOP,       // 00001111 00010000
        FW_MISSION    =   FW           |  STT_MISSION,          // 00011111 00000000

        FW_STOPPING    =   FW_MISSION   |  CMD_START_STOP,       // 00011111 00010000
        FW_TO_QUAD    =   FW           |  CMD_TRANSITION,        // 00001111 00001000

        MC_STARTING   =   MC           |  CMD_START_STOP,       // 00000111 00010000
        MC_MISSION    =   MC           |  STT_MISSION,          // 00010111 00000000

        MC_STOPPING    =   MC_MISSION   |  CMD_START_STOP,       // 00010111 00010000
        LANDING       =   MC           |  CMD_TAKEOFF_LAND,     // 00000111 00000100
        DISARMING     =   ARMED        |  CMD_ARM_DISARM,       // 00000011 00000010
    };


    enum Position {
        INIT_NORTH  = 0,
        INIT_EAST   = 0,
        INIT_UP     = 30,
        INIT_YAW    = 0
    };

    //  enum GlobalPosition {
    //      INIT_ALT    = global_position_[0] + 30, // 35:초기위치, 30:상승고도
    //      INIT_LAT    = global_position_[1],
    //      INIT_LON    = global_position_[2],
    //  };

    // enum Mode {
    //     MC = 3,
    //     FW_= 4
    // };

    struct GeographicCoordinate {
        float altitude;
        float latitude;
        float longitude;
        float min_pitch;
        float yaw;
    };

    constexpr float     NaN             = std::numeric_limits<float>::quiet_NaN();
    constexpr char      FCU_ARM[]       = "AUTO.RTL";
    constexpr char	    FCU_HOLD[]      = "AUTO.LOITER";
    constexpr char	    FCU_OFFBOARD[]  = "OFFBOARD";
    constexpr char	    FCU_TAKEOFF[]   = "AUTO.TAKEOFF";
    constexpr char	    FCU_LAND[]      = "AUTO.LAND";
    constexpr char      FCU_MISSION[]   = "AUTO.MISSION";
    constexpr char      FCU_POSITION[]   = "POSCTL";
    constexpr uint8_t   FCU_MC          = 3;
    constexpr uint8_t   FCU_FW          = 4;
    constexpr double RAD_2_DEG = 180 / M_PI;


    struct ReferenceWaypoint {
        float x;
        float y;
        float z;
        float yaw;

        ReferenceWaypoint(float x, float y, float z, float yaw=0.0f) 
            : x(x), y(y), z(z), yaw(yaw) { }
    };

    namespace mavlink {

        // https://mavlink.io/kr/messages/common.html#MAV_STATE
        enum State {
            UNINIT,
            BOOT,
            CALIBRATING,
            STANDBY,
            ACTIVE,
            CRITICAL,
            EMERGENCY,
            POWEROFF,
            FLIGHT_TERMINATION,
        };


        enum MavCommand {
            CONDITION_YAW   = 115,
            DO_CHANGE_SPEED = 178,
        };

    };
}




#endif
/*
string MODE_PX4_MANUAL      = MANUAL
string MODE_PX4_ACRO        = ACRO
string MODE_PX4_ALTITUDE    = ALTCTL
string MODE_PX4_POSITION    = POSCTL
string MODE_PX4_OFFBOARD    = OFFBOARD
string MODE_PX4_STABILIZED  = STABILIZED
string MODE_PX4_RATTITUDE   = RATTITUDE
string MODE_PX4_MISSION     = AUTO.MISSION
string MODE_PX4_LOITER      = AUTO.LOITER
string MODE_PX4_RTL         = AUTO.RTL
string MODE_PX4_LAND        = AUTO.LAND
string MODE_PX4_RTGS        = AUTO.RTGS
string MODE_PX4_READY       = AUTO.READY
string MODE_PX4_TAKEOFF     = AUTO.TAKEOFF

uint8 VTOL_STATE_UNDEFINED = 0
uint8 VTOL_STATE_TRANSITION_TO_FW = 1
uint8 VTOL_STATE_TRANSITION_TO_MC = 2
uint8 VTOL_STATE_MC = 3
uint8 VTOL_STATE_FW = 4

uint8 LANDED_STATE_UNDEFINED = 0
uint8 LANDED_STATE_ON_GROUND = 1
uint8 LANDED_STATE_IN_AIR = 2
uint8 LANDED_STATE_TAKEOFF = 3
uint8 LANDED_STATE_LANDING = 4

*/
