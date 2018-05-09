
#ifndef CSL_CONTEXT_H
#define CSL_CONTEXT_H

#include "util/memory.h"

namespace csl {

class Context {
public:

    ConstStringPool strpool;
    MemoryPool astpool, constantpool, typepool;
};

}

#endif
