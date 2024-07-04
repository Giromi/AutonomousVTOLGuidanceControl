#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>

class PostionSubscriber : public rclcpp::Node {
public: 
    PostionSubscriber() : Node("position_sub") {
       positionSubscribers();
    }

private:
    void positionSubscribers(void){
        auto default_qos = rclcpp::QoS(rclcpp::SystemDefaultsQoS());
        current_pos_sub = create_subscription<geometry_msgs::msg::PoseStamped>("/mavros/local_position/pose", default_qos,
        std::bind(&PostionSubscriber::currentPositionCallback, this, std::placeholders::_1
        ));

        set_pos_sub = create_subscription<geometry_msgs::msg::PoseStamped>("/mavros/setpoint_position/local", 10,
        std::bind(&PostionSubscriber::setPositionCallback, this, std::placeholders::_1
        ));
    }

    void currentPositionCallback(const geometry_msgs::msg::PoseStamped::SharedPtr msg) {
        _current_position = {msg->pose.position.x, msg->pose.position.y, msg->pose.position.z};
    }

    void setPositionCallback(const geometry_msgs::msg::PoseStamped::SharedPtr msg) {
        _set_position = {msg->pose.position.x, msg->pose.position.y, msg->pose.position.z};
        std::cout.precision(2);
        std::cout << "  현재 위치          목표 위치  " << std::endl;
        std::cout << "EAST  : " << _current_position[0] << "  -----> EAST : " << _set_position[0] << "\n"
        << "NORTH : " << _current_position[1] << "  -----> NORTH : " << _set_position[1] << "\n"
        << "UP    : " << _current_position[2] << "   -----> UP : " << _set_position[2] << std::endl;
    }

    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr    current_pos_sub;
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr    set_pos_sub;
    
    static std::array<double, 3>		    _current_position;
    static std::array<double, 3>		    _set_position;

};

std::array<double, 3>		            PostionSubscriber::_current_position{};
std::array<double, 3>		            PostionSubscriber::_set_position{};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<PostionSubscriber>());
    rclcpp::shutdown();
    return 0;
}