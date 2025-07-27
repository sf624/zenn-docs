#include "c2_with_short_circuit.hpp"

void c2_with_short_circuit(bool a, bool b, bool c) {
    if ((a && b) || c) {
        volatile int i = 0;
    }
}
