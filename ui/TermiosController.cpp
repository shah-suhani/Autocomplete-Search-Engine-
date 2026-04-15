#include "TermiosController.hpp"
#include <unistd.h>
#include <stdexcept>

namespace ui {

TermiosController::TermiosController() {
    if (tcgetattr(STDIN_FILENO, &original_) < 0)
        throw std::runtime_error("TermiosController: tcgetattr failed");

    struct termios raw = original_;
    raw.c_iflag &= ~(ICRNL | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) < 0)
        throw std::runtime_error("TermiosController: tcsetattr failed");
}

TermiosController::~TermiosController() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_);
}

} 
