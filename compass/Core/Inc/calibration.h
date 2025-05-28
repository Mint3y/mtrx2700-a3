#ifndef __CALIBRATION_H
#define __CALIBRATION_H

#include <stdint.h>

// Global variables for calibration offsets
extern float gyro_bias_x;
extern float gyro_bias_y;
extern float gyro_bias_z;
extern float mag_offset_x;
extern float mag_offset_y;
extern float mag_offset_z;
extern float soft_iron_scale;
extern float acc_offset_x;
extern float acc_offset_y;
extern float acc_offset_z;

// Function Prototypes
void Gyroscope_Calibrate(void);
void Magnetometer_Calibrate(void);
void Accelerometer_Calibrate(void);

#endif  // __CALIBRATION_H
