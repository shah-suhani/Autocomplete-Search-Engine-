#include "KeyHandler.hpp"
#include <unistd.h>

namespace ui {

KeyEvent KeyHandler::read() {
    unsigned char c = 0;
    if (::read(STDIN_FILENO, &c, 1) <= 0)
        return {KeyAction::CTRL_D};

    if (c == 0x1b) {
        unsigned char seq[2] = {0, 0};
        if (::read(STDIN_FILENO, &seq[0], 1) <= 0) return {KeyAction::ESC};
        if (::read(STDIN_FILENO, &seq[1], 1) <= 0) return {KeyAction::ESC};
        if (seq[0] == '[') {
            if (seq[1] == 'A') return {KeyAction::ARROW_UP};
            if (seq[1] == 'B') return {KeyAction::ARROW_DOWN};
        }
        return {KeyAction::ESC};
    }

    if (c == 4) return {KeyAction::CTRL_D};
    if (c == 9) return {KeyAction::TAB};
    if (c == 13 || c == 10) return {KeyAction::ENTER};
    if (c == 127 || c == 8) return {KeyAction::BACKSPACE};

    if (c >= 'a' && c <= 'z') return {KeyAction::CHAR, static_cast<char>(c)};

    if (c >= '1' && c <= '5') return {KeyAction::NUMBER, static_cast<char>(c)};

    char lc = (c >= 'A' && c <= 'Z') ? c + 32 : c;
    if (lc == 'm') return {KeyAction::KEY_M};
    if (lc == 'a') return {KeyAction::KEY_A};   
    if (lc == 'd') return {KeyAction::KEY_D};
    if (lc == 'b') return {KeyAction::KEY_B};
    if (lc == 'h') return {KeyAction::KEY_H};

    if (c == ' ') return {KeyAction::CHAR, ' '};

    return {KeyAction::UNKNOWN};
}

} 
