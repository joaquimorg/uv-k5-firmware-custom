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

#include <stdint.h>
#include <string.h>
#include <stdio.h>     // NULL
#include "debugging.h"

#include "ARMCM0.h"
#include "FreeRTOS.h"
#include "task.h"

#include "bsp/dp32g030/gpio.h"
#include "bsp/dp32g030/syscon.h"
#include "driver/systick.h"

#include "driver/uart.h"

#include "board.h"
#include "task_main.h"


void _putchar(__attribute__((unused)) char c) {
	UART_Send((uint8_t *)&c, 1);
}

void vAssertCalled( unsigned long ulLine, const char * const pcFileName ) {

    taskENTER_CRITICAL();
    {
        
		LogUartf("[ASSERT ERROR] %s %s: line=%lu\r\n", __func__, pcFileName, ulLine);
		
    }
    taskEXIT_CRITICAL();
}

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName ) {  	
  	( void ) pxTask;

	taskENTER_CRITICAL();
    {
        #ifdef ENABLE_UART_DEBUG
			unsigned int stackWm = uxTaskGetStackHighWaterMark(pxTask);
			LogUartf("[STACK ERROR] %s task=%s : %i\r\n", __func__, pcTaskName, stackWm);
		#endif
    }
    taskEXIT_CRITICAL();

}

/*-----------------------------------------------------------*/

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize ) {
	static StaticTask_t xIdleTaskTCB;
	static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize )
{
	static StaticTask_t xTimerTaskTCB;
	static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

/*-----------------------------------------------------------*/


void Main(void)
{
	// Enable clock gating of blocks we need
	SYSCON_DEV_CLK_GATE = 0
		| SYSCON_DEV_CLK_GATE_GPIOA_BITS_ENABLE
		| SYSCON_DEV_CLK_GATE_GPIOB_BITS_ENABLE
		| SYSCON_DEV_CLK_GATE_GPIOC_BITS_ENABLE
		| SYSCON_DEV_CLK_GATE_UART1_BITS_ENABLE
		| SYSCON_DEV_CLK_GATE_SPI0_BITS_ENABLE
		| SYSCON_DEV_CLK_GATE_SARADC_BITS_ENABLE
		| SYSCON_DEV_CLK_GATE_CRC_BITS_ENABLE
		| SYSCON_DEV_CLK_GATE_AES_BITS_ENABLE
		| SYSCON_DEV_CLK_GATE_PWM_PLUS0_BITS_ENABLE;


	SYSTICK_Init();
	BOARD_PORTCON_Init();
	BOARD_GPIO_Init();
	
	main_task_init();

#if(__ARM_ARCH_7A__ == 0U)
    /* Service Call interrupt might be configured before kernel start      */
    /* and when its priority is lower or equal to BASEPRI, svc instruction */
    /* causes a Hard Fault.                                                */
    NVIC_SetPriority(SVCall_IRQn, 0U);
#endif

	vTaskStartScheduler();

    while(1) {
		// oops, something went wrong
  	}

}
