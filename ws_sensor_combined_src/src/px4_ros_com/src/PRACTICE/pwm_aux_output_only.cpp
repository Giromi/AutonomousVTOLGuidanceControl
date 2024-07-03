
#include <rclcpp/parameter_client.hpp>
#include <mavros_msgs/srv/param_set.hpp>
#include <rclcpp/rclcpp.hpp>

class MavrosParamSetter : public rclcpp::Node {
    public:
        MavrosParamSetter() : Node("mavros_param_setter")
    {
        set_parameter("PWM_AUX_DIS1", 1500);
    }

        void set_parameter(const std::string& param_name, int param_value)
        {
            auto parameters_client = std::make_shared<rclcpp::SyncParametersClient>(this, "/mavros/param");
            std::cout << "1 Set mavros param " << param_name << " to " << param_value << std::endl;
            auto results = parameters_client->set_parameters({
                    rclcpp::Parameter(param_name, param_value)
                    });
            std::cout << "2 Set mavros param " << param_name << " to " << param_value << std::endl;

            for (const auto & result : results) {
                if (!result.successful) {
                    RCLCPP_ERROR(this->get_logger(), "Failed to set parameter: %s", result.reason.c_str());
                    return ; // Optional: Stop checking after the first failure
                }

                RCLCPP_INFO(this->get_logger(), "Parameter %s set to %d", param_name.c_str(), param_value);
            };
        };
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MavrosParamSetter>());
    rclcpp::shutdown();
    return 0;
}

