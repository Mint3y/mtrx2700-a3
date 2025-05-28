#ifndef MADGWICK_H
#define MADGWICK_H

#include <math.h>

// Quaternion of sensor frame relative to auxiliary frame
extern float q0, q1, q2, q3;

// Madgwick filter function
void MadgwickAHRSupdate(float gx, float gy, float gz,
                        float ax, float ay, float az,
                        float mx, float my, float mz);

// Sample frequency in Hz and algorithm gain (tunable)
extern float beta;             // 2 * proportional gain
extern float sampleFreq;       // sample frequency in Hz

#endif
