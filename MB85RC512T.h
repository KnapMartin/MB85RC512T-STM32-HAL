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
#define MB85RC512T_TIMEOUT (uint32_t)100 // ms
#define MB85RC512T_INTERRUPT 0
#define MB85RC512T_MAX_ADDRESS (uint32_t)65536 // bytes
#define MB85RC512T_BUFFLEN 64
#define MB85RC512T_WRITE_LEN (uint32_t)32 // must be smaller than bufflen
#define MB85RC512T_PRINT 0

#if MB85RC512T_PRINT == 1
#define MB85RC512T_TIMEOUT_UART (uint32_t)100 // ms
#endif

typedef enum
{
	MB85RC512T_NONE = -1,
	MB85RC512T_OK = 0,
	MB85RC512T_ERROR_INIT,
	MB85RC512T_ERROR_ADDRESS,
	MB85RC512T_ERROR_TX,
	MB85RC512T_ERROR_RX,
	MB85RC512T_ERROR_UART,
} MB85RC512T_State;

typedef struct
{
	I2C_HandleTypeDef *m_hi2c;
	uint8_t m_address;
	uint8_t m_init;
	uint8_t m_data_rx[MB85RC512T_BUFFLEN];
	uint8_t m_data_tx[MB85RC512T_BUFFLEN];
} MB85RC512T;

MB85RC512T_State MB85RC512T_init(MB85RC512T *device, I2C_HandleTypeDef *hi2c, const uint8_t address);
MB85RC512T_State MB85RC512T_deinit(MB85RC512T *device);

MB85RC512T_State MB85RC512T_write(MB85RC512T *device, const uint32_t address, const uint8_t *data, const size_t len);
MB85RC512T_State MB85RC512T_read(MB85RC512T *device, const uint32_t address, uint8_t *data, const size_t len);

MB85RC512T_State MB85RC512T_reset(MB85RC512T *device, const uint8_t value);

#if MB85RC512T_PRINT == 1
MB85RC512T_State MB85RC512T_print(MB85RC512T *device, UART_HandleTypeDef *huart);
#endif


#ifdef __cplusplus
}
#endif

#endif /* MB85RC512T_H_ */
