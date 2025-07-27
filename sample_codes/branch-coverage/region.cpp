#include "region.hpp"

void region(bool a, bool b, bool c) {
    if ((a && b) || c) {
        volatile int i = 0;
    }
}
