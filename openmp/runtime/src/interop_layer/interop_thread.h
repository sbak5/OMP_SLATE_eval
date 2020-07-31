#ifndef INTEROP_THREAD_H_
#define INTEROP_THREAD_H_
#if HCLIB_OMP
#include "litectx.h"
typedef LiteCtx* kmp_thread_t;
#elif CHARM_OMP || CONV_OMP
#include "ompcharm.h"
typedef CthThread kmp_thread_t;
#endif
#endif
