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


#include "apps.h"
#include "misc.h"
#include "settings.h"
#include "driver/bk4819.h"
#include "functions.h"
#include "radio.h"
#include "menu.h"
#include "app.h"

#include "gui/gui.h"
#include "ui.h"

#include "driver/uart.h"

uint8_t popupListSelected = 0;
uint8_t popupListSize = 0;

const int8_t dBmCorrTable[7] = {
			-15, // band 1
			-25, // band 2
			-20, // band 3
			-4, // band 4
			-7, // band 5
			-6, // band 6
			 -1  // band 7
		};

const char *VfoStateStr[] = {
       [VFO_STATE_NORMAL]="",
       [VFO_STATE_TX]="TX",
       [VFO_STATE_BUSY]="BUSY",
       [VFO_STATE_BAT_LOW]="BAT LOW",
       [VFO_STATE_TX_DISABLE]="TX DISABLE",
       [VFO_STATE_TIMEOUT]="TIMEOUT",
       [VFO_STATE_ALARM]="ALARM",
       [VFO_STATE_VOLTAGE_HIGH]="VOLT HIGH"
};

void MainVFO_showRSSI(void) {

    const uint8_t xPosVFO = 14;
    const uint8_t yPosVFO = 39;

    // 0x26 '&' RSSI Empty
    // 0x3F '?' RSSI Sep
    // 0x40 '@' RSSI Box

    if(FUNCTION_IsRx()) {
		UI_printf(&font_10, TEXT_ALIGN_LEFT, xPosVFO - 4, 0, yPosVFO + 3, true, true, "$");
	}

    UI_printf(&font_10, TEXT_ALIGN_LEFT, xPosVFO + 8, 0, yPosVFO + 3, false, false, "S");
    UI_printf(&font_small, TEXT_ALIGN_LEFT, xPosVFO + 16, 0, yPosVFO, false, false, "1?3?5?7?9?20?60?90");

    uint8_t bar[19];
    memset(bar, '&', sizeof(bar));
    bar[18] = 0x00;

    if ( FUNCTION_IsRx() ) {

        const int16_t s0_dBm   = 0;//-gSettings.S0_LEVEL;                  // S0 .. base level
	    const int16_t rssi_dBm = BK4819_GetRSSI_dBm() + dBmCorrTable[gRxVfo->Band];

        int s0_9 = 255;//gSettings.S0_LEVEL - gSettings.S9_LEVEL;
        const uint8_t s_level = (uint8_t)MIN(MAX((int32_t)(rssi_dBm - s0_dBm)*100 / (s0_9*100/9), 0), 9); // S0 - S9
        uint8_t overS9dBm = (uint8_t)MIN(MAX(rssi_dBm + 255/*gSettings.S9_LEVEL*/, 0), 99);
        uint8_t overS9Bars = (uint8_t)MIN(overS9dBm/10, 9);

        memset(bar, '@', s_level + overS9Bars);

        if(overS9Bars != 0) {
            UI_printf(&font_small, TEXT_ALIGN_LEFT, xPosVFO + 89, 0, yPosVFO + 4, false, false, "S9+%2d", overS9dBm);
        }
    }

    UI_printf(&font_small, TEXT_ALIGN_LEFT, xPosVFO + 16, 0, yPosVFO + 6, false, false, "%s", bar);

}

unsigned int sqrt16(unsigned int value)
{	// return square root of 'value'
	unsigned int shift = 16;         // number of bits supplied in 'value' .. 2 ~ 32
	unsigned int bit   = 1u << --shift;
	unsigned int sqrti = 0;
	while (bit)
	{
		const unsigned int temp = ((sqrti << 1) | bit) << shift--;
		if (value >= temp) {
			value -= temp;
			sqrti |= bit;
		}
		bit >>= 1;
	}
	return sqrti;
}

void MainVFO_showMICBar(void) {

    const uint8_t xPosVFO = 14;
    const uint8_t yPosVFO = 39;

    // 0x26 '&' RSSI Empty
    // 0x3F '?' RSSI Sep
    // 0x40 '@' RSSI Box

    UI_printf(&font_10, TEXT_ALIGN_LEFT, xPosVFO - 4, 0, yPosVFO + 3, true, true, "&");

    UI_printf(&font_10, TEXT_ALIGN_LEFT, xPosVFO + 8, 0, yPosVFO + 3, false, false, "M");
    UI_printf(&font_small, TEXT_ALIGN_LEFT, xPosVFO + 16, 0, yPosVFO, false, false, "1?3?5?7?9?10");

    uint8_t bar[13];
    memset(bar, '&', sizeof(bar));
    bar[12] = 0x00;

    // make non-linear to make more sensitive at low values
    const unsigned int level = MIN((unsigned int)(BK4819_GetVoiceAmplitudeOut() * 8), 65535u);
    const unsigned int sqrt_level = MIN(sqrt16(level), 124u);
    uint8_t bars = (uint8_t)(11 * sqrt_level / 124);
    bars = (uint8_t)MIN(bars, 11);
    memset(bar, '@', bars);

    UI_printf(&font_small, TEXT_ALIGN_LEFT, xPosVFO + 16, 0, yPosVFO + 6, false, false, "%s", bar);

}

void MainVFO_showCTCSS(const char *tx_rx, uint8_t code_type, uint8_t code_value, uint8_t ypos) {

    if ( code_type == CODE_TYPE_CONTINUOUS_TONE ) {
        UI_printf(&font_small, TEXT_ALIGN_LEFT, UI_nextX + 4, 0, ypos, true, true, "%s T %u.%u", tx_rx, CTCSS_Options[code_value] / 10, CTCSS_Options[code_value] % 10);
    } else if (code_type == CODE_TYPE_REVERSE_DIGITAL) {
        UI_printf(&font_small, TEXT_ALIGN_LEFT, UI_nextX + 4, 0, ypos, true, true, "%s D %03oI", tx_rx, DCS_Options[code_value]);
    } else if (code_type == CODE_TYPE_DIGITAL) {
        UI_printf(&font_small, TEXT_ALIGN_LEFT, UI_nextX + 4, 0, ypos, true, true, "%s D %03oN", tx_rx, DCS_Options[code_value]);
    }
}

uint8_t StatusLine = 0;

typedef struct {
    char name[20];
    uint32_t lower_freq;
    uint32_t upper_freq;
} FrequencyBand;

FrequencyBand bands[] = {
    {"HAM 17m", 1806800, 1816800},
    {"HAM 15m", 2100000, 2145000},
    {"HAM 12m", 2489000, 2499000},
    {"HAM 10m", 2800000, 2970000},
    {"HAM 6m", 5000000, 5400000},
    {"HAM 4m (Europe)", 7000000, 7100000},
    {"HAM 2m", 14400000, 14800000},
    {"HAM 1.25m", 21900000, 22500000},
    {"HAM 70cm", 42000000, 45000000},
    {"HAM 33cm", 90200000, 92800000},
    {"HAM 23cm", 124000000, 130000000},
    // Other Bands
    {"CB Radio", 2696500, 2740500}, // 11m Citizens Band
    {"FM Broadcast", 8800000, 10800000},
    {"Aircraft Band", 10800000, 13700000},
    {"Marine VHF", 15600000, 17400000},
    {"Weather Band", 16240000, 16255000},
    {"Public Service Band", 15000000, 17400000},
    {"Industrial/Business", 46100000, 47000000},
    {"GMRS/FRS", 46255000, 46772500},

    // PMR (Private Mobile Radio)
    {"PMR446", 44600000, 44620000}, // PMR 446

};

const char* getBandName(uint32_t frequency) {
    int num_bands = sizeof(bands) / sizeof(bands[0]);

    for (int i = 0; i < num_bands; ++i) {
        if (frequency >= bands[i].lower_freq && frequency <= bands[i].upper_freq) {
            return bands[i].name;
        }
    }

    return "";
}

// this must be in other place, radio specific
// todo
RegisterSpec registerRadio[] = {
    {"ACG",     BK4819_REG_7E, 0, 0,      0},
    {"LNAs",    BK4819_REG_13, 8, 0b11,   1},
    {"LNA",     BK4819_REG_13, 5, 0b111,  1},
    {"PGA",     BK4819_REG_13, 0, 0b111,  1},
    {"MIX",     BK4819_REG_13, 3, 0b11,   1},

    {"IF",      BK4819_REG_3D, 0, 0xFFFF, 0x2aaa},
    {"DEV",     BK4819_REG_40, 0, 0xFFF,  10},
    {"CMP",     BK4819_REG_31, 3, 1,      1},
    {"MIC",     BK4819_REG_7D, 0, 0xF,    1},
};

uint16_t GetRegRadioValue(uint8_t st) {
    RegisterSpec s = registerRadio[st];
    if(s.num == BK4819_REG_7E) {
	    return (BK4819_ReadRegister(s.num) & (1 << 15)) == 0;
    } else {
        return (uint16_t)((BK4819_ReadRegister(s.num) >> s.offset) & s.mask);
    }
}

void LockAGC( bool lockAGC ) {
  RADIO_SetupAGC(gVfoInfo[gSettings.activeVFO].Modulation == MODULATION_AM, lockAGC);
}

void SetRegRadioValue(uint8_t st, bool add) {  
    RegisterSpec s = registerRadio[st];    

    if(s.num == BK4819_REG_7E) {
        LockAGC(add);
    } else {        

        uint16_t v = GetRegRadioValue(st);
        
        uint16_t reg = BK4819_ReadRegister(s.num);
        if (add && v <= s.mask - s.inc) {
            v += s.inc;
        } else if (!add && v >= 0 + s.inc) {
            v -= s.inc;
        }
        // TODO: use max value for bits count in max value, or reset by additional
        // mask in spec
        reg &= (uint16_t)~(s.mask << s.offset);
        BK4819_WriteRegister(s.num, (uint16_t)(reg | (v << s.offset)));
    }
}

// end todo

void MainVFO_showVFO(void) {

    char String[17] = { 0 };
    
    const VFO_Info_t *vfoInfoA = &gVfoInfo[gSettings.activeVFO];

    const enum VfoState_t state = gVfoState;

    uint32_t frequency;
    uint8_t  yPosVFO = 23;

    UI_fillRect(0, 7, 128, 38, true);

    // VFO
    //UI_printf(&font_10, TEXT_ALIGN_LEFT, 3, 0, yPosVFO - 7, true, true, gSettings.activeVFO == 0 ? "VFO A" : "VFO B");

    if (state == VFO_STATE_TX){
        frequency = vfoInfoA->pTX->Frequency;
    } else {
        frequency = vfoInfoA->pRX->Frequency;
    }

    if ( IS_MR_CHANNEL(gScreenChannel[gSettings.activeVFO]) ) {
        // Channel Name
        memcpy(String, vfoInfoA->Name, 16);
        if (String[0] == 0) {
            UI_printf(&font_10, TEXT_ALIGN_LEFT, 3, 0, yPosVFO - 7, true, true, "CH-%03u", gScreenChannel[gSettings.activeVFO] + 1);
        } else {
            UI_printf(&font_10, TEXT_ALIGN_LEFT, 3, 0, yPosVFO - 7, true, true, "M%03u", gScreenChannel[gSettings.activeVFO] + 1);
            UI_printf(&font_10, TEXT_ALIGN_RIGHT, UI_nextX + 4, 124, yPosVFO - 8, false, false, String);
        }
    } else {
        UI_printf(&font_10, TEXT_ALIGN_LEFT, 3, 0, yPosVFO - 8, false, false, "%s", getBandName(frequency));
    }

   UI_printf(&font_n_20, TEXT_ALIGN_LEFT, 2, 0, yPosVFO + 9, false, false, gSettings.activeVFO == 0 ? "/" : "*");

    // Frequency

    if ( frequency >= _1GHz_in_KHz ) {
        UI_printf(&font_n_20, TEXT_ALIGN_RIGHT, 15, 110, yPosVFO + 10, false, false, "%1u.%03u.%03u", (frequency / 100000000), (frequency / 100000) % 1000, (frequency % 100000) / 100);
    } else if ( frequency >= 10000000 ) {
        UI_printf(&font_n_20, TEXT_ALIGN_RIGHT, 15, 110, yPosVFO + 10, false, false, "%3u.%03u", (frequency / 100000), (frequency % 100000) / 100);
    } else {
        UI_printf(&font_n_20, TEXT_ALIGN_RIGHT, 15, 110, yPosVFO + 10, false, false, "%2u.%03u", (frequency / 100000), (frequency % 100000) / 100);
    }
    UI_printf(&font_n_16,   TEXT_ALIGN_LEFT, UI_nextX + 2, 0, yPosVFO + 10, false, false, "%02u", (frequency % 100));

    if (state != VFO_STATE_TX && !FUNCTION_IsRx()){
        if (vfoInfoA->freq_config_RX.Frequency != vfoInfoA->freq_config_TX.Frequency) {
            // show the TX offset symbol
            if(vfoInfoA->TX_OFFSET_FREQUENCY_DIRECTION <=2 && vfoInfoA->TX_OFFSET_FREQUENCY_DIRECTION > 0) {
                UI_printf(&font_n_20, TEXT_ALIGN_LEFT, 2, 0, yPosVFO + 22, false, false, "%s", gSubMenu_SFT_D[vfoInfoA->TX_OFFSET_FREQUENCY_DIRECTION]);
            }
        }
    }

    if (state == VFO_STATE_TX){
        MainVFO_showMICBar();
    } else {
        MainVFO_showRSSI();
    }

    yPosVFO = 51;

    UI_fillRectWithChessboard(0, yPosVFO - 6, 128, 19, false);

    //UI_fillRect(0, yPosVFO - 6, 128, 19, true);

    // Modulation A
    UI_printf(&font_small, TEXT_ALIGN_LEFT, 3, 0, yPosVFO, true, true, gModulationStr[vfoInfoA->Modulation]);
    // BANDWIDTH A
    UI_printf(&font_small, TEXT_ALIGN_LEFT, UI_nextX + 4, 0, yPosVFO, true, true, gSubMenu_W_N[vfoInfoA->CHANNEL_BANDWIDTH]);
    // OUTPUT_POWER
    UI_printf(&font_small, TEXT_ALIGN_LEFT, UI_nextX + 4, 0, yPosVFO, true, true, gSubMenu_TXP[vfoInfoA->OUTPUT_POWER % 3]);

    if ( vfoInfoA->Modulation == MODULATION_FM ) {
        // DCS/CT/DCR A
        if ( vfoInfoA->pRX->CodeType > 0 ) {
            MainVFO_showCTCSS("$", vfoInfoA->pRX->CodeType, vfoInfoA->pRX->Code, yPosVFO);
        }
        if ( vfoInfoA->pTX->CodeType > 0 ) {
            MainVFO_showCTCSS("\"", vfoInfoA->pTX->CodeType, vfoInfoA->pTX->Code, yPosVFO);
        }
    }

    UI_nextX = 0;

    if (StatusLine == 0) {
        for (uint8_t i = 0; i < 5; ++i) {
            UI_printf(&font_small, TEXT_ALIGN_LEFT, (uint8_t)(UI_nextX + ( i == 0 ? 3 : 4 )), 0, 60, true, true, "%s %u", registerRadio[i].name, GetRegRadioValue(i));
        }
    } else {    
        for (uint8_t i = 0; i < 4; ++i) {
            UI_printf(&font_small, TEXT_ALIGN_LEFT, (uint8_t)(UI_nextX + ( i == 0 ? 3 : 4 )), 0, 60, true, true, "%s %u", registerRadio[i + 5].name, GetRegRadioValue(i + 5));
        }
    }

    if ( application_getPopup() != APP_POPUP_INFO ) {

        if (state != VFO_STATE_NORMAL && state != VFO_STATE_TX) {
            if (state < ARRAY_SIZE(VfoStateStr)) {
                application_showPopup(APP_POPUP_INFO, true);
                //LogUartf("VFO_STATE %i\r\n", state);
            }
        }
    }
}

void MainVFO_timeoutHandlerFunction() {
    if (StatusLine == 0) {
        StatusLine = 1;
    } else {
        StatusLine = 0;
    }
}

void MainVFO_renderFunction() {

    MainVFO_showVFO();

    if (GUI_inputNotEmpty()) {
        if (IS_MR_CHANNEL(gScreenChannel[gSettings.activeVFO])) {
            GUI_inputShow("Input Memory", "M ");
        } else {
            GUI_inputShowFreq("Input Freq.");
        }
    }
}


void MainVFO_keyHandlerFunction(KEY_Code_t key, KEY_State_t state) {
    if (!GUI_inputNotEmpty()) {
        switch (key) {
            case KEY_1:
                if ( state == KEY_PRESSED_WITH_F || state == KEY_LONG_PRESSED ) {
                }
                break;
            case KEY_2:
                if ( state == KEY_PRESSED_WITH_F || state == KEY_LONG_PRESSED ) {
                    main_push_message(RADIO_VFO_SWITCH);
                }
                break;
            case KEY_3:
                if ( state == KEY_PRESSED_WITH_F || state == KEY_LONG_PRESSED ) {
                    main_push_message(RADIO_VFO_SWITCH_MODE);
                }
                break;
            case KEY_4:
                if ( state == KEY_PRESSED_WITH_F || state == KEY_LONG_PRESSED ) {
                    popupListSelected = gTxVfo->CHANNEL_BANDWIDTH;
                    application_showPopup(APP_POPUP_W_N, true);
                }
                break;
            case KEY_5:
                if ( state == KEY_PRESSED_WITH_F || state == KEY_LONG_PRESSED ) {
                    popupListSelected = gTxVfo->Modulation;
                    application_showPopup(APP_POPUP_AM, true);
                }
                break;
            case KEY_6:
                if ( state == KEY_PRESSED_WITH_F || state == KEY_LONG_PRESSED ) {
                    popupListSelected = gTxVfo->OUTPUT_POWER;
                    application_showPopup(APP_POPUP_TXP, true);
                }
                break;

            case KEY_8:
                if ( state == KEY_PRESSED_WITH_F || state == KEY_LONG_PRESSED ) {
                    popupListSelected = 0;
                    application_showPopup(APP_POPUP_REG, true);
                }
                break;

            case KEY_STAR:
                if ( state == KEY_PRESSED ) {
                }
                break;

            case KEY_UP:
            case KEY_DOWN:
                if ( state == KEY_PRESSED || state == KEY_LONG_PRESSED_CONT ) {
                    main_push_message(key == KEY_UP ? RADIO_VFO_UP : RADIO_VFO_DOWN);
                } else if (state == KEY_RELEASED ) {
                    // save if key released
                    if (IS_FREQ_CHANNEL(gScreenChannel[gSettings.activeVFO])) {
                        main_push_message(RADIO_SAVE_CHANNEL);
                    } else {
                        main_push_message(RADIO_SAVE_VFO);
                        main_push_message(RADIO_VFO_CONFIGURE_RELOAD);
                        main_push_message(RADIO_RECONFIGURE_VFO);
                    }

                }
                break;

            case KEY_MENU:
                if ( state == KEY_PRESSED_WITH_F || state == KEY_LONG_PRESSED ) {
                    load_application(APP_MENU);
                }
                break;

            default:
                break;
        }
    }

    if ( state == KEY_RELEASED ) {
        switch (key) {
            case KEY_1:
            case KEY_2:
            case KEY_3:
            case KEY_4:
            case KEY_5:
            case KEY_6:
            case KEY_7:
            case KEY_8:
            case KEY_9:
            case KEY_0:
            case KEY_STAR:
            case KEY_F:
                if (IS_MR_CHANNEL(gScreenChannel[gSettings.activeVFO])) {
                    GUI_inputAppendKey(key, 3, false);
                } else {
                    GUI_inputAppendKey(key, 9, true);
                }
            break;
            case KEY_MENU:
                if (GUI_inputGetSize() > 0) {
                    if (IS_MR_CHANNEL(gScreenChannel[gSettings.activeVFO])) {
                        const uint16_t selChannel = (uint16_t)GUI_inputGetNumberClear() - 1;
                        main_push_message_value(RADIO_SET_CHANNEL, selChannel);
                    } else {
                        if (GUI_inputGetSize() > 6) {
                            const uint32_t selFreq = GUI_inputGetNumberClear();
                            main_push_message_value(RADIO_SET_FREQ, selFreq);
                        } else {
                            main_push_message_value(MAIN_MSG_PLAY_BEEP, BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL);
                        }
                    }
                } else {
                    load_application(APP_VFO_SETTINGS);
                }
                break;
            case KEY_EXIT:
                if (GUI_inputGetSize() > 0) {
                    GUI_inputReset();
                }
                break;

            default:
                break;
        }
    }
}

/* --------------------------------------------------------------------------- */
// Popups
/* --------------------------------------------------------------------------- */

void MainVFO_renderPopupFunction(APPS_Popup_t popup) {

    const uint8_t popupW = 70;
	const uint8_t popupH = 42;

    uint8_t startX;
    uint8_t startY;

    switch (popup) {
        case APP_POPUP_TXP:
            GUI_showPopup(popupW, popupH, &startX, &startY);
            UI_printf(&font_small, TEXT_ALIGN_CENTER, startX, startX + popupW - 2, startY, true, false, "VFO %s - TX POWER", gSettings.activeVFO == 0 ? "A" : "B");
            popupListSize = ARRAY_SIZE(gSubMenu_TXP) - 1;
            popupShowList(popupListSelected, popupListSize, startX, startY + 8, startX + popupW - 2, gSubMenu_TXP);
            break;
        case APP_POPUP_W_N:
            GUI_showPopup(popupW, popupH, &startX, &startY);
            UI_printf(&font_small, TEXT_ALIGN_CENTER, startX, startX + popupW - 2, startY, true, false, "VFO %s - BANDWIDTH", gSettings.activeVFO == 0 ? "A" : "B");
            popupListSize = ARRAY_SIZE(gSubMenu_W_N) - 1;
            popupShowList(popupListSelected, popupListSize, startX, startY + 8, startX + popupW - 2, gSubMenu_W_N);
            break;
        case APP_POPUP_AM:
            GUI_showPopup(popupW, popupH, &startX, &startY);
            UI_printf(&font_small, TEXT_ALIGN_CENTER, startX, startX + popupW - 2, startY, true, false, "VFO %s - MODULATION", gSettings.activeVFO == 0 ? "A" : "B");
            popupListSize = MODULATION_UKNOWN - 1;
            popupShowList(popupListSelected, popupListSize, startX, startY + 8, startX + popupW - 2, gModulationStr);
            break;
        case APP_POPUP_INFO:
            GUI_showPopup(popupW + 10, popupH - 10, &startX, &startY);
            UI_drawString(&font_10, TEXT_ALIGN_CENTER, startX, startX + (popupW + 10) - 2, startY + 10, VfoStateStr[gVfoState], true, false);
            break;
        case APP_POPUP_REG:
            GUI_showPopup(popupW + 20, popupH, &startX, &startY);
            UI_printf(&font_small, TEXT_ALIGN_CENTER, startX, startX + popupW + 20, startY, true, false, "Register");

            popupListSize = 8;

            uint8_t yPos = startY + 10;
            uint8_t listCount = 3;
            uint8_t offset = 0;
            popupListCalc(popupListSelected, popupListSize, &yPos, &listCount, &offset);
            for (uint8_t i = 0; i < listCount; i++) {
                bool isFill = popupListSelected == i + offset ? true : false;
                UI_printf(&font_10, TEXT_ALIGN_CENTER, startX, startX + 45, yPos, !isFill, isFill, "%s", registerRadio[i + offset].name);
                UI_printf(&font_10, TEXT_ALIGN_CENTER, startX + 45, startX + popupW + 20, yPos, !isFill, isFill, "%u", GetRegRadioValue(i + offset));
                yPos += 11;
            }

            /*for (uint8_t i = 0; i < popupListSize; ++i) {
                if ( popupListSelected == i ) {
                    UI_printf(&font_10, TEXT_ALIGN_LEFT, startX + 3, startX + popupW + 16, startY + ( i * 10 ), false, true, "%s %u", registerRadio[i].name, GetRegRadioValue(i));
                } else {
                    UI_printf(&font_10, TEXT_ALIGN_LEFT, startX + 3, startX + popupW + 16, startY + ( i * 10 ), true, false, "%s %u", registerRadio[i].name, GetRegRadioValue(i));
                }
            }*/

            break;
        default:
            break;
    }
}

void MainVFO_popupSave(APPS_Popup_t popup) {
    switch (popup) {
        case APP_POPUP_TXP:
            gTxVfo->OUTPUT_POWER = popupListSelected;
            main_push_message(RADIO_SAVE_CHANNEL);
            break;
        case APP_POPUP_W_N:
            gTxVfo->CHANNEL_BANDWIDTH = popupListSelected;
            main_push_message(RADIO_SAVE_CHANNEL);
            break;
        case APP_POPUP_AM:
            gTxVfo->Modulation = popupListSelected;
            main_push_message(RADIO_SAVE_CHANNEL);
            break;
        default:
            break;
    }
}

void MainVFO_keyHandlerPopupFunction(KEY_Code_t key, KEY_State_t state, APPS_Popup_t popup) {
    if ( popup == APP_POPUP_INFO ) {
        return;
    }
    if ( state == KEY_PRESSED) {
        if ( popup == APP_POPUP_REG ) {
            switch (key) {
                case KEY_8:
                    if ( popupListSelected < popupListSize ) {
                        popupListSelected++;
                    } else {
                        popupListSelected = 0;
                    }
                    break;
                
                case KEY_DOWN:
                    SetRegRadioValue(popupListSelected, true);
                    break;
                
                case KEY_UP:
                    SetRegRadioValue(popupListSelected, false);
                    break;

                //case KEY_MENU:
                case KEY_EXIT:
                    application_closePopup();
                    break;

                default:
                    break;
            }
            return;
        }
        switch (key) {
            case KEY_6:
            case KEY_5:
            case KEY_4:
            case KEY_DOWN:
                if ( popupListSelected < popupListSize ) {
                    popupListSelected++;
                } else {
                    popupListSelected = 0;
                }
                //MainVFO_popupSave(popup);
                break;
            case KEY_UP:
                if ( popupListSelected > 0 ) {
                    popupListSelected--;
                } else {
                    popupListSelected = popupListSize;
                }
                //MainVFO_popupSave(popup);
                break;
            case KEY_MENU:
                // save
                MainVFO_popupSave(popup);
                break;
            case KEY_EXIT:
                application_closePopup();
                break;
            default:
                break;
        }
   } else if ( state == KEY_RELEASED) {
        switch (key) {
            case KEY_MENU:
                application_closePopup();
                break;
            default:
                break;
        }
   }
}

/* --------------------------------------------------------------------------- */
// APP Main VFO
/* --------------------------------------------------------------------------- */

app_t APPMainVFO = {
    .showStatusLine = true,
    //.init = MainVFO_initFunction,
    .render = MainVFO_renderFunction,
    .timeoutHandler = MainVFO_timeoutHandlerFunction,
    .keyHandler = MainVFO_keyHandlerFunction,
    .renderPopup = MainVFO_renderPopupFunction,
    .keyHandlerPopup = MainVFO_keyHandlerPopupFunction,
};

