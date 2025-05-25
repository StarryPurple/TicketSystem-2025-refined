#ifndef INSOMNIA_OPTIONAL_TCC
#define INSOMNIA_OPTIONAL_TCC

#include "optional.h"

namespace insomnia {

template <class T>
optional<T>::optional()
  :  _has_value(false) {}

template <class T>
optional<T>::~optional() {
  reset();
}

template <class T>
optional<T>::optional(const optional &other) : optional() {
  if(other._has_value)
    new (data()) T(*other.data());
  _has_value = other._has_value;
}

template <class T>
optional<T>::optional(optional &&other) noexcept : optional() {
  if(other._has_value)
    new (data()) T(std::move(*other.data()));
  _has_value = other._has_value;
  other.reset();
}

template <class T>
optional<T>& optional<T>::operator=(const optional &other) {
  if(this == &other)
    return *this;
  reset();
  if(other._has_value)
    new (data()) T(*other.data());
  _has_value = other._has_value;
  return *this;
}

template <class T>
optional<T>& optional<T>::operator=(optional &&other) noexcept {
  if(this == &other)
    return *this;
  reset();
  if(other._has_value)
    new (data()) T(std::move(*other.data()));
  _has_value = other._has_value;
  other.reset();
  return *this;
}

template <class T>
optional<T>::optional(const T &t) : optional() {
  new (data()) T(t);
  _has_value = true;
}

template <class T>
optional<T>::optional(T &&t) noexcept : optional() {
  new (data()) T(std::move(t));
  _has_value = true;
}

template <class T>
optional<T>& optional<T>::operator=(const T &t) {
  reset();
  new (data()) T(t);
  _has_value = true;
  return *this;
}

template <class T>
optional<T>& optional<T>::operator=(T &&t) noexcept {
  reset();
  new (data()) T(std::move(t));
  _has_value = true;
  return *this;
}

template <class T>
template <class... Args>
optional<T>::optional(Args &&...args) : optional() {
  new (data()) T(std::forward<Args>(args)...);
  _has_value = true;
}

template <class T>
template <class... Args>
optional<T>& optional<T>::operator=(Args &&...args) noexcept {
  reset();
  new (data()) T(std::forward<Args>(args)...);
  _has_value = true;
  return *this;
}

template <class T>
void optional<T>::reset() {
  if(!_has_value)
    return;
  data()->~T();
  _has_value = false;
}

template <class T>
T& optional<T>::operator*() {
  if(!_has_value)
    throw container_is_empty("empty optional");
  return *data();
}

template <class T>
T* optional<T>::operator->() {
  if(!_has_value)
    throw container_is_empty("empty optional");
  return data();
}

template <class T>
const T& optional<T>::operator*() const {
  if(!_has_value)
    throw container_is_empty("empty optional");
  return *data();
}

template <class T>
const T* optional<T>::operator->() const {
  if(!_has_value)
    throw container_is_empty("empty optional");
  return data();
}

}

#endif