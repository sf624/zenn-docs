#include "mcdc.hpp"

void mcdc(bool a, bool b, bool c) {
    if ((a && b) || c) {
        volatile int i = 0;
    }
}
