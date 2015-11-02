
#ifndef FILTER_H_
#define FILTER_H_
#include <stdint.h>

#define IIR_SHIFT         8

int16_t iirLPFilterSingle(int32_t in, int32_t attenuation,  int32_t* filt);

#endif //FILTER_H_
