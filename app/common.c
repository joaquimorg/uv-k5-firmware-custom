#include "app/chFrScanner.h"
#include "audio.h"
#include "functions.h"
#include "misc.h"
#include "settings.h"
#include "ui/ui.h"

void COMMON_KeypadLockToggle() 
{

    if (gScreenToDisplay != DISPLAY_MENU &&
        gCurrentFunction != FUNCTION_TRANSMIT)
    {	// toggle the keyboad lock

        gSettings.KEY_LOCK = !gSettings.KEY_LOCK;

        //gRequestSaveSettings = true;
    }
}

void COMMON_SwitchVFOs()
{
#ifdef ENABLE_SCAN_RANGES    
    gScanRangeStart = 0;
#endif
    gSettings.activeVFO ^= 1;

    if (gSettings.CROSS_BAND_RX_TX != CROSS_BAND_OFF)
        gSettings.CROSS_BAND_RX_TX = gSettings.activeVFO + 1;
    if (gSettings.DUAL_WATCH != DUAL_WATCH_OFF)
        gSettings.DUAL_WATCH = gSettings.activeVFO + 1;

    //gRequestSaveSettings  = 1;
    //gFlagReconfigureVfos  = true;
    gScheduleDualWatch = true;

    gRequestDisplayScreen = DISPLAY_MAIN;
}

void COMMON_SwitchVFOMode()
{

    //if (gSettings.VFO_OPEN) {
        if (IS_MR_CHANNEL(gTxVfo->CHANNEL_SAVE))
        {	// swap to frequency mode
            gScreenChannel[gSettings.activeVFO] = gFreqChannel[gSettings.activeVFO];
            //gRequestSaveVFO            = true;
            //gVfoConfigureMode          = VFO_CONFIGURE_RELOAD;
            return;
        }

        uint8_t Channel = RADIO_FindNextChannel(gMrChannel[gSettings.activeVFO], 1, false, 0);
        if (Channel != 0xFF)
        {	// swap to channel mode
            gScreenChannel[gSettings.activeVFO] = Channel;
            //gRequestSaveVFO     = true;
            //gVfoConfigureMode   = VFO_CONFIGURE_RELOAD;
            return;
        }
    //}
}