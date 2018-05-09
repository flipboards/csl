
#ifndef CSL_CONTEXT_H
#define CSL_CONTEXT_H

#include "util/memory.h"

class Context {
public:

    ConstStringPool strpool;
    MemoryPool astpool, constantpool, typepool;
};


#endif
