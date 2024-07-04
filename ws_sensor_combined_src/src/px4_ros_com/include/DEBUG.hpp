
#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <bitset>
#include <iostream>

#define RESET           "\033[0m"
#define BLACK           "\033[30m"              /* Black */
#define RED             "\033[31m"              /* Red */
#define GREEN           "\033[32m"              /* Green */
#define YELLOW          "\033[33m"              /* Yellow */
#define BLUE            "\033[34m"              /* Blue */
#define MAGENTA         "\033[35m"              /* Magenta */
#define CYAN            "\033[36m"              /* Cyan */
#define WHITE           "\033[37m"              /* White */
#define BOLDBLACK       "\033[1m\033[30m"       /* Bold Black */
#define BOLDRED         "\033[1m\033[31m"       /* Bold Red */
#define BOLDGREEN       "\033[1m\033[32m"       /* Bold Green */
#define BOLDYELLOW      "\033[1m\033[33m"       /* Bold Yellow */
#define BOLDBLUE        "\033[1m\033[34m"       /* Bold Blue */
#define BOLDMAGENTA     "\033[1m\033[35m"       /* Bold Magenta */
#define BOLDCYAN        "\033[1m\033[36m"       /* Bold Cyan */
#define BOLDWHITE       "\033[1m\033[37m"       /* Bold White */
#define DEBUG_HEADER    BOLDWHITE << "[DEBUG] " << RESET

namespace DEBUG {
    template <typename T>
    void printBinary(const std::string& msg, const T& target, const char* color) {
        std::cout << DEBUG_HEADER << color 
                  << msg << std::bitset<8>(target) << RESET << std::endl;
    }

    template <typename T>
    void printBool(const std::string& msg, const T& target, const char* color) {
        std::cout << DEBUG_HEADER << color 
                  << msg << std::boolalpha << target << RESET << std::endl;
    }

    template <typename T>
    void print(const std::string& msg, const T& target, const char* color) {
        std::cout << DEBUG_HEADER << color 
                  << msg << target << RESET << std::endl;
    }

    template <typename T>
    void printArray(const std::string& msg, const T& target, const size_t len, const char* color="WHITE") {
        std::cout << DEBUG_HEADER << color << msg;
        for (size_t i = 0; i < len; i++) {
            std::cout << target[i] << ", ";
        }
        std::cout << RESET << std::endl;
    }

    template <typename T>
    void msg(const std::string& msg, const T& target, const char* color = BOLDWHITE) {
        std::cout << color << msg << target << RESET << std::endl;
    }

}

#endif
