#ifndef INSOMNIA_BUFFER_POOL_H
#define INSOMNIA_BUFFER_POOL_H

#include "fstream.h"

namespace insomnia {


template <class T, class Meta = void>
class BufferPool {
private:
  struct Frame {

  };
public:
  class Reader {

  };
  class Writer {

  };
private:

};

}

#include "buffer_pool.tcc"

#endif