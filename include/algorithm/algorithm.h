#ifndef INSOMNIA_ALGORITHM_H
#define INSOMNIA_ALGORITHM_H

namespace insomnia {

template <class RAIt, class Compare>
void sort(RAIt begin, RAIt end, Compare comp);

template <class RAIt>
void sort(RAIt begin, RAIt end) {
  using compare_t = std::less<typename std::iterator_traits<RAIt>::value_type>;
  sort(begin, end, compare_t());
}

}

#include "algorithm.tcc"

#endif