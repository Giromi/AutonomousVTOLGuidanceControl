#include "px4_ros_com/OffboardMavros.hpp"

    /* -- StateCommand Function*/

void OffboardMavros::localPositionCommandStart(void) {

    const Eigen::Vector4d cur_position_vector3d(
        _cur_position[0], _cur_position[1], _cur_position[2], vtol::NaN
    );

    if (wp_manager.isArrived(cur_position_vector3d)) {
        wp_manager.pop();
    }
}
//


// /* -- Callback Functions -- */
// void OffboardMavros::offboardResponseCallback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future, const std::array<const std::string, 2> msg) {
//     const bool result = future.get()->mode_sent;
//     printSuccessInfo(result, msg);
// }
//
// void OffboardMavros::positionResponseCallback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future, const std::array<const std::string, 2> msg) {
//     const bool result = future.get()->mode_sent;
//     printSuccessInfo(result, msg);
// }
//
// void OffboardMavros::missionResponseCallback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future, const std::array<const std::string, 2> msg) {
//     const bool result = future.get()->mode_sent;
//     printSuccessInfo(result, msg);
//     if (result == true) {
//         ;
//     } else {
//         ;
//     }
// }
//
// void	OffboardMavros::holdResponseCallback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future, const std::array<const std::string, 2> msg) {
//     const bool result = future.get()->mode_sent;
//     printSuccessInfo(result, msg);
//     // if (result == true) {
//     //     ;
//     // } else {
//     //     ;
//     // }
// }


// void OffboardMavros::turnOffBitIfPassedSeconds(const t_bit bit, const double timer) {
//     last_request = this->now();
// }

// shared_future를 사용하는 이유는 비동기로 요청을 보내기 때문에 요청에 대한 응답을 받아야하기 때문이다.
