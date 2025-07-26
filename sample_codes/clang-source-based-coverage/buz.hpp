#ifndef BUZ_HPP_
#define BUZ_HPP_

template <typename T>
void buz(bool a, bool b, bool c) {
    if ((a && b) || c) {
        volatile int i = 0;
    } else {
        volatile int j = 1;
    }
}

#endif // BUZ_HPP_
