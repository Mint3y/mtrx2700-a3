#ifndef DELAYFUNCTION_H
#define DELAYFUNCTION_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Blocking millisecond delay using TIM2, one-pulse mode
void delay(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif // DELAYFUNCTION_H
