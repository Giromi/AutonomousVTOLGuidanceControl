#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/msg/string.hpp"

class RobotNewsStationNode : public rclcpp::Node
{
    rclcpp::Publisher<example_interfaces::msg::String>::SharedPtr publisher;
    int num;
    rclcpp::TimerBase::SharedPtr timer;

    void PublishNews()
    {
        auto msg=example_interfaces::msg::String();
        msg.data=std::string("Data : ")+std::to_string(num)+std::string("\n\n---\n\n");
        publisher->publish(msg);
        num+=2;
    }
    public:
    void Publish_start()
    {
        auto msg=example_interfaces::msg::String();
        msg.data=std::string("Let's gogo!!\n\n");
        publisher->publish(msg);
    }
    RobotNewsStationNode() : Node("robot_news_station")
    {
        num=0;
        publisher=this->create_publisher<example_interfaces::msg::String>("Data_sending",10);
        timer=this->create_wall_timer(std::chrono::milliseconds(500),
                                      std::bind(&RobotNewsStationNode::PublishNews,this));
        RCLCPP_INFO(this->get_logger(),"Robot News Station has been started.");
    }
};
 
int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<RobotNewsStationNode>();
    node -> Publish_start();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}

