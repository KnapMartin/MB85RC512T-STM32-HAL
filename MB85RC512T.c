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

MB85RC512T_State MB85RC512T_init(MB85RC512T *device, I2C_HandleTypeDef *hi2c, const uint8_t address)
{
	device->m_address = address << 1;
	device->m_hi2c = hi2c;
	device->m_init = 1;

	return MB85RC512T_OK;
}

MB85RC512T_State MB85RC512T_deinit(MB85RC512T *device)
{
	device->m_address = 0;
	device->m_hi2c = NULL;
	device->m_init = 0;

	return MB85RC512T_OK;
}

MB85RC512T_State MB85RC512T_write(MB85RC512T *device, const uint32_t address, const uint8_t *data, const size_t len)
{
	if (!device->m_init) return MB85RC512T_ERROR_INIT;
	if (address + len - 1 > MB85RC512T_MAX_ADDRESS) return MB85RC512T_ERROR_ADDRESS;

	device->m_data_tx[0] = (uint8_t)(address >> 8);
	device->m_data_tx[1] = (uint8_t)(address & 0xFF);

	for (uint32_t ctr = 0; ctr < len; ++ctr)
	{
		device->m_data_tx[ctr + 2] = data[ctr];
	}

	if (HAL_I2C_Master_Transmit(device->m_hi2c, device->m_address, device->m_data_tx, len + 2, MB85RC512T_TIMEOUT) != HAL_OK)
	{
		return MB85RC512T_ERROR_TX;
	}

	return MB85RC512T_OK;
}

MB85RC512T_State MB85RC512T_read(MB85RC512T *device, const uint32_t address, uint8_t *data, const size_t len)
{
	if (!device->m_init) return MB85RC512T_ERROR_INIT;
	if (address + len - 1 > MB85RC512T_MAX_ADDRESS) return MB85RC512T_ERROR_ADDRESS;

	device->m_data_tx[0] = (uint8_t)(address >> 8);
	device->m_data_tx[1] = (uint8_t)(address & 0xFF);

	if (HAL_I2C_Master_Transmit(device->m_hi2c, device->m_address, device->m_data_tx, 2, MB85RC512T_TIMEOUT) != HAL_OK)
	{
		return MB85RC512T_ERROR_TX;
	}

	if (HAL_I2C_Master_Receive(device->m_hi2c, device->m_address, device->m_data_rx, len, MB85RC512T_TIMEOUT) != HAL_OK)
	{
		return MB85RC512T_ERROR_RX;
	}

	memcpy(data, device->m_data_rx, len);

	return MB85RC512T_OK;
}

MB85RC512T_State MB85RC512T_reset(MB85RC512T *device, const uint8_t value)
{
	if (!device->m_init) return MB85RC512T_ERROR_INIT;

	uint32_t writeLen = MB85RC512T_WRITE_LEN;

	for (uint32_t byteCtr = 0; byteCtr < writeLen; ++byteCtr)
	{
		device->m_data_tx[byteCtr + 2] = value;
	}

	for (uint32_t address = 0; address < MB85RC512T_MAX_ADDRESS; address += writeLen)
	{
		device->m_data_tx[0] = (uint8_t)(address >> 8);
		device->m_data_tx[1] = (uint8_t)(address & 0xFF);

		if (HAL_I2C_Master_Transmit(device->m_hi2c, device->m_address, device->m_data_tx, writeLen + 2, MB85RC512T_TIMEOUT) != HAL_OK)
		{
			return MB85RC512T_ERROR_TX;
		}
	}

	return MB85RC512T_OK;
}

#if MB85RC512T_PRINT == 1
MB85RC512T_State MB85RC512T_print(MB85RC512T *device, UART_HandleTypeDef *huart)
{
	if (!device->m_init) return MB85RC512T_ERROR_INIT;

	uint32_t readLen = 8;
	uint8_t printBuff[16];

	for (uint32_t address = 0; address < MB85RC512T_MAX_ADDRESS; address += readLen)
	{
		device->m_data_tx[0] = (uint8_t)(address >> 8);
		device->m_data_tx[1] = (uint8_t)(address & 0xFF);

		if (HAL_I2C_Master_Transmit(device->m_hi2c, device->m_address, device->m_data_tx, 2, MB85RC512T_TIMEOUT) != HAL_OK)
		{
			return MB85RC512T_ERROR_TX;
		}

		if (HAL_I2C_Master_Receive(device->m_hi2c, device->m_address, device->m_data_rx, readLen, MB85RC512T_TIMEOUT) != HAL_OK)
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
			sprintf((char*)printBuff, " %02X", device->m_data_rx[byteCtr]);
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
