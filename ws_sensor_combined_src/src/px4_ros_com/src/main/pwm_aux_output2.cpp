#include <rclcpp/rclcpp.hpp>
#include <mavros_msgs/srv/param_set.hpp>

class ParamSetter : public rclcpp::Node
{
public:
    ParamSetter() : Node("param_setter")
    {
        client_ = this->create_client<mavros_msgs::srv::ParamSet>("/mavros/param/set");
        while (!client_->wait_for_service(std::chrono::seconds(1))) {
            RCLCPP_WARN(this->get_logger(), "Waiting for /mavros/param/set service to be available");
        }

        set_param("PWM_AUX_FUNC1", 201.0);
        set_param("PWM_AUX_DIS1", 1500.0);
    }

private:
    void set_param(const std::string &param_id, double value)
    {
        auto request = std::make_shared<mavros_msgs::srv::ParamSet::Request>();
        request->param_id = param_id;
        request->value.integer = 0;
        request->value.real = value;

        auto result = client_->async_send_request(request);
        // Wait for the result.
        if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), result) ==
            rclcpp::FutureReturnCode::SUCCESS)
        {
            RCLCPP_INFO(this->get_logger(), "Set param %s to %f", param_id.c_str(), value);
        }
        else
        {
            RCLCPP_ERROR(this->get_logger(), "Failed to call service /mavros/param/set");
        }
    }

    rclcpp::Client<mavros_msgs::srv::ParamSet>::SharedPtr client_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<ParamSetter>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
