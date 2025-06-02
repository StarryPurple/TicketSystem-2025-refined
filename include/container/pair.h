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
  constexpr pair(std::initializer_list<std::decay_t<T1>> il1,
                 std::initializer_list<std::decay_t<T2>> il2)
    : first(*il1.begin()), second(*il2.begin()) {}

  constexpr auto operator<=>(const pair &other) const = default;
};

template <class T1, class T2>
pair<T1, T2> make_pair(T1 &&x, T2 &&y) {
  return pair<T1, T2>(std::forward<T1>(x), std::forward<T2>(y));
}

}

#endif