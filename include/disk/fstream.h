#ifndef INSOMNIA_FSTREAM_H
#define INSOMNIA_FSTREAM_H

namespace insomnia {

template <class T, class Meta>
requires (sizeof(T) % 4096 == 0 && sizeof(Meta) % 4096 == 0)
class fstream {

public:

  void write()

};

}



#endif