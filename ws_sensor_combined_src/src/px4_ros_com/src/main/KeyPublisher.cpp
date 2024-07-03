#include "key_event/KeyPublisher.hpp"


KeyPublisher::KeyPublisher() : Node("KeyPublisher") {
    _publisher = this->create_publisher<std_msgs::msg::String>("chatter", 10);
    // 비동기적으로 키보드 입력을 처리
    // 이 스레드는 노드의 메인 루프와 별도로 실행되므로, 
    // rclcpp::spin(node) 호출에 의해 블로킹되지 않고 키보드 입력을 계속 받을 수 있음
    _input_thread = std::thread([this]() { _processInput(); });
}

KeyPublisher::~KeyPublisher(void) {
    KeyPublisher::_set_is_running(false);
    if (_input_thread.joinable()) {
        _input_thread.join();
    }
}

void KeyPublisher::sigint_handler(int signum) {
    KeyPublisher::_set_is_running(false);
    if (signum == SIGINT) {
        rclcpp::shutdown();
    }
}

void KeyPublisher::_processInput() {
    while (rclcpp::ok() 
            && KeyPublisher::_get_is_running()
            && KeyPublisher::_action()
            && KeyPublisher::_publish());
}

bool KeyPublisher::_publish(void) {
    auto message = std_msgs::msg::String();
    message.data = (c == '\0') ? arrow_str : std::string(1, c);
    RCLCPP_INFO(this->get_logger(), "Publishing: '%s'\n", message.data.c_str());
    _publisher->publish(message);
    return true;
}

void KeyPublisher::_set_is_running(const bool flag) {
    _mtx.lock();
    _is_running = flag;
    _mtx.unlock();
}

bool KeyPublisher::_get_is_running(void) {
    _mtx.lock();
    const bool result = _is_running;
    _mtx.unlock();
    return result;
}

bool KeyPublisher::_action(void) {
    std::cout << "VTOL Shell: ";
    c = std::getchar();
    std::size_t i = KeyPublisher::_key_string.find(c);
    if (i == std::string::npos) //  못찼으면 std::string::npos 반환    
        return true;
    const bool result = KeyPublisher::_key_func[i]();
    KeyPublisher::_set_is_running(result);
    return result;
}

bool KeyPublisher::press_h(void) {
    std::cout << ">>> Return to Home <<<\n" << std::endl;
    return true;
}

bool KeyPublisher::press_q(void) {
    std::cout << "Really Want? Press q to quit\n(if you don't, push any key)" << std::endl;
    c = std::getchar();
    if (c == 'q') {
        rclcpp::shutdown();
        return false;
    }
    return true;
}

bool KeyPublisher::press_b(void) {
    std::cout << ">>> Back <<<\n" << std::endl;
    return true;
}

bool KeyPublisher::press_arrow(void) {
    if (std::getchar() == '[') {
        c = std::getchar();
    }
    switch (c) {
        case 'A': // 위쪽 방향키
           KeyPublisher::press_arrow_up();
            break;
        case 'B': // 아래쪽 방향키
            KeyPublisher::press_arrow_down();
            break;
        case 'C': // 오른쪽 방향키
            KeyPublisher::press_arrow_right();
            break;
        case 'D': // 왼쪽 방향키
            KeyPublisher::press_arrow_left();
            break;
        default:
            break;
    }
    c = '\0';
    return true;
}

bool KeyPublisher::press_arrow_up(void) {
    arrow_str = "↑";
    std::cout << ">>> N axis ++ <<< \n" << std::endl;
    return true;
}

bool KeyPublisher::press_arrow_down(void) {
    arrow_str = "↓";
    std::cout << ">>> N axis -- <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::press_arrow_left(void) {
    arrow_str = "←";
    std::cout << ">>> E axis -- <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::press_arrow_right(void) {
    arrow_str = "→";
    std::cout << ">>> E axis ++ <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::press_u(void){
    std::cout << ">>> U axis ++ <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::press_d(void){
    std::cout << ">>> U axis ++ <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::press_8(void) {
    std::cout << ">>> Y axis ++ <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::press_2(void) {
    std::cout << ">>> Roll ++ <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::press_3(void) {
    std::cout << ">>> Roll -- <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::press_4(void) {
    std::cout << ">>> Pitch ++ <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::press_5(void) {
    std::cout << ">>> Pitch -- <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::press_6(void) {
    std::cout << ">>> Yaw ++ <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::press_7(void) {
    std::cout << ">>> Yaw -- <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::press_w(void) {
    std::cout << ">>> VTOL Transition <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::press_plus(void) {
    std::cout << ">>> U axis ++ <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::press_minus(void) {
    std::cout << ">>> U axis -- <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::press_questionmark(void) {
    std::cout << ">>> Help <<< \n" << std::endl;
    std::cout << "< COMMAND >" << std::endl;
    std::cout << "  q: Quit" << std::endl;
    std::cout << "  h: Home" << std::endl;
    std::cout << "  b: Back" << std::endl;
    std::cout << "  ↑: Z axis ↑" << std::endl;
    std::cout << "  ↓: Z axis ↓" << std::endl;
    std::cout << "  6: X axis ↑" << std::endl;
    std::cout << "  4: X axis ↓" << std::endl;
    std::cout << "  8: Y axis ↑" << std::endl;
    std::cout << "  2: Y axis ↓" << std::endl;
    std::cout << "  ←: not yet" << std::endl;
    std::cout << "  →: not yet" << std::endl;
    std::cout << std::endl;
    return true;
}


std::mutex KeyPublisher::_mtx;  // 공유 데이터에 대한 접근을 보호하기 위한 뮤텍스
bool KeyPublisher::_is_running = true;
const std::string KeyPublisher::_key_string = "\033udqhb2345678?w+-";
bool (*KeyPublisher::_key_func[])() = {
    // & 의미 생략 가능, 가독성을 위해 추가
    &KeyPublisher::press_arrow,         // 지평방향 position control 제어
    &KeyPublisher::press_u,            // 고도++
    &KeyPublisher::press_d,            // 고도--
    &KeyPublisher::press_q,
    &KeyPublisher::press_h,
    &KeyPublisher::press_b,
    &KeyPublisher::press_2,
    &KeyPublisher::press_3,
    &KeyPublisher::press_4,
    &KeyPublisher::press_5,
    &KeyPublisher::press_6,
    &KeyPublisher::press_7,
    &KeyPublisher::press_8,
    &KeyPublisher::press_questionmark,
    &KeyPublisher::press_w,
    &KeyPublisher::press_plus,
    &KeyPublisher::press_minus
};

char KeyPublisher::c = '\0';
std::string KeyPublisher::arrow_str = "";
