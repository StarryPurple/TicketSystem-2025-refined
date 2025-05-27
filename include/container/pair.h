#ifndef INSOMNIA_PAIR_H
#define INSOMNIA_PAIR_H

namespace insomnia {

template <class T1, class T2>
struct pair {
  T1 first;
  T2 second;
  constexpr pair() = default;
  template <class U1, class U2>
  constexpr pair(U1 &&u1, U2 &&u2)
    : first(std::forward<U1>(u1)), second(std::forward<U2>(u2)) {}
};

}

#endif