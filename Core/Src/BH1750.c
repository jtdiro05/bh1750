#include "BH1750.h"
#include <stdbool.h>

#define TIMEOUT_10MS (10)
#define H_RES_MEAS_TIME_MS (180)
#define L_RES_MEAS_TIME_MS (24)

static bh1750_t* sensors[2] = {NULL, NULL};


//------------------------------------
// private functions
static HAL_StatusTypeDef register_sensor(bh1750_t *bh1750){

	if(bh1750 == NULL)
		return HAL_ERROR;

	else if(sensors[0] == NULL || sensors[0] == bh1750)
		sensors[0] = bh1750;
	else if(sensors [1] == NULL || sensors[1] == bh1750)
		sensors[1] = bh1750;
	else
		return HAL_ERROR;

	return HAL_OK;
}


//---------------
static HAL_StatusTypeDef unregister_sensor(bh1750_t *bh1750){

	if(bh1750 == NULL)
		return HAL_ERROR;

	if(sensors[0] == bh1750)
		sensors[0] = NULL;

	if(sensors[1] == bh1750)
		sensors[1] = NULL;

	return HAL_OK;
}


//---------------
static HAL_StatusTypeDef send_instruction(bh1750_t *bh1750, BH1750_instruction_t instruction){

	if(bh1750 == NULL)
		return HAL_ERROR;

	uint8_t data = (uint8_t) instruction;

	return HAL_I2C_Master_Transmit(bh1750->hi2c, bh1750->i2c_addr, &data, 1, TIMEOUT_10MS);
}


//----------------
static HAL_StatusTypeDef receive_data(bh1750_t *bh1750){

	if(bh1750 == NULL)
		return HAL_ERROR;

	return HAL_I2C_Master_Receive_DMA(bh1750->hi2c, bh1750->i2c_addr, bh1750->rx_buffer, 2);
}


//----------------
static HAL_StatusTypeDef power_on(bh1750_t *bh1750, bool power){

	if(bh1750 == NULL)
		return HAL_ERROR;

	if(power)
		bh1750->state = BH1750_READY;
	else
		bh1750->state = BH1750_INIT;

	return send_instruction(bh1750, power ? BH1750_POWER_ON : BH1750_POWER_DOWN);
}


//----------------
static HAL_StatusTypeDef reset(bh1750_t *bh1750){

	if(bh1750 == NULL)
		return HAL_ERROR;

	return send_instruction(bh1750, BH1750_RESET);
}


//----------------
static bh1750_t* get_handle_by_tim(TIM_HandleTypeDef *htim){

	if(htim == NULL)
	    return NULL;

	if (sensors[0] != NULL && sensors[0]->htim == htim)
	    return sensors[0];

	if (sensors[1] != NULL && sensors[1]->htim == htim)
		return sensors[1];

	return NULL;
}


//----------------
static bh1750_t* get_handle_by_i2c(I2C_HandleTypeDef *hi2c){

	if(hi2c == NULL)
	    return NULL;

	if (sensors[0] != NULL && sensors[0]->hi2c == hi2c && sensors[0]->state == BH1750_BUSY)
	    return sensors[0];

	if (sensors[1] != NULL && sensors[1]->hi2c == hi2c && sensors[1]->state == BH1750_BUSY)
		return sensors[1];

	return NULL;
}


//----------------
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){

	bh1750_t *bh1750 = get_handle_by_tim(htim);

	if(bh1750 != NULL){
		HAL_TIM_Base_Stop_IT(htim);
		receive_data(bh1750);
	}
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c) {

    bh1750_t *bh1750 = get_handle_by_i2c(hi2c);

    if (bh1750 != NULL) {
        uint16_t raw_level = (bh1750->rx_buffer[0] << 8) | bh1750->rx_buffer[1];
        bh1750->latest_data = (float)raw_level / 1.2f;
        bh1750->state = BH1750_READY;
    }
}



//---------------------------------------------
// public functions
HAL_StatusTypeDef BH1750_init(bh1750_t *bh1750, I2C_HandleTypeDef *hi2c, TIM_HandleTypeDef *htim, bh1750_addr_t addr){

	if(bh1750 == NULL || hi2c == NULL || htim == NULL)
		return HAL_ERROR;

	bh1750->hi2c = hi2c;
	bh1750->htim = htim;
	bh1750->state = BH1750_READY;
	bh1750->meas_mode = BH1750_H_RES_M;
	bh1750->latest_data = 0;
	bh1750->i2c_addr = ((uint8_t) addr) << 1;
	bh1750->rx_buffer[0] = 0;
	bh1750->rx_buffer[1] = 0;

	return register_sensor(bh1750);
}


//----------------
HAL_StatusTypeDef BH1750_deinit(bh1750_t *bh1750){

	if(bh1750 == NULL)
		return HAL_ERROR;

	HAL_TIM_Base_Stop_IT(bh1750->htim);
	power_on(bh1750, 0);
	unregister_sensor(bh1750);

	bh1750->hi2c = NULL;
	bh1750->htim = NULL;
	bh1750->state = BH1750_DEINIT;

	return HAL_OK;
}


//----------------
HAL_StatusTypeDef BH1750_single_meas(bh1750_t *bh1750,  bh1750_mode_t mode){

	if(bh1750 == NULL)
		return HAL_ERROR;
	else if(bh1750->state == BH1750_BUSY)
		return HAL_BUSY;

	uint32_t meas_time;
	BH1750_instruction_t instruction;

	if(mode == BH1750_H_RES_M){
		meas_time = H_RES_MEAS_TIME_MS;
		instruction = BH1750_ONE_TIME_H_RES_MODE;
	}
	else if(mode == BH1750_H_RES2_M){
		meas_time = H_RES_MEAS_TIME_MS;
		instruction = BH1750_ONE_TIME_H_RES_MODE2;
	}
	else if(mode == BH1750_L_RES_M){
		meas_time = L_RES_MEAS_TIME_MS;
		instruction = BH1750_ONE_TIME_L_RES_MODE;
	}
	else
		return HAL_ERROR;


	if(send_instruction(bh1750, instruction) != HAL_OK)
		return HAL_ERROR;

	bh1750->state = BH1750_BUSY;
	__HAL_TIM_SET_AUTORELOAD(bh1750->htim, meas_time - 1);
	__HAL_TIM_SET_COUNTER(bh1750->htim, 0);
	return HAL_TIM_Base_Start_IT(bh1750->htim);

}

//----------------
float BH1750_get_data(bh1750_t *bh1750){

	if(bh1750 == NULL)
		return -67;

	return bh1750->latest_data;
}
