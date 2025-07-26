#include "bar.hpp"

void bar(bool a, bool b, bool c) {
    if ((a && b) || c) {
        volatile int i = 0; // This line is executed when a and b are true, or c is true
    } else {
        volatile int j = 1; // This line is executed when a and b are false, and c is false
    }
}
