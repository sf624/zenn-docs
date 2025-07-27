#include "c0.hpp"

void c0(bool a, bool b, bool c) {
    if ((a && b) || c) {
        volatile int i = 0;
    }
}
