#include "key_event/TerminalMode.hpp"

/* 키보드 입력을 즉시 받아들이기 위해서는  터미널 설정을 바꿔야 함 */
/* 기존 터미널 세팅은 Enter 키를 누를 때까지 입력을 받아들이지 않음 */
TerminalMode::TerminalMode(void) {
    tcgetattr(STDIN_FILENO, &orig_termios);
    termios new_termios = orig_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    std::cout << ">>> Start <<<" << std::endl;
}

TerminalMode::~TerminalMode(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
    std::cout << ">>> Quit <<<" << std::endl;
}
