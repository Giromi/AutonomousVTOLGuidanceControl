#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <memory>


/* 키보드 입력을 즉시 받아들이기 위해서는  터미널 설정을 바꿔야 함 */
/* 기존 터미널 세팅은 Enter 키를 누를 때까지 입력을 받아들이지 않음 */
class TerminalMode {
public:
    TerminalMode(void) {
        tcgetattr(STDIN_FILENO, &orig_termios);
        termios new_termios = orig_termios;
        new_termios.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
        std::cout << ">>> Start <<<" << std::endl;
    }

    ~TerminalMode(void) {
        tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
        std::cout << ">>> Quit <<<" << std::endl;
    }

private:
    termios orig_termios;
};

class KeyPublisher : public rclcpp::Node {
public:
    KeyPublisher() : Node("keyboard_publisher"), _is_running(true) {
        _publisher = this->create_publisher<std_msgs::msg::String>("chatter", 10);
        // 비동기적으로 키보드 입력을 처리
        // 이 스레드는 노드의 메인 루프와 별도로 실행되므로, 
        // rclcpp::spin(node) 호출에 의해 블로킹되지 않고 키보드 입력을 계속 받을 수 있음
        _input_thread = std::thread([this]() { _processInput(); });
    }

    ~KeyPublisher(void) {
        _is_running = false;
        if (_input_thread.joinable()) {
            _input_thread.join();
        }
    }

    static void	sigint_handler(int signum) {
        if (signum == SIGINT) {
            rclcpp::shutdown();
        }
    }

private:
    void _processInput() {
        while (_is_running && rclcpp::ok()) {
            std::cout << "VTOL Shell: " << std::endl;
            char c = std::getchar();
            if (_check_quit(c) == true) {
                _is_running = false;
                rclcpp::shutdown();
                return ;
            } 
            auto message = std_msgs::msg::String();
            message.data = std::string(1, c);
            if (_is_running == true) {
                RCLCPP_INFO(this->get_logger(), "Publishing: '%s'", message.data.c_str());
            }
            _publisher->publish(message);
        }
    }

    bool _check_quit(char c) {
        if (_is_running == false) { // 이미 종료되었으면 무시(멀티스레드 때문에 가능한 상황)
            return true;
        }
        if (c == 'q') {          // singal quit Ctrl-C 처리
            std::cout << "Really Want? Press q to quit\n(if you don't, push any key)" << std::endl;
            c = std::getchar();
            if (c == 'q') {
                return true;
            } else {
                std::cout << ">>> Continue <<<" << std::endl;
            }
        }
        return false;
    }

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr _publisher;
    bool            _is_running;
    std::thread     _input_thread;
    TerminalMode    _rawMode;
};

int main(int argc, char **argv) {
	signal(SIGINT, KeyPublisher::sigint_handler);
    rclcpp::init(argc, argv);
    auto node = std::make_shared<KeyPublisher>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}

