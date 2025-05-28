#include "calibration.h"
#include "BSP/stm32f3_discovery_gyroscope.h"
#include "BSP/stm32f3_discovery_accelerometer.h"
#include "serial.h"
#include "main.h"  // For delay, HAL includes
#include "delayfunction.h"
#include "stm32f3xx_hal_conf.h"

// Global calibration offsets


float gyro_bias_x = 0;
float gyro_bias_y = 0;
float gyro_bias_z = 0;
float mag_offset_x = 0, mag_offset_y = 0, mag_offset_z = 0;
float acc_offset_x = 0, acc_offset_y = 0, acc_offset_z = 0;
float soft_iron_scale = 1.0f;

// Gyroscope Zero Bias Calibration
void Gyroscope_Calibrate(void) {
    float sum_x = 0;
    float sum_y = 0;
    float sum_z = 0;
    float gyro_data[3];

    SerialOutputString("Calibrating Gyroscope... Keep device still.\r\n", &USART1_PORT);

    for (int i = 0; i < 200; i++) {
        BSP_GYRO_GetXYZ(gyro_data);
        sum_x += gyro_data[0];  // X-axis
        sum_y += gyro_data[1];  // Y-axis
        sum_z += gyro_data[2];  // Z-axis
        delay(10);

    }

    gyro_bias_x = sum_x / 200;
    gyro_bias_y = sum_y / 200;
    gyro_bias_z = sum_z / 200;

    SerialOutputString("Gyro Calibration Done.\r\n", &USART1_PORT);
}

// Magnetometer Offset Calibration (X, Y, Z)
void Magnetometer_Calibrate(void) {
    int16_t mag_data[3];
    int16_t mag_x_max = -32768, mag_x_min = 32767;
    int16_t mag_y_max = -32768, mag_y_min = 32767;
    int16_t mag_z_max = -32768, mag_z_min = 32767;

    SerialOutputString("Rotate device 360 deg for Magnetometer Calibration...\r\n", &USART1_PORT);

    for (int i = 0; i < 500; i++) {
        Magnetometer_ReadXYZ(mag_data);
        int16_t mag_x = mag_data[0];
        int16_t mag_y = mag_data[1];
        int16_t mag_z = mag_data[2];

        if (mag_x > mag_x_max) mag_x_max = mag_x;
        if (mag_x < mag_x_min) mag_x_min = mag_x;
        if (mag_y > mag_y_max) mag_y_max = mag_y;
        if (mag_y < mag_y_min) mag_y_min = mag_y;
        if (mag_z > mag_z_max) mag_z_max = mag_z;
        if (mag_z < mag_z_min) mag_z_min = mag_z;

        delay(10);

    }

    mag_offset_x = (mag_x_max + mag_x_min) / 2.0f;
    mag_offset_y = (mag_y_max + mag_y_min) / 2.0f;
    mag_offset_z = (mag_z_max + mag_z_min) / 2.0f;

    // Soft Iron Scaling Correction
	float radius_x = (mag_x_max - mag_x_min) / 2.0f;
	float radius_y = (mag_y_max - mag_y_min) / 2.0f;
	soft_iron_scale = radius_x / radius_y;

    SerialOutputString("Magnetometer Calibration Done.\r\n", &USART1_PORT);
}


// Accelerometer Offset Calibration
void Accelerometer_Calibrate(void) {
    int16_t acc_data[3];
    float sum_x = 0, sum_y = 0, sum_z = 0;

    SerialOutputString("Place board flat for Accelerometer Calibration...\r\n", &USART1_PORT);

    for (int i = 0; i < 200; i++) {
        BSP_ACCELERO_GetXYZ(acc_data);
        sum_x += acc_data[0];
        sum_y += acc_data[1];
        sum_z += acc_data[2];
        delay(10);

    }

    acc_offset_x = sum_x / 200;
    acc_offset_y = sum_y / 200;
    acc_offset_z = (sum_z / 200) - 1000.0;  // Assuming 1g = 1000mg

    SerialOutputString("Accelerometer Calibration Done.\r\n", &USART1_PORT);
}
