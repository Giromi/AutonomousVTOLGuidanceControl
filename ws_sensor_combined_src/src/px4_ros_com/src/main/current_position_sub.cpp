#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>


class CurrentPostionSubscriber : public rclcpp::Node {
public: 
    CurrentPostionSubscriber() : Node("offboard_mavros_local_position") {
       auto default_qos = rclcpp::QoS(rclcpp::SystemDefaultsQoS());
        current_pos_sub_ = create_subscription<geometry_msgs::msg::PoseStamped>("/mavros/local_position/pose", default_qos,
        std::bind(&CurrentPostionSubscriber::currentpositionCallback, this, std::placeholders::_1
        ));
    }
private:
    void currentpositionCallback(const geometry_msgs::msg::PoseStamped::SharedPtr msg) {
        current_position_ = {msg->pose.position.x, msg->pose.position.y, msg->pose.position.z};
        std::cout.precision(2);
        std::cout << "현재 위치" << std::endl;
        std::cout << "x : " << current_position_[0] << "\n"
        << "y : " << current_position_[1] << "\n"
        << "z : " << current_position_[2] << std::endl;
    }
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr    current_pos_sub_;

    static std::array<double, 3>		    current_position_;
};

std::array<double, 3>		            CurrentPostionSubscriber::current_position_{};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<CurrentPostionSubscriber>());
    rclcpp::shutdown();
    return 0;
}