#ifndef __mips_internal_h__
#define __mips_internal_h__

#ifndef ENABLE_MSA
#define ENABLE_MSA 1
#endif

#if ENABLE_MSA
#include "msa.h"
#endif

#endif // __mips_internal_h__
