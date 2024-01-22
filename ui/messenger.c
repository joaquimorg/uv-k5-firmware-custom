
#ifdef ENABLE_MESSENGER

#include <string.h>

#include <string.h>
#include "app/messenger.h"
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "misc.h"
#include "settings.h"
#include "ui/messenger.h"
#include "ui/helper.h"
#include "ui/inputbox.h"
#include "ui/ui.h"
#ifdef ENABLE_DOCK
	#include "app/uart.h"
#endif

void UI_DisplayMSG(void) {

	static char String[37];

	UI_DisplayClear();
	memset(String, 0, sizeof(String));

    strcpy(String, "Messenger");
    GUI_DisplaySmallest(String, 128-1-36, 2, false, true);

	// Message Display

	memset(String, 0, sizeof(String));

	uint8_t mPos = 1;
	const uint8_t mLine = 7;

    for (int i = 0; i < DISPLAY_MSG_COUNT; ++i) {
		//sprintf(String, "%s", rxMessage[i]);
        uint8_t currIdx=currDisplayMsgID+(DISPLAY_MSG_COUNT-i-1);
		GUI_DisplaySmallest(rxMessage[currIdx], 2, mPos, false, true);
		#ifdef ENABLE_DOCK
			UART_SendUiElement(1, 2, (mPos / 6), 4, strlen(rxMessage[currIdx]), rxMessage[currIdx]);
		#endif
		mPos += mLine;
    }

    // Scroll Indicator
    if (canScrollUp) {
        char str[] = ""; // ASCII 128
        GUI_DisplaySmallest(str+1, 120, 19, false, true); // Low byte only
    }
    if (canScrollDown) {
        char str[] = ""; // ASCII 129
        GUI_DisplaySmallest(str+1, 120, 22, false, true); // Low byte only
    }

    // Input box
    memset(String, 0, sizeof(String));
    strcpy(String, "Input");
    UI_DrawDottedLineBuffer(gFrameBuffer, 2, 44, 94, 44, true, 2);
    GUI_DisplaySmallest(String, 98, 42, false, true);
    

	memset(String, 0, sizeof(String));
	if ( keyboardType == NUMERIC ) {
		strcpy(String, "2");
	} else if ( keyboardType == UPPERCASE ) {
		strcpy(String, "B");
	} else {
		strcpy(String, "b");
	}

	UI_DrawRectangleBuffer(gFrameBuffer, 119, 40, 125, 48, true);
	GUI_DisplaySmallest(String, 121, 42, false, true);
	#ifdef ENABLE_DOCK
		UART_SendUiElement(121, 42, (32 / 6), 4, strlen(String), String);
	#endif

    // Char bar
	memset(String, 0, sizeof(String));
	sprintf(String, "%s_", cMessage);
	//UI_PrintStringSmall(String, 3, 0, 6);
	GUI_DisplaySmallest(String, 2, 48, false, true);
	#ifdef ENABLE_DOCK
		UART_SendUiElement(2, 2, (38 / 6), 4, strlen(String), String);
	#endif

    // Debug Msgs 1
    // memset(String, 0, sizeof(String));
	// sprintf(String, "Did:%u", currDisplayMsgID);
	// GUI_DisplaySmallest(String, 64, 24, false, true);

    // memset(String, 0, sizeof(String));
	// sprintf(String, "Rx:%u", totalMsgsReceived);
	// GUI_DisplaySmallest(String, 64, 31, false, true);

	// Debug Msgs 2
	// memset(String, 0, sizeof(String));
	// sprintf(String, "S:%u", gErrorsDuringMSG);
	// GUI_DisplaySmallest(String, 4, 12, false, true);

    
	// memset(String, 0, sizeof(String));
	// for (uint8_t i = 0; i < 19; i++) {
	// 	sprintf(&String[i*2], "%02X", rxMessage[i]);
	// }

	// GUI_DisplaySmallest(String, 20, 34, false, true);

	ST7565_BlitFullScreen();
}

#endif
