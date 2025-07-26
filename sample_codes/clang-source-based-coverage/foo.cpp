#include "foo.hpp"

void foo(bool a, bool b, bool c) {
    if ((a && b) || c) {
        volatile int i = 0;
    } else {
        volatile int j = 1;
    }
}
