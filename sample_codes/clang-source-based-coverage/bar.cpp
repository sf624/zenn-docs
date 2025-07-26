#include "bar.hpp"

void bar(const int x, const int y) {
  if ((x > 0) && (y > 0)) {
    volatile int i = 0;
  } else {
    volatile int i = 1;
  }
}
