#include "c1.hpp"

void c1(bool a, bool b, bool c) {
    if ((a && b) || c) {
        volatile int i = 0;
    }
}
