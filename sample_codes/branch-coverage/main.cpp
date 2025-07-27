#include "c0.hpp"
#include "region.hpp"
#include "c1.hpp"
#include "c2_without_short_circuit.hpp"
#include "c2_with_short_circuit.hpp"
#include "mcdc.hpp"

int main() {
    c0(true, true, true);

    region(true, false, true);

    c1(true, true, false);
    c1(false, true, false);

    c2_without_short_circuit(true, true, true);
    c2_without_short_circuit(false, false, false);

    c2_with_short_circuit(false, false, false);
    c2_with_short_circuit(true, true, false);
    c2_with_short_circuit(true, false, true);

    mcdc(false, false, false);
    mcdc(false, false, true);
    mcdc(true, true, false);
    mcdc(true, false, false);
}
