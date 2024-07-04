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

    static void	sigintHandler(int signum);

private:
    void processInput();
    bool publish(void);
    
    static void _setIsRunning(const bool flag);
    static bool _getIsRunning(void);
    static bool _action(void);

    static bool _pressH(void);
    static bool _pressQ(void);
    static bool _pressB(void);
    static bool _pressArrow(void);
    static bool _pressArrowUp(void);
    static bool _pressArrowDown(void);
    static bool _pressArrowLeft(void);
    static bool _pressArrowRight(void);
    static bool _pressU(void);
    static bool _pressD(void);
    static bool _press8(void);
    static bool _press2(void);
    static bool _press3(void);
    static bool _press4(void);
    static bool _press5(void);
    static bool _press6(void);
    static bool _press7(void);
    static bool _pressW(void);
    static bool _pressPlus(void);
    static bool _pressMinus(void);
    static bool _pressQuestionmark(void);

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr _publisher;
    std::thread                 input_thread;
    TerminalMode                rawMode;

    static bool                 _is_running;
    static const std::string    _key_string;
    static const std::string    _arrow_string;
    static bool (*keyFunc[])(void);
    static std::mutex _mtx;  // 공유 데이터에 대한 접근을 보호하기 위한 뮤텍스
    static char                 c;
    static std::string          _arrow_str;
};

#endif