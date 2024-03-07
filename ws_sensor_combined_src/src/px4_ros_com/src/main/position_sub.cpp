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
        current_pos_sub_ = create_subscription<geometry_msgs::msg::PoseStamped>("/mavros/local_position/pose", default_qos,
        std::bind(&PostionSubscriber::currentpositionCallback, this, std::placeholders::_1
        ));

        set_pos_sub_ = create_subscription<geometry_msgs::msg::PoseStamped>("/mavros/setpoint_position/local", 10,
        std::bind(&PostionSubscriber::setpositionCallback, this, std::placeholders::_1
        ));
    }

    void currentpositionCallback(const geometry_msgs::msg::PoseStamped::SharedPtr msg) {
        current_position_ = {msg->pose.position.x, msg->pose.position.y, msg->pose.position.z};
    }

    void setpositionCallback(const geometry_msgs::msg::PoseStamped::SharedPtr msg) {
        set_position_ = {msg->pose.position.x, msg->pose.position.y, msg->pose.position.z};
        std::cout.precision(2);
        std::cout << "  현재 위치          목표 위치  " << std::endl;
        std::cout << "EAST  : " << current_position_[0] << "  -----> EAST : " << set_position_[0] << "\n"
        << "NORTH : " << current_position_[1] << "  -----> NORTH : " << set_position_[1] << "\n"
        << "UP    : " << current_position_[2] << "   -----> UP : " << set_position_[2] << std::endl;
    }

    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr    current_pos_sub_;
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr    set_pos_sub_;
    
    static std::array<double, 3>		    current_position_;
    static std::array<double, 3>		    set_position_;

};

std::array<double, 3>		            PostionSubscriber::current_position_{};
std::array<double, 3>		            PostionSubscriber::set_position_{};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<PostionSubscriber>());
    rclcpp::shutdown();
    return 0;
}