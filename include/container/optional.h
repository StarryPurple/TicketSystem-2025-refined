#ifndef INSOMNIA_OPTIONAL_H
#define INSOMNIA_OPTIONAL_H

#include "exception.h"

namespace insomnia {

inline struct nullopt_t {} nullopt;

template <class T>
class optional {
private:

  alignas(T) char _storage[sizeof(T)];
  bool _has_value;

  T* data() { return reinterpret_cast<T*>(_storage); }

public:

  optional();
  ~optional();
  optional(const optional &);
  optional(optional &&) noexcept;
  optional& operator=(const optional &);
  optional& operator=(optional &&) noexcept;
  optional(const T &);
  optional(T &&) noexcept;
  optional& operator=(const T&);
  optional& operator=(T &&) noexcept;
  template <class ...Args>
  optional(Args &&...);
  template <class ...Args>
  optional& operator=(Args &&...) noexcept;
  explicit optional(nullopt_t) : optional() {}


  bool has_value() const { return _has_value; }
  void reset();

  T& operator*();
  T* operator->();
  const T& operator*() const;
  const T* operator->() const;
};

template <class T>
optional<T> make_optional(const T &t) { return optional<T>(t); }
template <class T>
optional<T> make_optional(T &&t) { return optional<T>(std::move(t)); }

}

#include "optional.tcc"

#endif