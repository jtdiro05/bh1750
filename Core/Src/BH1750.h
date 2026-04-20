#ifndef BH1750_H
#define BH1750_H

#include <stdint.h>
#include "stm32u5xx_hal.h"


// instructions
typedef enum{
	BH1750_POWER_DOWN = 0x00,
	BH1750_POWER_ON = 0x01,
	BH1750_RESET = 0x07,
	BH1750_CONT_H_RES_MODE = 0x10,
	BH1750_CONT_H_RES_MODE2 = 0x11,
	BH1750_CONT_L_RES_MODE = 0x13,
	BH1750_ONE_TIME_H_RES_MODE = 0x20, // 120ms, 1lx
	BH1750_ONE_TIME_H_RES_MODE2 = 0x21, // 120ms, 0.5lx
	BH1750_ONE_TIME_L_RES_MODE = 0x23, // 16ms, 4lx
	BH1750_CHANGE_MEAS_TIME_HB = 0x40,
	BH1750_CHANGE_MEAS_TIME_LB = 0x60
} BH1750_instruction_t;


// address
typedef enum {
	BH1750_ADDR_H = 0x5C,
	BH1750_ADDR_L = 0x23
} bh1750_addr_t;


// status
typedef enum {
	BH1750_DEINIT,
	BH1750_INIT,
	BH1750_READY,
	BH1750_BUSY
} bh1750_state_t;


// measurement mode
typedef enum{
	BH1750_H_RES_M,
	BH1750_H_RES2_M,
	BH1750_L_RES_M
} bh1750_mode_t;


// BH1750 type
typedef struct{
	I2C_HandleTypeDef *hi2c;
	TIM_HandleTypeDef *htim;
	bh1750_state_t state;
	bh1750_mode_t meas_mode;
	float latest_data;
	uint16_t i2c_addr;
	uint8_t rx_buffer[2];
} bh1750_t;


HAL_StatusTypeDef BH1750_init(bh1750_t *bh1750, I2C_HandleTypeDef *hi2c, TIM_HandleTypeDef *htim, bh1750_addr_t addr);
HAL_StatusTypeDef BH1750_deinit(bh1750_t *bh1750);
HAL_StatusTypeDef BH1750_single_meas(bh1750_t *bh1750, bh1750_mode_t mode);
float BH1750_get_data(bh1750_t *bh1750);

#endif
