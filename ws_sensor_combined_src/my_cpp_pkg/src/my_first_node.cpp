#include "rclcpp/rclcpp.hpp"

class MyNode: public rclcpp::Node{
    int counter;

    void timerCallback(){
        counter++;
        RCLCPP_INFO(this->get_logger(),"Hello %d",counter);
    }
    rclcpp::TimerBase::SharedPtr timer; //hello

    public:
    MyNode(): Node("cpp_test"),counter(0){
        //RCLCPP_INFO(this->get_logger(),"Hello Cpp Node");
        timer=this->create_wall_timer(std::chrono::seconds(1),
                                      std::bind(&MyNode::timerCallback,this));
    }

};

int main(int argc, char **argv ){
    rclcpp::init(argc,argv);
    auto node =std::make_shared<MyNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}