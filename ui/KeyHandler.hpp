#pragma once

namespace ui {

enum class KeyAction {
    CHAR,       
    BACKSPACE,
    ARROW_UP,
    ARROW_DOWN,
    ENTER,
    TAB,       
    KEY_M,       
    KEY_A,      
    KEY_D,      
    KEY_B,       
    KEY_H,       
    ESC,         
    NUMBER,      
    CTRL_D,      
    UNKNOWN
};

struct KeyEvent {
    KeyAction action = KeyAction::UNKNOWN;
    char ch = 0; 
};

class KeyHandler {
public:
    static KeyEvent read();
};

} 