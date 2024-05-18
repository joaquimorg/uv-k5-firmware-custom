/* Copyright 2023 Manuel Jinger
 * Copyright 2023 Dual Tachyon
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

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "bsp/dp32g030/gpio.h"
#include "driver/gpio.h"
#include "driver/keyboard.h"
#include "driver/systick.h"
#include "driver/i2c.h"
#include "misc.h"
#include "applications_task.h"

KEY_Code_t gKeyReading0     = KEY_INVALID;
KEY_Code_t gKeyReading1     = KEY_INVALID;
uint16_t   gDebounceCounter = 0;

StackType_t key_task_stack[configMINIMAL_STACK_SIZE];
StaticTask_t key_task_buffer;

// Define the number of rows and columns of the matrix keyboard
#define ROWS 5
#define COLS 4

static const struct {

	// Using a 16 bit pre-calculated shift and invert is cheaper
	// than using 8 bit and doing shift and invert in code.
	uint16_t set_to_zero_mask;

	// We are very fortunate.
	// The key and pin defines fit together in a single u8, making this very efficient
	struct {
		KEY_Code_t key : 5;
		uint8_t    pin : 3; // Pin 6 is highest
	} pins[4];

} keyboard[] = {

{	// First row
		.set_to_zero_mask = ~(1u << GPIOA_PIN_KEYBOARD_4) & 0xffff,
		.pins = {
			{ .key = KEY_MENU,  .pin = GPIOA_PIN_KEYBOARD_0},
			{ .key = KEY_1,     .pin = GPIOA_PIN_KEYBOARD_1},
			{ .key = KEY_4,     .pin = GPIOA_PIN_KEYBOARD_2},
			{ .key = KEY_7,     .pin = GPIOA_PIN_KEYBOARD_3}
		}
	},
	{	// Second row
		.set_to_zero_mask = ~(1u << GPIOA_PIN_KEYBOARD_5) & 0xffff,
		.pins = {
			{ .key = KEY_UP,    .pin = GPIOA_PIN_KEYBOARD_0},
			{ .key = KEY_2 ,    .pin = GPIOA_PIN_KEYBOARD_1},
			{ .key = KEY_5 ,    .pin = GPIOA_PIN_KEYBOARD_2},
			{ .key = KEY_8 ,    .pin = GPIOA_PIN_KEYBOARD_3}
		}
	},
	{	// Third row
		.set_to_zero_mask = ~(1u << GPIOA_PIN_KEYBOARD_6) & 0xffff,
		.pins = {
			{ .key = KEY_DOWN,  .pin = GPIOA_PIN_KEYBOARD_0},
			{ .key = KEY_3   ,  .pin = GPIOA_PIN_KEYBOARD_1},
			{ .key = KEY_6   ,  .pin = GPIOA_PIN_KEYBOARD_2},
			{ .key = KEY_9   ,  .pin = GPIOA_PIN_KEYBOARD_3}
		}
	},
	{	// Fourth row
		.set_to_zero_mask = ~(1u << GPIOA_PIN_KEYBOARD_7) & 0xffff,
		.pins = {
			{ .key = KEY_EXIT,  .pin = GPIOA_PIN_KEYBOARD_0},
			{ .key = KEY_STAR,  .pin = GPIOA_PIN_KEYBOARD_1},
			{ .key = KEY_0   ,  .pin = GPIOA_PIN_KEYBOARD_2},
			{ .key = KEY_F   ,  .pin = GPIOA_PIN_KEYBOARD_3}
		}
	},
	{	// FN Row
		// Set to zero to handle special case of nothing pulled down
		.set_to_zero_mask = 0xffff,
		.pins = {
			{ .key = KEY_SIDE1,   .pin = GPIOA_PIN_KEYBOARD_0},			
			{ .key = KEY_SIDE2,   .pin = GPIOA_PIN_KEYBOARD_1},
			{ .key = KEY_INVALID, .pin = GPIOA_PIN_KEYBOARD_2},
			{ .key = KEY_PTT,     .pin = GPIOA_PIN_KEYBOARD_3}
		}
	},
};

KEY_Code_t KEYBOARD_Poll(void) {	
	return KEY_INVALID;
}

// Declare local variables to store the current and previous key states
bool key_state[ROWS][COLS] = {0};
//bool stable_key_state[ROWS][COLS] = {0};

bool key_ptt = false;
KEY_State_t prev_key_state[ROWS][COLS] = {0};
KEY_State_t prev_state_ptt = KEY_RELEASED;

TickType_t long_press_timer[ROWS][COLS] = {0};

// Define the long press time in milliseconds
#define LONG_PRESS_TIME 500

bool gWasFKeyPressed  = false;

// Scan the matrix keyboard and detect key events
void keyboard_read() {

	uint16_t regH, regL;

	// KEY_PTT
	key_ptt = !GPIO_CheckBit(&GPIOC->DATA, GPIOC_PIN_PTT);
	if( prev_state_ptt == KEY_PRESSED && key_ptt == false ) {
		app_push_message_key(KEY_PTT, KEY_RELEASED);
		prev_state_ptt = KEY_RELEASED;
		//return;
	} else if( prev_state_ptt == KEY_RELEASED && key_ptt == true ) {
		app_push_message_key(KEY_PTT, KEY_PRESSED);
		prev_state_ptt = KEY_PRESSED;
		//return;
	} else if( prev_state_ptt == KEY_PRESSED && key_ptt == true ) {
		return;
	}

	// Set high for FN Keys
	GPIOA->DATA |=  1u << GPIOA_PIN_KEYBOARD_4 |
					1u << GPIOA_PIN_KEYBOARD_5 |
					1u << GPIOA_PIN_KEYBOARD_6 |
					1u << GPIOA_PIN_KEYBOARD_7;

	GPIOA->DATA &= keyboard[ROWS - 1].set_to_zero_mask;

	for (uint8_t i = 0; i < 8; i++) {
		regL = GPIOA->DATA;
		vTaskDelay(pdMS_TO_TICKS(1));
	}
	
	regL = GPIOA->DATA;
	
	for (uint8_t j = 0; j < 2; j++) {
		const uint16_t mask = 1u << keyboard[ROWS - 1].pins[j].pin;
		key_state[ROWS - 1][j] = (bool)!(regL & mask);
	}

	// Scan each row and column of the matrix keyboard
    for (uint8_t i = 0; i < (ROWS - 1); i++) {

		// Set all high
		GPIOA->DATA |=  1u << GPIOA_PIN_KEYBOARD_4 |
						1u << GPIOA_PIN_KEYBOARD_5 |
						1u << GPIOA_PIN_KEYBOARD_6 |
						1u << GPIOA_PIN_KEYBOARD_7;

		regH = GPIOA->DATA;
		// Clear the pin we are selecting
		GPIOA->DATA &= keyboard[i].set_to_zero_mask;
		for (uint8_t i = 0; i < 8; i++) {
			regL = GPIOA->DATA;
			vTaskDelay(pdMS_TO_TICKS(1));
		}
		regL = GPIOA->DATA;

		for (uint8_t j = 0; j < COLS; j++) {
			const uint16_t mask = 1u << keyboard[i].pins[j].pin;
			if((regH & mask)) {
				key_state[i][j] = (bool)!(regL & mask);
			}
		}

	}

	// Create I2C stop condition since we might have toggled I2C pins
	// This leaves GPIOA_PIN_KEYBOARD_4 and GPIOA_PIN_KEYBOARD_5 high
	I2C_Stop();

	// Reset VOICE pins
	GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_6);
	GPIO_SetBit(  &GPIOA->DATA, GPIOA_PIN_KEYBOARD_7);

}

void keyboard_task() {
	TickType_t current_tick = 0;
	TickType_t elapsed_time = 0;
	
	// Scan each row and column of the matrix keyboard
    for (uint8_t i = 0; i < ROWS; i++) {
      	for (uint8_t j = 0; j < COLS; j++) {

			// Get the current tick count
			current_tick = xTaskGetTickCount();

			// Check if the key is pressed				
			if (key_state[i][j]) {
				
				// Check if the key was previously released
				if (prev_key_state[i][j] == KEY_RELEASED) {
					// Call the callback function with the key pressed state
					if(gWasFKeyPressed) {
						app_push_message_key(keyboard[i].pins[j].key, KEY_PRESSED_WITH_F);
						prev_key_state[i][j] = KEY_PRESSED_WITH_F;
						gWasFKeyPressed = false;
					} else {
						app_push_message_key(keyboard[i].pins[j].key, KEY_PRESSED);
						prev_key_state[i][j] = KEY_PRESSED;
					}

					if ( keyboard[i].pins[j].key == KEY_F ) {
						gWasFKeyPressed = true;
					}

					// Start the long press timer
					long_press_timer[i][j] = current_tick;
					//prev_key_state[i][j] = KEY_PRESSED;

				} else if (prev_key_state[i][j] == KEY_PRESSED) {
					// Check if the long press time has elapsed
					elapsed_time = current_tick - long_press_timer[i][j];
					if (elapsed_time >= pdMS_TO_TICKS(LONG_PRESS_TIME)) {
						// Stop the long press timer
						long_press_timer[i][j] = 0;
						prev_key_state[i][j] = KEY_LONG_PRESSED;
					}
				} else if (prev_key_state[i][j] == KEY_LONG_PRESSED || prev_key_state[i][j] == KEY_LONG_PRESSED_CONT) {
					// Call the callback function with the key long pressed state
					app_push_message_key(keyboard[i].pins[j].key, prev_key_state[i][j]);
					prev_key_state[i][j] = KEY_LONG_PRESSED_CONT;
				}
			}
			else {
				// Check if the key was previously pressed
				if (prev_key_state[i][j] != KEY_RELEASED) {
					
					if (prev_key_state[i][j] != KEY_PRESSED_WITH_F && 
						prev_key_state[i][j] != KEY_LONG_PRESSED && 
						prev_key_state[i][j] != KEY_LONG_PRESSED_CONT
					) {
						// Call the callback function with the key released state
						app_push_message_key(keyboard[i].pins[j].key, KEY_RELEASED);
					}

					if (prev_key_state[i][j] == KEY_LONG_PRESSED_CONT &&
						(keyboard[i].pins[j].key == KEY_UP || keyboard[i].pins[j].key == KEY_DOWN)
					) {
						// Call the callback function with the key released state
						app_push_message_key(keyboard[i].pins[j].key, KEY_RELEASED);
					}


					// Stop the long press timer
					long_press_timer[i][j] = 0;
					prev_key_state[i][j] = KEY_RELEASED;
				}
			}

		}
	}

}


/*-----------------------------------------------------------*/

void key_task(void* arg) {
	(void)arg;

    for (;;) {
		keyboard_read();
        keyboard_task();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}



void keyboard_init() {
    xTaskCreateStatic(
		key_task,
		"KEY",
		ARRAY_SIZE(key_task_stack),
		NULL,
		2 + tskIDLE_PRIORITY,
		key_task_stack,
		&key_task_buffer
	);
}