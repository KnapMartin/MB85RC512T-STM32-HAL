/*
 * MB85RC512T.c
 *
 *  Created on: Feb 17, 2025
 *      Author: knap-linux
 */

#include "MB85RC512T.h"

#include <string.h>
#if MB85RC512T_PRINT == 1
#include <stdio.h>
#endif

/**
 * @brief Initialize device object.
 * 
 * @param self Device object pointer.
 * @param hi2c HAL I2C handle.
 * @param address Device I2C address.
 * @return MB85RC512T_State 
 */
MB85RC512T_State MB85RC512T_init(struct MB85RC512T *self, I2C_HandleTypeDef *hi2c, const uint8_t address)
{
	if (address < 0x08 || address > 0x77)
	{
		return MB85RC512T_ERROR_I2C_ADDRESS;
	}

	self->m_address = address << 1;
	self->m_hi2c = hi2c;
	self->m_init = 1;
	self->m_timeout = MB85RC512T_DEFAULT_TIMEOUT;

	return MB85RC512T_OK;
}

/**
 * @brief Deinitialize device struct.
 * 
 * @param self Device object pointer.
 * @return MB85RC512T_State 
 */
MB85RC512T_State MB85RC512T_deinit(struct MB85RC512T *self)
{
	self->m_address = 0;
	self->m_hi2c = NULL;
	self->m_init = 0;

	return MB85RC512T_OK;
}

/**
 * @brief Device write function. To write at selected address pass a buffer pointer and buffer length.
 * 
 * @param self Device object pointer.
 * @param address Selected device address.
 * @param data Data to be written.
 * @param len Length of data to be written.
 * @return MB85RC512T_State 
 */
MB85RC512T_State MB85RC512T_write(struct MB85RC512T *self, const uint32_t address, const uint8_t *data, const size_t len)
{
	if (!self->m_init) return MB85RC512T_ERROR_INIT;
	if (address + len - 1 > MB85RC512T_MAX_ADDRESS) return MB85RC512T_ERROR_ADDRESS;

#if MB85RC512T_INTERRUPT == 1
	uint32_t timeout = HAL_GetTick() + self->m_timeout;
	while (self->m_hi2c->State != HAL_I2C_STATE_READY)
	{
		if (HAL_GetTick() > timeout) return MB85RC512T_ERROR_TIMEOUT;
	}
#endif

	self->m_data_tx[0] = (uint8_t)(address >> 8);
	self->m_data_tx[1] = (uint8_t)(address & 0xFF);

	memcpy(&self->m_data_tx[2], data, len);

#if MB85RC512T_INTERRUPT == 1
	if (HAL_I2C_Master_Transmit_IT(self->m_hi2c, self->m_address, self->m_data_tx, len + 2) != HAL_OK)
#else
	if (HAL_I2C_Master_Transmit(self->m_hi2c, self->m_address, self->m_data_tx, len + 2, self->m_timeout) != HAL_OK)
#endif
	{
		return MB85RC512T_ERROR_TX;
	}

	return MB85RC512T_OK;
}

/**
 * @brief Device read function. To read from selected address pass a buffer pointer and buffer length.
 * 
 * @param self Device object pointer.
 * @param address Selected device address.
 * @param data Outout buffer.
 * @param len Length of data to be read.
 * @return MB85RC512T_State 
 */
MB85RC512T_State MB85RC512T_read(struct MB85RC512T *self, const uint32_t address, uint8_t *data, const size_t len)
{
	if (!self->m_init) return MB85RC512T_ERROR_INIT;
	if (address + len - 1 > MB85RC512T_MAX_ADDRESS) return MB85RC512T_ERROR_ADDRESS;

#if MB85RC512T_INTERRUPT == 1
	uint32_t timeout = HAL_GetTick() + self->m_timeout;
	while (self->m_hi2c->State != HAL_I2C_STATE_READY)
	{
		if (HAL_GetTick() > timeout) return MB85RC512T_ERROR_TIMEOUT;
	}
#endif

	self->m_data_tx[0] = (uint8_t)(address >> 8);
	self->m_data_tx[1] = (uint8_t)(address & 0xFF);

	if (HAL_I2C_Master_Transmit(self->m_hi2c, self->m_address, self->m_data_tx, 2, self->m_timeout) != HAL_OK)
	{
		return MB85RC512T_ERROR_TX;
	}

	if (HAL_I2C_Master_Receive(self->m_hi2c, self->m_address, self->m_data_rx, len, self->m_timeout) != HAL_OK)
	{
		return MB85RC512T_ERROR_RX;
	}

	memcpy(data, self->m_data_rx, len);

	return MB85RC512T_OK;
}

/**
 * @brief Reset device state to a selected value.
 * 
 * @param self Device object pointer.
 * @param value Value to be set.
 * @return MB85RC512T_State 
 */
MB85RC512T_State MB85RC512T_reset(struct MB85RC512T *self, const uint8_t value)
{
    if (!self->m_init) return MB85RC512T_ERROR_INIT;

    uint32_t writeLen = MB85RC512T_WRITE_LEN;
    uint32_t pageSize = MB85RC512T_WRITE_LEN;

    memset(&self->m_data_tx[2], value, writeLen);

    for (uint32_t address = 0; address < MB85RC512T_MAX_ADDRESS; address += writeLen)
    {
        uint32_t remainingInPage = pageSize - (address % pageSize);
        uint32_t chunkSize = (writeLen > remainingInPage) ? remainingInPage : writeLen;

        self->m_data_tx[0] = (uint8_t)(address >> 8);
        self->m_data_tx[1] = (uint8_t)(address & 0xFF);

        if (HAL_I2C_Master_Transmit(self->m_hi2c, self->m_address, self->m_data_tx, chunkSize + 2, self->m_timeout) != HAL_OK)
        {
            return MB85RC512T_ERROR_TX;
        }
    }

    return MB85RC512T_OK;
}


#if MB85RC512T_PRINT == 1
/**
 * @brief Print device state.
 * 
 * @param self Device object pointer.
 * @param huart HAL UART handle pointer.
 * @return MB85RC512T_State 
 */
MB85RC512T_State MB85RC512T_print(struct MB85RC512T *self, UART_HandleTypeDef *huart)
{
	if (!self->m_init) return MB85RC512T_ERROR_INIT;

	uint32_t readLen = 8;
	uint8_t printBuff[16];

	for (uint32_t address = 0; address < MB85RC512T_MAX_ADDRESS; address += readLen)
	{
		self->m_data_tx[0] = (uint8_t)(address >> 8);
		self->m_data_tx[1] = (uint8_t)(address & 0xFF);

		if (HAL_I2C_Master_Transmit(self->m_hi2c, self->m_address, self->m_data_tx, 2, self->m_timeout) != HAL_OK)
		{
			return MB85RC512T_ERROR_TX;
		}

		if (HAL_I2C_Master_Receive(self->m_hi2c, self->m_address, self->m_data_rx, readLen, self->m_timeout) != HAL_OK)
		{
			return MB85RC512T_ERROR_RX;
		}

		sprintf((char*)printBuff, "%04X:", (uint16_t)address);
		if (HAL_UART_Transmit(huart, printBuff, strlen((char*)printBuff), MB85RC512T_TIMEOUT_UART) != HAL_OK)
		{
			return MB85RC512T_ERROR_UART;
		}

		for (uint32_t byteCtr = 0; byteCtr < readLen; ++byteCtr)
		{
			sprintf((char*)printBuff, " %02X", self->m_data_rx[byteCtr]);
			if (HAL_UART_Transmit(huart, printBuff, strlen((char*)printBuff), MB85RC512T_TIMEOUT_UART) != HAL_OK)
			{
				return MB85RC512T_ERROR_UART;
			}
		}

		sprintf((char*)printBuff, "\r\n");
		if (HAL_UART_Transmit(huart, printBuff, strlen((char*)printBuff), MB85RC512T_TIMEOUT_UART) != HAL_OK)
		{
			return MB85RC512T_ERROR_UART;
		}
	}

	return MB85RC512T_OK;
}
#endif
