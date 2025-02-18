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

#if MB85RC512T_CMSIS_OS2 == 1
MB85RC512T_State MB85RC512T_init(struct MB85RC512T *self, I2C_HandleTypeDef *hi2c, const uint8_t address, osMutexId_t *mutex_handle)
#else
MB85RC512T_State MB85RC512T_init(struct MB85RC512T *self, I2C_HandleTypeDef *hi2c, const uint8_t address)
#endif
{
	if (address < 0x08 || address > 0x77)
	{
		return MB85RC512T_ERROR_I2C_ADDRESS;
	}

	self->m_address = address << 1;
	self->m_hi2c = hi2c;
	self->m_init = 1;
	self->m_timeout = MB85RC512T_DEFAULT_TIMEOUT;
#if MB85RC512T_CMSIS_OS2 == 1
	self->m_mutex_handle = mutex_handle;
#endif

	return MB85RC512T_OK;
}

MB85RC512T_State MB85RC512T_deinit(struct MB85RC512T *self)
{
	self->m_address = 0;
	self->m_hi2c = NULL;
	self->m_init = 0;
#if MB85RC512T_CMSIS_OS2 == 1
	self->m_mutex_handle = NULL;
#endif

	return MB85RC512T_OK;
}

MB85RC512T_State MB85RC512T_write(struct MB85RC512T *self, const uint32_t address, const uint8_t *data, const size_t len)
{
	if (!self->m_init) return MB85RC512T_ERROR_INIT;
	if (address + len - 1 > MB85RC512T_MAX_ADDRESS) return MB85RC512T_ERROR_ADDRESS;

#if MB85RC512T_CMSIS_OS2 == 1
	if (osMutexAcquire(*self->m_mutex_handle, osWaitForever) != osOK)
	{
		return MB85RC512T_ERROR_MUTEX;
	}
#endif

#if MB85RC512T_INTERRUPT == 1
	uint32_t timeout = HAL_GetTick() + self->m_timeout;
	while (self->m_hi2c->State != HAL_I2C_STATE_READY)
	{
		if (HAL_GetTick() > timeout)
		{
#if MB85RC512T_CMSIS_OS2 == 1
			osMutexRelease(*self->m_mutex_handle);
#endif
			return MB85RC512T_ERROR_TIMEOUT;
		}
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
#if MB85RC512T_CMSIS_OS2 == 1
		osMutexRelease(*self->m_mutex_handle);
#endif
		return MB85RC512T_ERROR_TX;
	}

#if MB85RC512T_CMSIS_OS2 == 1
	if (osMutexRelease(*self->m_mutex_handle) != osOK)
	{
		return MB85RC512T_ERROR_MUTEX;
	}
#endif

	return MB85RC512T_OK;
}

MB85RC512T_State MB85RC512T_read(struct MB85RC512T *self, const uint32_t address, uint8_t *data, const size_t len)
{
	if (!self->m_init) return MB85RC512T_ERROR_INIT;
	if (address + len - 1 > MB85RC512T_MAX_ADDRESS) return MB85RC512T_ERROR_ADDRESS;

#if MB85RC512T_CMSIS_OS2 == 1
	if (osMutexAcquire(*self->m_mutex_handle, osWaitForever) != osOK)
	{
		return MB85RC512T_ERROR_MUTEX;
	}
#endif

#if MB85RC512T_INTERRUPT == 1
	uint32_t timeout = HAL_GetTick() + self->m_timeout;
	while (self->m_hi2c->State != HAL_I2C_STATE_READY)
	{
		if (HAL_GetTick() > timeout)
		{
#if MB85RC512T_CMSIS_OS2 == 1
			osMutexRelease(*self->m_mutex_handle);
#endif
			return MB85RC512T_ERROR_TIMEOUT;
		}
	}
#endif

	self->m_data_tx[0] = (uint8_t)(address >> 8);
	self->m_data_tx[1] = (uint8_t)(address & 0xFF);

	if (HAL_I2C_Master_Transmit(self->m_hi2c, self->m_address, self->m_data_tx, 2, self->m_timeout) != HAL_OK)
	{
#if MB85RC512T_CMSIS_OS2 == 1
		osMutexRelease(*self->m_mutex_handle);
#endif
		return MB85RC512T_ERROR_TX;
	}

	if (HAL_I2C_Master_Receive(self->m_hi2c, self->m_address, self->m_data_rx, len, self->m_timeout) != HAL_OK)
	{
#if MB85RC512T_CMSIS_OS2 == 1
		osMutexRelease(*self->m_mutex_handle);
#endif
		return MB85RC512T_ERROR_RX;
	}

	memcpy(data, self->m_data_rx, len);

#if MB85RC512T_CMSIS_OS2 == 1
	if (osMutexRelease(*self->m_mutex_handle) != osOK)
	{
		return MB85RC512T_ERROR_MUTEX;
	}
#endif

	return MB85RC512T_OK;
}

MB85RC512T_State MB85RC512T_reset(struct MB85RC512T *self, const uint8_t value)
{
    if (!self->m_init) return MB85RC512T_ERROR_INIT;

#if MB85RC512T_CMSIS_OS2 == 1
	if (osMutexAcquire(*self->m_mutex_handle, osWaitForever) != osOK)
	{
		return MB85RC512T_ERROR_MUTEX;
	}
#endif

    uint32_t write_len = MB85RC512T_WRITE_LEN;
    uint32_t page_size = MB85RC512T_WRITE_LEN;

    memset(&self->m_data_tx[2], value, write_len);

    for (uint32_t address = 0; address < MB85RC512T_MAX_ADDRESS; address += write_len)
    {
        uint32_t remaining_in_page = page_size - (address % page_size);
        uint32_t chunk_size = (write_len > remaining_in_page) ? remaining_in_page : write_len;

        self->m_data_tx[0] = (uint8_t)(address >> 8);
        self->m_data_tx[1] = (uint8_t)(address & 0xFF);

        if (HAL_I2C_Master_Transmit(self->m_hi2c, self->m_address, self->m_data_tx, chunk_size + 2, self->m_timeout) != HAL_OK)
        {
#if MB85RC512T_CMSIS_OS2 == 1
        	osMutexRelease(*self->m_mutex_handle);
#endif
            return MB85RC512T_ERROR_TX;
        }
    }

#if MB85RC512T_CMSIS_OS2 == 1
	if (osMutexRelease(*self->m_mutex_handle) != osOK)
	{
		return MB85RC512T_ERROR_MUTEX;
	}
#endif

    return MB85RC512T_OK;
}


#if MB85RC512T_PRINT == 1
MB85RC512T_State MB85RC512T_print(struct MB85RC512T *self, UART_HandleTypeDef *huart)
{
	if (!self->m_init) return MB85RC512T_ERROR_INIT;

#if MB85RC512T_CMSIS_OS2 == 1
	if (osMutexAcquire(*self->m_mutex_handle, osWaitForever) != osOK)
	{
		return MB85RC512T_ERROR_MUTEX;
	}
#endif

	uint32_t read_len = 8;
	uint8_t print_buff[16];

	for (uint32_t address = 0; address < MB85RC512T_MAX_ADDRESS; address += read_len)
	{
		self->m_data_tx[0] = (uint8_t)(address >> 8);
		self->m_data_tx[1] = (uint8_t)(address & 0xFF);

		if (HAL_I2C_Master_Transmit(self->m_hi2c, self->m_address, self->m_data_tx, 2, self->m_timeout) != HAL_OK)
		{
#if MB85RC512T_CMSIS_OS2 == 1
			osMutexRelease(*self->m_mutex_handle);
#endif
			return MB85RC512T_ERROR_TX;
		}

		if (HAL_I2C_Master_Receive(self->m_hi2c, self->m_address, self->m_data_rx, read_len, self->m_timeout) != HAL_OK)
		{
#if MB85RC512T_CMSIS_OS2 == 1
			osMutexRelease(*self->m_mutex_handle);
#endif
			return MB85RC512T_ERROR_RX;
		}

		sprintf((char*)print_buff, "%04X:", (uint16_t)address);
		if (HAL_UART_Transmit(huart, print_buff, strlen((char*)print_buff), MB85RC512T_TIMEOUT_UART) != HAL_OK)
		{
#if MB85RC512T_CMSIS_OS2 == 1
			osMutexRelease(*self->m_mutex_handle);
#endif
			return MB85RC512T_ERROR_UART;
		}

		for (uint32_t byteCtr = 0; byteCtr < read_len; ++byteCtr)
		{
			sprintf((char*)print_buff, " %02X", self->m_data_rx[byteCtr]);
			if (HAL_UART_Transmit(huart, print_buff, strlen((char*)print_buff), MB85RC512T_TIMEOUT_UART) != HAL_OK)
			{
#if MB85RC512T_CMSIS_OS2 == 1
				osMutexRelease(*self->m_mutex_handle);
#endif
				return MB85RC512T_ERROR_UART;
			}
		}

		sprintf((char*)print_buff, "\r\n");
		if (HAL_UART_Transmit(huart, print_buff, strlen((char*)print_buff), MB85RC512T_TIMEOUT_UART) != HAL_OK)
		{
#if MB85RC512T_CMSIS_OS2 == 1
			osMutexRelease(*self->m_mutex_handle);
#endif
			return MB85RC512T_ERROR_UART;
		}
	}

#if MB85RC512T_CMSIS_OS2 == 1
	if (osMutexRelease(*self->m_mutex_handle) != osOK)
	{
		return MB85RC512T_ERROR_MUTEX;
	}
#endif

	return MB85RC512T_OK;
}
#endif
