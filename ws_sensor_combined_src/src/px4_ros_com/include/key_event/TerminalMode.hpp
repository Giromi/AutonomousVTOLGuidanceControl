
#ifndef TERMINALMODE_HPP
# define TERMINALMODE_HPP

#include <termios.h>
#include <iostream>
#include <unistd.h>

/* 키보드 입력을 즉시 받아들이기 위해서는  터미널 설정을 바꿔야 함 */
/* 기존 터미널 세팅은 Enter 키를 누를 때까지 입력을 받아들이지 않음 */
class TerminalMode {
public:
    TerminalMode(void);
    ~TerminalMode(void);
private:
    termios orig_termios;
};

#endif