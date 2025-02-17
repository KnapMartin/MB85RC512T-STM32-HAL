/*
 * MB85RC512T.h
 *
 *  Created on: Feb 17, 2025
 *      Author: knap-linux
 */

#ifndef MB85RC512T_H_
#define MB85RC512T_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define MB85RC512T_VERSION "0.0.0"
#define MB85RC512T_TEST 0
#define MB85RC512T_TIMEOUT 100 // ms
#define MB85RC512T_INTERRUPT 0

typedef enum
{
	MB85RC512T_NONE = -1,
	MB85RC512T_OK,
	MB85RC512T_ERROR,
	MB85RC512T_ERROR_TX,
	MB85RC512T_ERROR_RX
} MB85RC512T_State;

typedef struct
{
	I2C_HandleTypeDef *m_hi2c;
	uint8_t m_address;
	uint8_t m_init;
	uint8_t m_data_rx[8];
	uint8_t m_data_tx[8];
} MB85RC512T;

MB85RC512T_State MB85RC512T_init(MB85RC512T *device, I2C_HandleTypeDef *hi2c, const uint8_t address);
MB85RC512T_State MB85RC512T_deinit(MB85RC512T *device);

MB85RC512T_State MB85RC512T_write8(MB85RC512T *device, const uint8_t data);
MB85RC512T_State MB85RC512T_read8(MB85RC512T *device, uint8_t *data);

#if MB85RC512T_TEST == 1

#endif

#ifdef __cplusplus
}
#endif

#endif /* MB85RC512T_H_ */
