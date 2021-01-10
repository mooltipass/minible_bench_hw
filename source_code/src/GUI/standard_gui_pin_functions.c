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
/*!  \file     standard_gui_pin_functions.c
*    \brief    General user interface - pin functions
*    Created:  22/6/2014
*    Author:   Mathieu Stephan
*/
#include <string.h>
#include "smart_card_higher_level_functions.h"
#include "gui_screen_functions.h"
#include "gui_basic_functions.h"
#include "usb_cmd_parser.h"
#include "oled_wrapper.h"
#include "mini_inputs.h"
#include "defines.h"
#include "delays.h"
#include "anim.h"
#ifdef HARDWARE_OLIVIER_V1

/*! \fn     guiDisplayPinOnPinEnteringScreen(uint8_t* current_pin, uint8_t selected_digit)
*   \brief  Overwrite the digits on the current pin entering screen
*   \param  current_pin     Array containing the pin
*   \param  selected_digit  Currently selected digit
*   \param  stringID        String ID for text qu
*/
void guiDisplayPinOnPinEnteringScreen(uint8_t* current_pin, uint8_t selected_digit, uint8_t stringID)
{
    #if defined(HARDWARE_OLIVIER_V1)
        (void)stringID;
        oledFillXY(88, 31, 82, 19, 0x00);
        for (uint8_t i = 0; i < 4; i++)
        {
            oledSetXY(88+22*i, 25);
            if (i != selected_digit)
            {
                oledPutch('*');
            }
            else
            {
                if (current_pin[i] >= 0x0A)
                {
                    oledPutch(current_pin[i]+'A'-0x0A);
                }
                else
                {
                    oledPutch(current_pin[i]+'0');
                }
            }
        }
    #elif defined(MINI_VERSION)
        // Display bitmap
        oledBitmapDrawFlash(0, 0, selected_digit+BITMAP_PIN_SLOT1, 0);
        miniOledSetMaxTextY(62);
        miniOledAllowTextWritingYIncrement();
        miniOledPutCenteredString(TWO_LINE_TEXT_FIRST_POS, readStoredStringToBuffer(stringID));
        miniOledPreventTextWritingYIncrement();
        miniOledResetMaxTextY();
        oledSetFont(FONT_PROFONT_14);
        for (uint8_t i = 0; i < 4; i++)
        {
            oledSetXY(64+17*i, 6);
            if (i != selected_digit)
            {
                oledPutch('*');
            }
            else
            {
                if (current_pin[i] >= 0x0A)
                {
                    oledPutch(current_pin[i]+'A'-0x0A);
                }
                else
                {
                    oledPutch(current_pin[i]+'0');
                }
            }
        }
        oledSetFont(FONT_DEFAULT);
        miniOledFlushEntireBufferToDisplay();
    #endif
}

/*! \fn     guiGetPinFromUser(volatile uint16_t* pin_code, uint8_t stringID)
*   \brief  Ask the user to enter a PIN
*   \param  pin_code    Pointer to where to store the pin code
*   \param  stringID    String ID
*   \return If the user approved the request
*/
RET_TYPE guiGetPinFromUser(volatile uint16_t* pin_code, uint8_t stringID)
{
    // If we don't need a pin code, send default one
    #if defined(NO_PIN_CODE_REQUIRED)
        *pin_code = SMARTCARD_DEFAULT_PIN;
        return RETURN_OK;
    #endif
    
    #if defined(HARDWARE_OLIVIER_V1)
        RET_TYPE ret_val = RETURN_NOK;
        uint8_t selected_digit = 0;
        uint8_t finished = FALSE;
        uint8_t current_pin[4];
        RET_TYPE temp_rettype;
        int8_t temp_int8;
    
        // Set current pin to 0000
        memset((void*)current_pin, 0, 4);
    
        // Draw pin entering bitmap
        oledClear();
        oledBitmapDrawFlash(0, 0, BITMAP_YES_NO, 0);
        oledBitmapDrawFlash(83, 51, BITMAP_PIN_LINES, 0);
        oledBitmapDrawFlash(238, 23, BITMAP_RIGHT_ARROW, 0);
        oledPutstrXY(0, 0, OLED_CENTRE, readStoredStringToBuffer(stringID));
        oledDisplayOtherBuffer();
        oledSetFont(FONT_PROFONT_24);
        oledWriteActiveBuffer();
    
        // Display current pin on screen
        guiDisplayPinOnPinEnteringScreen(current_pin, selected_digit, stringID);
    
        // While the user hasn't entered his pin
        while(!finished)
        {
            // Still process the USB commands
            usbProcessIncoming(USB_CALLER_PIN);
            // Detect key touches
            temp_rettype = touchDetectionRoutine(0);
            // Send it to the touch wheel interface logic
            temp_int8 = touchWheelIntefaceLogic(temp_rettype);
        
            // Position increment / decrement
            if (temp_int8 != 0)
            {
                if ((current_pin[selected_digit] == 0x0F) && (temp_int8 == 1))
                {
                    current_pin[selected_digit] = 0xFF;
                }
                else if ((current_pin[selected_digit] == 0) && (temp_int8 == -1))
                {
                    current_pin[selected_digit] = 0x10;
                }
                current_pin[selected_digit] += temp_int8;
                guiDisplayPinOnPinEnteringScreen(current_pin, selected_digit, stringID);
            }
        
            if ((isSmartCardAbsent() == RETURN_OK) || (hasTimerExpired(TIMER_USERINT, TRUE) == TIMER_EXPIRED))
            {
                // Smartcard removed, no reason to continue
                ret_val = RETURN_NOK;
                finished = TRUE;
            }
            if (temp_rettype & RETURN_LEFT_PRESSED)
            {
                if (selected_digit == 1)
                {
                    oledFillXY(0, 23, 18, 18, 0x00);
                    oledBitmapDrawFlash(0, 24, BITMAP_CROSS, 0);
                }
                if (selected_digit > 0)
                {
                    // When going back set pin digit to 0
                    current_pin[selected_digit] = 0;
                    current_pin[--selected_digit] = 0;
                }
                else
                {
                    ret_val = RETURN_NOK;
                    finished = TRUE;
                }
                guiDisplayPinOnPinEnteringScreen(current_pin, selected_digit, stringID);
                oledBitmapDrawFlash(238, 23, BITMAP_RIGHT_ARROW, 0);
            }
            else if (temp_rettype & RETURN_RIGHT_PRESSED)
            {
                if (selected_digit == 2)
                {
                    oledFillXY(238, 23, 18, 18, 0x00);
                    oledBitmapDrawFlash(240, 24, BITMAP_TICK, 0);
                }
                if (selected_digit < 3)
                {
                    selected_digit++;
                }
                else
                {
                    ret_val = RETURN_OK;
                    finished = TRUE;
                }
                guiDisplayPinOnPinEnteringScreen(current_pin, selected_digit, stringID);
                oledBitmapDrawFlash(0, 23, BITMAP_LEFT_ARROW, 0);
            }
        }
    
        // Reset default font
        oledSetFont(FONT_DEFAULT);
        oledWriteInactiveBuffer();
    
        // Store the pin
        *pin_code = (uint16_t)(((uint16_t)(current_pin[0]) << 12) | (((uint16_t)current_pin[1]) << 8) | (current_pin[2] << 4) | current_pin[3]);
    
        // Set current pin to 0000
        memset((void*)current_pin, 0, 4);
    
        // Prevent touches until the user lifts his finger
        touchInhibitUntilRelease();
    
        // Return success status
        return ret_val;
    #elif defined(MINI_VERSION)
        RET_TYPE ret_val = RETURN_NOK;
        uint8_t detection_result = 0;
        uint8_t selected_digit = 0;
        uint8_t finished = FALSE;
        uint8_t current_pin[4];
    
        // Set current pin to 0000
        memset((void*)current_pin, 0, 4);
        
        // Clear current detections
        miniWheelClearDetections();
    
        // Display current pin on screen
        guiDisplayPinOnPinEnteringScreen(current_pin, selected_digit, stringID);
    
        // While the user hasn't entered his pin
        while(!finished)
        {
            // Still process the USB commands
            usbProcessIncoming(USB_CALLER_PIN);
            // detection result
            detection_result = miniGetWheelAction(FALSE, FALSE);
        
            // Position increment / decrement
            if ((detection_result == WHEEL_ACTION_UP) || (detection_result == WHEEL_ACTION_DOWN))
            {
                if ((current_pin[selected_digit] == 0x0F) && (detection_result == WHEEL_ACTION_UP))
                {
                    current_pin[selected_digit] = 0xFF;
                }
                else if ((current_pin[selected_digit] == 0) && (detection_result == WHEEL_ACTION_DOWN))
                {
                    current_pin[selected_digit] = 0x10;
                }
                if (detection_result == WHEEL_ACTION_UP)
                {
                    current_pin[selected_digit]++;
                } 
                else
                {
                    current_pin[selected_digit]--;
                }
                guiDisplayPinOnPinEnteringScreen(current_pin, selected_digit, stringID);
            }
        
            // Return if card removed or timer expired
            if ((isSmartCardAbsent() == RETURN_OK) || (hasTimerExpired(TIMER_USERINT, TRUE) == TIMER_EXPIRED))
            {
                // Smartcard removed, no reason to continue
                ret_val = RETURN_NOK;
                finished = TRUE;
            }
            
            // Change digit position or return/proceed
            if (detection_result == WHEEL_ACTION_LONG_CLICK)
            {
                if (selected_digit > 0)
                {
                    // When going back set pin digit to 0
                    current_pin[selected_digit] = 0;
                    current_pin[--selected_digit] = 0;
                }
                else
                {
                    ret_val = RETURN_NOK;
                    finished = TRUE;
                }
                guiDisplayPinOnPinEnteringScreen(current_pin, selected_digit, stringID);
            }
            else if (detection_result == WHEEL_ACTION_SHORT_CLICK)
            {
                if (selected_digit < 3)
                {
                    selected_digit++;
                }
                else
                {
                    ret_val = RETURN_OK;
                    finished = TRUE;
                }
                guiDisplayPinOnPinEnteringScreen(current_pin, selected_digit, stringID);
            }
        }
    
        // Store the pin
        *pin_code = (uint16_t)(((uint16_t)(current_pin[0]) << 12) | (((uint16_t)current_pin[1]) << 8) | (current_pin[2] << 4) | current_pin[3]);
    
        // Set current pin to 0000 & set default font
        memset((void*)current_pin, 0, 4);
        oledSetFont(FONT_DEFAULT);
    
        // Return success status
        return ret_val;
    #endif
}

/*! \fn     guiCardUnlockingProcess(void)
*   \brief  Function called for the user to unlock his smartcard
*   \return success status
*/
RET_TYPE guiCardUnlockingProcess(void)
{
    volatile uint16_t temp_pin;
    RET_TYPE temp_rettype;
    
    while (1)
    {
        if (guiGetPinFromUser(&temp_pin, ID_STRING_INSERT_PIN) == RETURN_OK)
        {            
            // Try unlocking the smartcard
            temp_rettype = mooltipassDetectedRoutine(&temp_pin);
            
            switch(temp_rettype)
            {
                case RETURN_MOOLTIPASS_4_TRIES_LEFT :
                {
                    // Smartcard unlocked
                    temp_pin = 0x0000;
                    return RETURN_OK;
                }
                case RETURN_MOOLTIPASS_0_TRIES_LEFT :
                {
                    guiDisplayInformationOnScreenAndWait(ID_STRING_CARD_BLOCKED);
                    return RETURN_NOK;
                }
                case RETURN_MOOLTIPASS_PB :
                {
                    guiDisplayInformationOnScreenAndWait(ID_STRING_PB_CARD);
                    return RETURN_NOK;
                }
                default :
                {
                    // Both the enum and the defines allow us to do that
                    guiDisplayInformationOnScreenAndWait(ID_STRING_WRONGPIN1LEFT + temp_rettype - RETURN_MOOLTIPASS_1_TRIES_LEFT);
                    break;
                }
            }
        }
        else
        {
            // User cancelled the request
            return RETURN_NOK;
        }
    }
}
#endif