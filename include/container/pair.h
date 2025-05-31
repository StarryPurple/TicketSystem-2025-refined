#ifndef INSOMNIA_PAIR_H
#define INSOMNIA_PAIR_H

namespace insomnia {

template <class T1, class T2>
struct pair {
  T1 first;
  T2 second;
  constexpr pair() = default;
  template <class U1, class U2>
  constexpr pair(U1 &&x, U2 &&y)
    : first(std::forward<U1>(x)), second(std::forward<U2>(y)) {}
};

template <class T1, class T2>
pair<T1, T2> make_pair(T1 &&x, T2 &&y) {
  return pair<T1, T2>(std::forward<T1>(x), std::forward<T2>(y));
}

}

#endif