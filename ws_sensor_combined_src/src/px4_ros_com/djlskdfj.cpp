
#include <stdio.h>

char str[] = "Hello worldsjdflaksjdflkasjd;flkajs;dlfkjas;dlkfja;lskdjflskd";

int main(void) {
    for(size_t i = 0; str[i] == '\0'; ++i) {
        printf("%c\n", str[i]);
    }
}





