#pragma once
#include <termios.h>

namespace ui {

class TermiosController {
public:
    TermiosController();
    ~TermiosController();

    TermiosController(const TermiosController&) = delete;
    TermiosController& operator=(const TermiosController&) = delete;

private:
    struct termios original_;
};

} 
