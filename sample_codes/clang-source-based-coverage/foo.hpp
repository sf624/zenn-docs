#ifndef FOO_HPP_
#define FOO_HPP_

template <typename T>
void foo(const T x, const T y) {
  if ((x > 0) && (y > 0)) {
    volatile int i = 0;
  } else {
    volatile int i = 1;
  }
}

#endif // FOO_HPP_
