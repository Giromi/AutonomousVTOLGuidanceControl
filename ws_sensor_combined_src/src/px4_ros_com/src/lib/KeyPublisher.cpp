#include "key_event/KeyPublisher.hpp"


KeyPublisher::KeyPublisher() : Node("KeyPublisher") {
    _publisher = this->create_publisher<std_msgs::msg::String>("chatter", 10);
    // 비동기적으로 키보드 입력을 처리
    // 이 스레드는 노드의 메인 루프와 별도로 실행되므로, 
    // rclcpp::spin(node) 호출에 의해 블로킹되지 않고 키보드 입력을 계속 받을 수 있음
    input_thread = std::thread([this]() { processInput(); });
}

KeyPublisher::~KeyPublisher(void) {
    KeyPublisher::_setIsRunning(false);
    if (input_thread.joinable()) {
        input_thread.join();
    }
}

void KeyPublisher::sigintHandler(int signum) {
    KeyPublisher::_setIsRunning(false);
    if (signum == SIGINT) {
        rclcpp::shutdown();
    }
}

void KeyPublisher::processInput() {
    while (rclcpp::ok() 
            && KeyPublisher::_getIsRunning()
            && KeyPublisher::_action()
            && KeyPublisher::publish());
}

bool KeyPublisher::publish(void) {
    auto message = std_msgs::msg::String();
    message.data = (c == '\0') ? _arrow_str : std::string(1, c);
    RCLCPP_INFO(this->get_logger(), "Publishing: '%s'\n", message.data.c_str());
    _publisher->publish(message);
    return true;
}

void KeyPublisher::_setIsRunning(const bool flag) {
    _mtx.lock();
    _is_running = flag;
    _mtx.unlock();
}

bool KeyPublisher::_getIsRunning(void) {
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
    const bool result = KeyPublisher::keyFunc[i]();
    KeyPublisher::_setIsRunning(result);
    return result;
}

bool KeyPublisher::_pressH(void) {
    std::cout << ">>> Return to Home <<<\n" << std::endl;
    return true;
}

bool KeyPublisher::_pressQ(void) {
    std::cout << "Really Want? Press q to quit\n(if you don't, push any key)" << std::endl;
    c = std::getchar();
    if (c == 'q') {
        rclcpp::shutdown();
        return false;
    }
    return true;
}

bool KeyPublisher::_pressB(void) {
    std::cout << ">>> Back <<<\n" << std::endl;
    return true;
}

bool KeyPublisher::_pressArrow(void) {
    if (std::getchar() == '[') {
        c = std::getchar();
    }
    switch (c) {
        case 'A': // 위쪽 방향키
           KeyPublisher::_pressArrowUp();
            break;
        case 'B': // 아래쪽 방향키
            KeyPublisher::_pressArrowDown();
            break;
        case 'C': // 오른쪽 방향키
            KeyPublisher::_pressArrowRight();
            break;
        case 'D': // 왼쪽 방향키
            KeyPublisher::_pressArrowLeft();
            break;
        default:
            break;
    }
    c = '\0';
    return true;
}

bool KeyPublisher::_pressArrowUp(void) {
    _arrow_str = "↑";
    std::cout << ">>> N axis ++ <<< \n" << std::endl;
    return true;
}

bool KeyPublisher::_pressArrowDown(void) {
    _arrow_str = "↓";
    std::cout << ">>> N axis -- <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::_pressArrowLeft(void) {
    _arrow_str = "←";
    std::cout << ">>> E axis -- <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::_pressArrowRight(void) {
    _arrow_str = "→";
    std::cout << ">>> E axis ++ <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::_pressU(void){
    std::cout << ">>> U axis ++ <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::_pressD(void){
    std::cout << ">>> U axis ++ <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::_press8(void) {
    std::cout << ">>> Y axis ++ <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::_press2(void) {
    std::cout << ">>> Roll ++ <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::_press3(void) {
    std::cout << ">>> Roll -- <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::_press4(void) {
    std::cout << ">>> Pitch ++ <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::_press5(void) {
    std::cout << ">>> Pitch -- <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::_press6(void) {
    std::cout << ">>> Yaw ++ <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::_press7(void) {
    std::cout << ">>> Yaw -- <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::_pressW(void) {
    std::cout << ">>> VTOL Transition <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::_pressPlus(void) {
    std::cout << ">>> U axis ++ <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::_pressMinus(void) {
    std::cout << ">>> U axis -- <<< \n" << std::endl;
    return true;
}

 bool KeyPublisher::_pressQuestionmark(void) {
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
bool (*KeyPublisher::keyFunc[])() = {
    // & 의미 생략 가능, 가독성을 위해 추가
    &KeyPublisher::_pressArrow,         // 지평방향 position control 제어
    &KeyPublisher::_pressU,            // 고도++
    &KeyPublisher::_pressD,            // 고도--
    &KeyPublisher::_pressQ,
    &KeyPublisher::_pressH,
    &KeyPublisher::_pressB,
    &KeyPublisher::_press2,
    &KeyPublisher::_press3,
    &KeyPublisher::_press4,
    &KeyPublisher::_press5,
    &KeyPublisher::_press6,
    &KeyPublisher::_press7,
    &KeyPublisher::_press8,
    &KeyPublisher::_pressQuestionmark,
    &KeyPublisher::_pressW,
    &KeyPublisher::_pressPlus,
    &KeyPublisher::_pressMinus
};

char KeyPublisher::c = '\0';
std::string KeyPublisher::_arrow_str = "";
