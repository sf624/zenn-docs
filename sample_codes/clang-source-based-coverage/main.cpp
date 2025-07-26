#include "foo.hpp"
#include "bar.hpp"

int main() {
  bar(2, 3);
  bar(2, -5);
  bar(-3, 3);

  foo<int>(2, 3);
  foo<int>(2, -5);
  foo<long>(-3, 3);

  for (int i = 0; i < 10; ++i) {
    volatile int j = i;
  }

  return 0;
}
