#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>

class CurrentPostionSubscriber : public rclcpp::Node {
public: 
    CurrentPostionSubscriber() : Node("offboard_mavros_local_position") {
       auto default_qos = rclcpp::QoS(rclcpp::SystemDefaultsQoS());
        current_pos_sub = create_subscription<geometry_msgs::msg::PoseStamped>("/mavros/local_position/pose", default_qos,
        std::bind(&CurrentPostionSubscriber::currentPositionCallback, this, std::placeholders::_1
        ));
    }
private:
    void currentPositionCallback(const geometry_msgs::msg::PoseStamped::SharedPtr msg) {
        _current_position = {msg->pose.position.x, msg->pose.position.y, msg->pose.position.z};
        std::cout.precision(2);
        std::cout << "현재 위치" << std::endl;
        std::cout << "EAST : " << _current_position[0] << "\n"
        << "NORTH : " << _current_position[1] << "\n"
        << "UP : " << _current_position[2] << std::endl;
    }
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr    current_pos_sub;

    static std::array<double, 3>		    _current_position;
};

std::array<double, 3>		            CurrentPostionSubscriber::_current_position{};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<CurrentPostionSubscriber>());
    rclcpp::shutdown();
    return 0;
}