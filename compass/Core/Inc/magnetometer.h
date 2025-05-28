#ifndef MAGNETOMETER_H
#define MAGNETOMETER_H

#include <stdint.h>

void Magnetometer_Init(void);
void Magnetometer_ReadXYZ(int16_t *pDataXYZ);

#endif
