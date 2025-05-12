#ifndef INSOMNIA_EXCEPTION_H
#define INSOMNIA_EXCEPTION_H

#include <stdexcept>

namespace insomnia {

// inherits must be public for catch(std::exception &) to catch.

class insomnia_exception : public std::runtime_error {
public:
  explicit insomnia_exception(const char *detail = "") : runtime_error(detail) {}
};

class container_exception : public insomnia_exception {
public:
  explicit container_exception(const char *detail = "") : insomnia_exception(detail) {}
};

class container_is_empty : public container_exception {
public:
  explicit container_is_empty(const char *detail = "") : container_exception(detail) {}
};

class invalid_iterator : public container_exception {
public:
  explicit invalid_iterator(const char *detail = "") : container_exception(detail) {}
};

class index_out_of_bound : public container_exception {
public:
  explicit index_out_of_bound(const char *detail = "") : container_exception(detail) {}
};

class disk_exception : public insomnia_exception {
public:
  explicit disk_exception(const char *detail = "") : insomnia_exception(detail) {}
};

class segmentation_fault : public disk_exception {
public:
  explicit segmentation_fault(const char *detail = "") : disk_exception(detail) {}
};

class pool_exception : public insomnia_exception {
public:
  explicit pool_exception(const char *detail = "") : insomnia_exception(detail) {}
};

class pool_overflow : public pool_exception {
public:
  explicit pool_overflow(const char *detail = "") : pool_exception(detail) {}
};

class invalid_pool : public pool_exception {
public:
  explicit invalid_pool(const char *detail = "") : pool_exception(detail) {}
};

class database_exception : public insomnia_exception {
public:
  explicit database_exception(const char *detail = "") : insomnia_exception(detail) {}
};

class debug_exception : public database_exception {
public:
  explicit debug_exception(const char *detail = "") : database_exception(detail) {}
};

}

#endif