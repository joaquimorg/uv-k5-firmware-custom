/* Copyright 2024 joaquim.org
 * https://github.com/joaquimorg
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

#include "misc.h"
#include "settings.h"
#include "driver/bk4819.h"
#include "functions.h"
#include "radio.h"
#include "app.h"
#include "audio.h"
#include "task_main.h"

#include "debugging.h"


void VFO_Up_Down(int8_t Direction) {
	uint8_t Channel = gScreenChannel[gSettings.activeVFO];

    if (IS_FREQ_CHANNEL(Channel)) { // step/down in frequency
        const uint32_t frequency = APP_SetFrequencyByStep(gTxVfo, Direction);

        if (RX_freq_check(frequency) < 0) { // frequency not allowed
            //gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
            main_push_message_value(MAIN_MSG_PLAY_BEEP, BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL);
            return;
        }
        gTxVfo->freq_config_RX.Frequency = frequency;
        gTxVfo->freq_config_TX.Frequency = frequency;

        BK4819_SetFrequency(frequency);
        BK4819_RX_TurnOn();
        //gRequestSaveChannel = 1;
        main_push_message(RADIO_SAVE_CHANNEL);
        //main_push_message(RADIO_VFO_CONFIGURE);        
        //LogUartf("freq up/down. %u \r\n", gTxVfo->freq_config_RX.Frequency);
        return;
    }

    const uint8_t Next = RADIO_FindNextChannel((uint8_t)(Channel + Direction), Direction, false, 0);
    if (Next == 0xFF)
        return;
    if (Channel == Next)
        return;
    gMrChannel[gSettings.activeVFO] = Next;
    gScreenChannel[gSettings.activeVFO] = Next;
    main_push_message(RADIO_SAVE_VFO);
    //main_push_message(RADIO_VFO_CONFIGURE_RELOAD);
    //LogUartf("channel up/down. %u \r\n", Next);
}