/* CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/license_cddl-1.0.txt
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/license_cddl-1.0.txt
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*!  \file     mini_gui_smartcard_functions.c
*    \brief    General user interface - smartcard functions
*    Created:  22/6/2014
*    Author:   Mathieu Stephan
*/ 
#include <string.h>
#include "smart_card_higher_level_functions.h"
#include "gui_screen_functions.h"
#include "gui_basic_functions.h"
#include "logic_aes_and_comms.h"
#include "logic_smartcard.h"
#include "timer_manager.h"
#include "logic_eeprom.h"
#include "mini_inputs.h"
#include "hid_defines.h"
#include "aes256_ctr.h"
#include "oledmini.h"
#include "defines.h"
#include "delays.h"
#include "anim.h"
#include "aes.h"
#include "gui.h"
#ifdef MINI_VERSION


/*! \fn     guiDisplayInsertSmartCardScreenAndWait(void)
*   \brief  Ask for the user to insert his smart card
*   \return RETURN_OK if the user inserted and unlocked his smartcard
*/
RET_TYPE guiDisplayInsertSmartCardScreenAndWait(void)
{
    RET_TYPE card_detect_ret = RETURN_JRELEASED;
    
    // Switch on lights
    activityDetectedRoutine();

    // Draw insert bitmap
    miniOledClearFrameBuffer();    
    #define BETA_TESTER_V
    #ifdef BETA_TESTER_V
        miniOledBitmapDrawFlash(0, 0, BITMAP_INSERT_CARD, OLED_SCROLL_NONE);
        #ifdef MINI_CREDENTIAL_MANAGEMENT
        miniOledPutCenteredString(21, "alt-beta v0.60");
        #endif
        miniOledFlushEntireBufferToDisplay();
    #else
        miniOledBitmapDrawFlash(0, 0, BITMAP_INSERT_CARD, OLED_SCROLL_FLIP);
    #endif
    
    // Wait for either timeout or for the user to insert his smartcard
    while ((hasTimerExpired(TIMER_USERINT, TRUE) == TIMER_RUNNING) && (card_detect_ret != RETURN_JDETECT))
    {
        card_detect_ret = isCardPlugged();
        miniGetWheelAction(FALSE, FALSE);
    }
    
    // If the user didn't insert his smart card
    if (card_detect_ret != RETURN_JDETECT)
    {
        // Get back to other screen
        guiGetBackToCurrentScreen();
        return RETURN_NOK;
    }
    else
    {
        return handleSmartcardInserted();
    }
}
#endif