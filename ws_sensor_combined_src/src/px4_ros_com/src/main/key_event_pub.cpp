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
    KeyPublisher() : Node("KeyPublisher") {
        _publisher = this->create_publisher<std_msgs::msg::String>("chatter", 10);
        // 비동기적으로 키보드 입력을 처리
        // 이 스레드는 노드의 메인 루프와 별도로 실행되므로, 
        // rclcpp::spin(node) 호출에 의해 블로킹되지 않고 키보드 입력을 계속 받을 수 있음
        _input_thread = std::thread([this]() { _processInput(); });
    }


    ~KeyPublisher(void) {
        _set_is_running(false);
        if (_input_thread.joinable()) {
            _input_thread.join();
        }
    }

    static void	sigint_handler(int signum) {
        _set_is_running(false);
        if (signum == SIGINT) {
            rclcpp::shutdown();
        }
    }

private:

    void _processInput() {
        char c;
        while (rclcpp::ok() 
               && _check_running(c)
               && _action(c)
               && _publish(c));
    }

    bool _publish(const char c) {
        auto message = std_msgs::msg::String();
        message.data = std::string(1, c);
        RCLCPP_INFO(this->get_logger(), "Publishing: '%s'\n", message.data.c_str());
        _publisher->publish(message);
        return true;
    }

    static void _set_is_running(const bool flag) {
        KeyPublisher::_mtx.lock();
        KeyPublisher::_is_running = flag;
        KeyPublisher::_mtx.unlock();
    }

    static bool _get_is_running(void) {
        KeyPublisher::_mtx.lock();
        const bool result = KeyPublisher::_is_running;
        KeyPublisher::_mtx.unlock();
        return result;
    }

    static bool _check_running(char& c) {
        std::cout << "VTOL Shell: ";
        c = std::getchar();
        const bool result = _get_is_running();
        return result;
    }

    static bool _action(const char c) {
        std::cout << "'" << c << "'"<< std::endl;
        std::size_t i = KeyPublisher::_key_string.find(c);
        if (i == std::string::npos) //  못찼으면 std::string::npos 반환    
            return true;
        const bool result = KeyPublisher::_key_func[i](c);
        _set_is_running(result);
        return result;
    }

    static bool _press_help(const char c) {
        static_cast<void>(c);
        std::cout << ">>> Help <<< \n" << std::endl;
        std::cout << "< COMMAND >" << std::endl;
        std::cout << "  q: Quit" << std::endl;
        std::cout << "  h: Home" << std::endl;
        std::cout << "  b: Back" << std::endl;
        std::cout << std::endl;
        return true;
    }

    static bool _press_quit(const char c) {
        std::cout << "Really Want? Press q to quit\n(if you don't, push any key)" << std::endl;
        const char q = std::getchar();
        if (c == q) {
            rclcpp::shutdown();
            return false;
        }
        return true;
    }

    static bool _press_back(const char c) {
        static_cast<void>(c);
        std::cout << ">>> Back <<< \n" << std::endl;
        return true;
    }

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr _publisher;
    std::thread                 _input_thread;
    TerminalMode                _rawMode;

    static bool                 _is_running;
    static const std::string    _key_string;
    static bool (*_key_func[])(const char c);
    static std::mutex _mtx;  // 공유 데이터에 대한 접근을 보호하기 위한 뮤텍스
};

std::mutex KeyPublisher::_mtx;  // 공유 데이터에 대한 접근을 보호하기 위한 뮤텍스
bool KeyPublisher::_is_running = true;
const std::string KeyPublisher::_key_string = "qhb";
bool (*KeyPublisher::_key_func[])(const char c) = {
    // & 의미 생략 가능, 가독성을 위해 추가
    &KeyPublisher::_press_quit,
    &KeyPublisher::_press_help,
    &KeyPublisher::_press_back
};

int main(int argc, char **argv) {
	signal(SIGINT, KeyPublisher::sigint_handler);
    rclcpp::init(argc, argv);
    auto node = std::make_shared<KeyPublisher>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}

