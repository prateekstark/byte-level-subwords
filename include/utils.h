#ifndef UTILS_H
#define UTILS_H

#include <string>

static inline void trim(std::string& str) {
    int beginIndex = 0;
    int endIndex = str.size() - 1;
    for (int i = beginIndex; i <= endIndex; i++) {
        if(str[i] != ' ' && str[i] != '\n') {
            beginIndex = i;
            break;
        }
    }
    for (int i = endIndex; i >= beginIndex; i--) {
        if(str[i] != ' ' && str[i] != '\n') {
            endIndex = i;
            break;
        }
    }
    str = str.substr(beginIndex, endIndex + 1);
}

template <typename T>
static inline void print(T object) {
    std::cout << object << std::endl;
    
}

#endif