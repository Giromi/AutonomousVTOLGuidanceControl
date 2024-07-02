#ifndef KEYPUBLISHER_HPP
# define KEYPUBLISHER_HPP

#include "key_event/TerminalMode.hpp"
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <thread>
#include <memory>


class KeyPublisher : public rclcpp::Node {
public:
    KeyPublisher(void);
    ~KeyPublisher(void);

    static void	sigint_handler(int signum);

private:
    void _processInput();
    bool _publish(void);
    
    static void _set_is_running(const bool flag);
    static bool _get_is_running(void);
    static bool _action(void);

    static bool press_h(void);
    static bool press_q(void);
    static bool press_b(void);
    static bool press_arrow(void);
    static bool press_arrow_up(void);
    static bool press_arrow_down(void);
    static bool press_arrow_left(void);
    static bool press_arrow_right(void);
    static bool press_u(void);
    static bool press_d(void);
    static bool press_8(void);
    static bool press_2(void);
    static bool press_3(void);
    static bool press_4(void);
    static bool press_5(void);
    static bool press_6(void);
    static bool press_7(void);
    static bool press_w(void);
    static bool press_plus(void);
    static bool press_minus(void);
    static bool press_questionmark(void);

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr _publisher;
    std::thread                 _input_thread;
    TerminalMode                _rawMode;

    static bool                 _is_running;
    static const std::string    _key_string;
    static const std::string    _arrow_string;
    static bool (*_key_func[])(void);
    static std::mutex _mtx;  // 공유 데이터에 대한 접근을 보호하기 위한 뮤텍스
    static char                 c;
    static std::string          arrow_str;
};

#endif