#include "foo.hpp"
#include "bar.hpp"
#include "buz.hpp"

int main() {
  foo(false, false, false);
  foo(true, true, false);
  foo(true, false, true);

  bar(false, false, false);
  bar(false, false, true);
  bar(true, true, false);
  bar(true, false, false);

  buz<int>(false, false, false);
  buz<int>(false, false, true);
  buz<int>(true, true, false);
  buz<long>(true, false, false);

  return 0;
}
