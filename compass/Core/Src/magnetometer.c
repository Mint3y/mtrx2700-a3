#include "stm32f3xx_hal.h"
#include "serial.h"

// Define Magnetometer I2C Address
#define MAG_I2C_ADDRESS  0x1E

extern I2C_HandleTypeDef hi2c1;  // Use the correct I2C handle

// Write to Magnetometer Register
void Magnetometer_WriteReg(uint8_t reg, uint8_t value) {
    HAL_I2C_Mem_Write(&hi2c1, MAG_I2C_ADDRESS << 1, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, 1000);
}

// Read from Magnetometer Register
uint8_t Magnetometer_ReadReg(uint8_t reg) {
    uint8_t value = 0;
    HAL_I2C_Mem_Read(&hi2c1, MAG_I2C_ADDRESS << 1, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, 1000);
    return value;
}

// Initialize Magnetometer Configuration Registers
void Magnetometer_Init(void) {
    Magnetometer_WriteReg(0x60, 0x8C);  // CFG_REG_A_M: Continuous mode, 100Hz, Temp Comp
    Magnetometer_WriteReg(0x61, 0x02);  // CFG_REG_B_M: Offset cancel
    Magnetometer_WriteReg(0x62, 0x10);  // CFG_REG_C_M: Block Data Update
    HAL_Delay(10);  // Short delay after config
}

// Read Magnetometer XYZ values
void Magnetometer_ReadXYZ(int16_t *pDataXYZ) {
    uint8_t buffer[6];

    // Read OUTX_L_M to OUTZ_H_M (0x68 to 0x6D)
    HAL_I2C_Mem_Read(&hi2c1, MAG_I2C_ADDRESS << 1, 0x68, I2C_MEMADD_SIZE_8BIT, buffer, 6, 1000);

    pDataXYZ[0] = (int16_t)(buffer[1] << 8 | buffer[0]);  // X-axis
    pDataXYZ[1] = (int16_t)(buffer[3] << 8 | buffer[2]);  // Y-axis
    pDataXYZ[2] = (int16_t)(buffer[5] << 8 | buffer[4]);  // Z-axis
}
