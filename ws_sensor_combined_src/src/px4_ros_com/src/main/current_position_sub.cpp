#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <sensor_msgs/sensor_msgs/msg/nav_sat_fix.hpp>
#include <mavros_msgs/mavros_msgs/msg/gpsraw.hpp>
#include <mavros_msgs/mavros_msgs/msg/gpsrtk.hpp>
class PostionSubscriber : public rclcpp::Node {
public: 
    PostionSubscriber() : Node("offboard_mavros_local_position") {
        auto default_qos = rclcpp::QoS(rclcpp::SystemDefaultsQoS());
        local_pos_sub = create_subscription<geometry_msgs::msg::PoseStamped>("/mavros/local_position/pose", default_qos,
        std::bind(&PostionSubscriber::localPositionCallback, this, std::placeholders::_1
        ));
        global_pos_sub = create_subscription<sensor_msgs::msg::NavSatFix>("/mavros/global_position/global", default_qos, 
        std::bind(&PostionSubscriber::gpsCallBack, this, std::placeholders::_1
        ));
        rtk_pos_sub1 = create_subscription<mavros_msgs::msg::GPSRTK>("/mavros/gpsstatus/gps1/rtk", default_qos, 
        std::bind(&PostionSubscriber::rtk1CallBack, this, std::placeholders::_1
        ));
        rtk_pos_sub2 = create_subscription<mavros_msgs::msg::GPSRTK>("/mavros/gpsstatus/gps2/rtk", default_qos, 
        std::bind(&PostionSubscriber::rtk2CallBack, this, std::placeholders::_1
        ));
        rtk_raw_sub1 = create_subscription<mavros_msgs::msg::GPSRAW>("/mavros/gpsstatus/gps1/raw", default_qos, 
        std::bind(&PostionSubscriber::rtk1RawCallBack, this, std::placeholders::_1
        ));
        rtk_raw_sub2 = create_subscription<mavros_msgs::msg::GPSRAW>("/mavros/gpsstatus/gps2/raw", default_qos, 
        std::bind(&PostionSubscriber::rtk2RawCallBack, this, std::placeholders::_1
        ));
        initializeTimers(50);
    }
    
private:
    void cmdHandler(void) {

        std::cout.precision(2);
        
        std::cout << "Local Position " << std::endl;
        std::cout << "ALTITUDE, LATITUDE, LONGITUDE : " << "(" << local_pos.pose.position.x <<", "<< local_pos.pose.position.y  << ", " << local_pos.pose.position.z  << ")" << std::endl;

        std::cout << "Global Position " << std::endl;
        std::cout << "ALRITUDE, LATITUDE, LONGITUDE : " << "(" << global_pos.altitude <<", "<< global_pos.latitude << ", " << global_pos.longitude << ")" << std::endl;

        std::cout << "GPS1 Raw Position " << std::endl;
        std::cout << "ALRITUDE, LATITUDE, LONGITUDE : " << "(" << rtk_pos_raw.alt <<", "<< rtk_pos_raw.lat << ", " << rtk_pos_raw.lon << ")" << std::endl;
        
        std::cout << "GPS2 Raw Position " << std::endl;
        std::cout << "ALRITUDE, LATITUDE, LONGITUDE : " << "(" << rtk_pos_raw.alt <<", "<< rtk_pos_raw.lat << ", " << rtk_pos_raw.lon << ")" << std::endl;

        std::cout << "GPS1 RTK Position " << std::endl;
        std::cout << "BaseLine1.a, BaseLine1.b, BaseLine1.c : " << "(" << rtk_pos.baseline_a <<", "<< rtk_pos.baseline_b << ", " << rtk_pos.baseline_c << ")" << std::endl;
        
        std::cout << "GPS2 RTK Position " << std::endl;
        std::cout << "BaseLine2.a, BaseLine2.b, BaseLine2.c : " << "(" << rtk_pos.baseline_a <<", "<< rtk_pos.baseline_b << ", " << rtk_pos.baseline_c << ")" << std::endl;

        
        
    }

    void localPositionCallback(const geometry_msgs::msg::PoseStamped::SharedPtr msg) {
        local_pos = *msg;
    }

    void gpsCallBack(const sensor_msgs::msg::NavSatFix::SharedPtr msg) {
        global_pos = *msg;
    }

    void rtk1CallBack(const mavros_msgs::msg::GPSRTK::SharedPtr msg) {
        rtk_pos = *msg;
    }
    void rtk2CallBack(const mavros_msgs::msg::GPSRTK::SharedPtr msg) {
        rtk_pos = *msg;
    }
    void rtk1RawCallBack(const mavros_msgs::msg::GPSRAW::SharedPtr msg) {
        rtk_pos_raw = *msg;
    }
    void rtk2RawCallBack(const mavros_msgs::msg::GPSRAW::SharedPtr msg) {
        rtk_pos_raw = *msg;
    }
   
    void initializeTimers(const int rate_hz) {
    const int rate_ms = 1000 / rate_hz;
    timer = this->create_wall_timer(
            std::chrono::milliseconds(rate_ms),
            std::bind(&PostionSubscriber::cmdHandler, this)); 
    }

    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr    local_pos_sub;
    rclcpp::Subscription<sensor_msgs::msg::NavSatFix>::SharedPtr        global_pos_sub;
    rclcpp::Subscription<mavros_msgs::msg::GPSRAW>::SharedPtr           rtk_raw_sub1;
    rclcpp::Subscription<mavros_msgs::msg::GPSRAW>::SharedPtr           rtk_raw_sub2;
    rclcpp::Subscription<mavros_msgs::msg::GPSRTK>::SharedPtr           rtk_pos_sub1;
    rclcpp::Subscription<mavros_msgs::msg::GPSRTK>::SharedPtr           rtk_pos_sub2;


    geometry_msgs::msg::PoseStamped local_pos;
    sensor_msgs::msg::NavSatFix     global_pos;
    mavros_msgs::msg::GPSRTK        rtk_pos;
    mavros_msgs::msg::GPSRAW        rtk_pos_raw;

    rclcpp::TimerBase::SharedPtr timer;

};
    


int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<PostionSubscriber>());
    rclcpp::shutdown();
    return 0;
}
