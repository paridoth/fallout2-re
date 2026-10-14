#ifndef FALLOUT_COMPAT_H_
#define FALLOUT_COMPAT_H_

#include <assert.h>

// MSVC exposes these helpers through the Windows headers. Keep the original
// source portable when building with GCC/MinGW, where NOMINMAX can suppress
// the Windows definitions.
#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#endif /* FALLOUT_COMPAT_H_ */
