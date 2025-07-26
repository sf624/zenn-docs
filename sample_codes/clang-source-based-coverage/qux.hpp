#ifndef QUX_HPP_
#define QUX_HPP_

template <typename T>
void qux(bool a, bool b, bool c) {
    if ((a && b) || c) {
        volatile int i = 0;
    } else {
        volatile int j = 1;
    }
}

#endif // QUX_HPP_
