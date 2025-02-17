/*
 * MB85RC512T.c
 *
 *  Created on: Feb 17, 2025
 *      Author: knap-linux
 */

#include "MB85RC512T.h"

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

MB85RC512T_State MB85RC512T_write8(MB85RC512T *device, const uint8_t data)
{
	return MB85RC512T_OK;
}

MB85RC512T_State MB85RC512T_read8(MB85RC512T *device, uint8_t *data)
{
	return MB85RC512T_OK;
}

#if MB85RC512T_TEST == 1

#endif
