/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#include <stddef.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "driver/eeprom.h"
#include "driver/i2c.h"
#include "driver/system.h"

void EEPROM_ReadBuffer(uint32_t address, void *pBuffer, uint16_t size) {
	taskENTER_CRITICAL();
	/*I2C_Start();

	I2C_Write(0xA0);

	I2C_Write((uint8_t)((Address >> 8) & 0xFF));
	I2C_Write((uint8_t)((Address >> 0) & 0xFF));

	I2C_Start();

	I2C_Write(0xA1);

	I2C_ReadBuffer(pBuffer, Size);

	I2C_Stop();*/

	uint8_t IIC_ADD = (uint8_t)(0xA0 | ((address / 0x10000) << 1));

	I2C_Start();
	I2C_Write(IIC_ADD);
	I2C_Write((uint8_t)((address >> 8) & 0xFF));
	I2C_Write((uint8_t)(address & 0xFF));
	I2C_Start();
	I2C_Write(IIC_ADD + 1);
	I2C_ReadBuffer(pBuffer, (uint8_t)size);
	I2C_Stop();

	taskEXIT_CRITICAL();
}

static uint8_t tmpBuffer[128];
void EEPROM_WriteBuffer(uint32_t address, void* pBuffer, uint16_t size) {
	/*if (pBuffer == NULL || Address >= 0x2000)
		return;


	uint8_t buffer[8];
	EEPROM_ReadBuffer(Address, buffer, 8);
	if (memcmp(pBuffer, buffer, 8) == 0) {
		return;
	}

	taskENTER_CRITICAL();
	I2C_Start();
	I2C_Write(0xA0);
	I2C_Write((uint8_t)((Address >> 8) & 0xFF));
	I2C_Write((uint8_t)((Address >> 0) & 0xFF));
	I2C_WriteBuffer(pBuffer, 8);
	I2C_Stop();

	// give the EEPROM time to burn the data in (apparently takes 5ms)
	SYSTEM_DelayMs(8);
	taskEXIT_CRITICAL();*/

	if (pBuffer == NULL) {
		return;
	}
	const uint8_t PAGE_SIZE = 32;

	taskENTER_CRITICAL();
	while (size) {
		uint16_t i = address % PAGE_SIZE;
		uint16_t rest = PAGE_SIZE - i;
		uint16_t n = size < rest ? size : rest;

		EEPROM_ReadBuffer(address, tmpBuffer, n);
		if (memcmp(pBuffer, tmpBuffer, n) != 0) {
			uint8_t IIC_ADD = (uint8_t)(0xA0 | ((address / 0x10000) << 1));

			I2C_Start();
			I2C_Write(IIC_ADD);
			I2C_Write((uint8_t)((address >> 8) & 0xFF));
			I2C_Write((uint8_t)(address & 0xFF));

			I2C_WriteBuffer(pBuffer, (uint8_t)n);

			I2C_Stop();
			SYSTEM_DelayMs(10);
		}

		pBuffer += n;
		address += n;
		size -= n;
	}

	taskEXIT_CRITICAL();

}
