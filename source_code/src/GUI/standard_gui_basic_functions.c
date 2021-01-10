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
/*!  \file     standard_gui_basic_functions.c
*    \brief    General user interface - basic functions
*    Created:  22/6/2014
*    Author:   Mathieu Stephan
*/
#include "gui_screen_functions.h"
#include "timer_manager.h"
#include "logic_eeprom.h"
#include "oled_wrapper.h"
#include "mini_inputs.h"
#include "defines.h"
#include "delays.h"
#include "anim.h"
#include "pwm.h"
#include "gui.h"
#ifdef HARDWARE_OLIVIER_V1

// Unique variables for the Mooltipass standard version
#if defined (HARDWARE_OLIVIER_V1)
    // Touch logic: is touch wheel pressed in our algo
    uint8_t touch_logic_press = FALSE;
    // Touch logic: reference position of first touch
    uint8_t touch_logic_ref_position;
    // Bool to know if lights are on
    uint8_t areLightsOn = FALSE;
    // Current led mask for the PCB
    uint8_t currentLedMask = 0;
#endif
// Screen saver on bool
uint8_t screenSaverOn = FALSE;


/*! \fn     isScreenSaverOn(void)
*   \brief  Returns screen saver bool
*   \return screen saver bool
*/
uint8_t isScreenSaverOn(void)
{
    return screenSaverOn;
}

/*! \fn     activityDetectedRoutine(void)
*   \brief  What to do when user activity has been detected
*/
void activityDetectedRoutine(void)
{    
    // Activate timers for automatic switch off & user interaction timeout
    #if defined(HARDWARE_OLIVIER_V1)
        activateTimer(TIMER_LIGHT, LIGHT_TIMER_DEL);
    #endif
    activateTimer(TIMER_SCREEN, SCREEN_TIMER_DEL);
    activateTimer(SLOW_TIMER_LOCKOUT, getMooltipassParameterInEeprom(LOCK_TIMEOUT_PARAM));
    activateTimer(TIMER_USERINT, ((uint16_t)controlEepromParameter(getMooltipassParameterInEeprom(USER_INTER_TIMEOUT_PARAM), MIN_USER_INTER_DEL/1000, MAX_USER_INTER_DEL/1000)) << 10);
    
    // If the screen was off, turn it on!
    if (oledIsOn() == FALSE)
    {
        oledOn();
        screenComingOnDelay();
    }
    
    // If we are in screen saver mode, exit it!
    if (screenSaverOn == TRUE)
    {
        screenSaverOn = FALSE;
    }
    
    // There are only lights in the Mooltipass standard version
    #if defined(HARDWARE_OLIVIER_V1)
        // If the lights were off, turn them on!
        if (areLightsOn == FALSE)
        {
            setPwmDc(MAX_PWM_VAL);
            activateGuardKey();
            areLightsOn = TRUE;
        }
    #endif
}

#if defined(HARDWARE_OLIVIER_V1)
/*! \fn     getTouchedPositionAnswer(uint8_t led_mask)
*   \brief  Use the capacitive interface to get quarter position
*   \param  led_mask    Led mask for the touchdetection routine
*   \return Number between 0 and 5 for valid pos, -1 otherwise
*/
int8_t getTouchedPositionAnswer(uint8_t led_mask)
{
    #if defined(ALWAYS_ACCEPT_REQUESTS)
        // First quarter is discarded, it means we want yes or no!
        if (led_mask & LED_MASK_WHEEL_TLEFT)
        {
            return TOUCHPOS_RIGHT;
        }
        else
        {
            return TOUCHPOS_WHEEL_TLEFT;
        }
    #endif

    RET_TYPE touch_detect_result;
    
    // Switch on lights
    activityDetectedRoutine();
    
    // Clear possible remaining detection
    touchDetectionRoutine(led_mask);
    
    // Additional masking in case we only want left / right
    uint8_t additional_mask = 0xFF;
    if (led_mask == LED_MASK_WHEEL)
    {
        additional_mask = RETURN_LEFT_PRESSED | RETURN_RIGHT_PRESSED;
    }
    
    // Wait for a touch press
    do
    {
        // User interaction timeout or smartcard removed
        if ((hasTimerExpired(TIMER_USERINT, TRUE) == TIMER_EXPIRED) || (isSmartCardAbsent() == RETURN_OK))
        {
            return -1;
        }
        // Read usb comms as the plugin could ask to cancel the request
        if (usbCancelRequestReceived() == RETURN_OK)
        {
            return -1;
        }

        touch_detect_result = touchDetectionRoutine(led_mask) & TOUCH_PRESS_MASK & additional_mask;
    }
    while (!touch_detect_result);
    
    // Prevent touches until the user lifts his finger
    touchInhibitUntilRelease();
    
    // Did the user press one of the two touch buttons?
    if (touch_detect_result & RETURN_LEFT_PRESSED)
    {
        return TOUCHPOS_LEFT;
    }
    else if (touch_detect_result & RETURN_RIGHT_PRESSED)
    {
        return TOUCHPOS_RIGHT;
    }
    else
    {
        return (int8_t)getWheelTouchDetectionQuarter();
    } 
}

/*! \fn     touchWheelIntefaceLogic(void)
*   \brief  Use the wheel and get -1 or +1 ticks
*   \param  touch_detection_result  Result from the touchdetectionroutine
*   \return -1 or +1 ticks
*/
int8_t touchWheelIntefaceLogic(RET_TYPE touch_detection_result)
{
    uint8_t temp_position;
    uint8_t up_lower_slot;
    uint8_t up_higher_slot;
    uint8_t down_lower_slot;
    uint8_t down_higher_slot;
    
    if (touch_detection_result & RETURN_WHEEL_PRESSED)
    {
        // Get touched position
        temp_position = getLastRawWheelPosition();
        
        // If it is the first touch, store reference position
        if (touch_logic_press == FALSE)
        {
            touch_logic_ref_position = temp_position;
            touch_logic_press = TRUE;
        }
        
        up_lower_slot = touch_logic_ref_position + WHEEL_TICK_INCREMENT;
        up_higher_slot = touch_logic_ref_position + 3*WHEEL_TICK_INCREMENT;
        down_higher_slot = touch_logic_ref_position - WHEEL_TICK_INCREMENT;
        down_lower_slot = touch_logic_ref_position - 3*WHEEL_TICK_INCREMENT;
        
        // Detect wrap arounds
        if (up_lower_slot > up_higher_slot)
        {
            if ((temp_position > up_lower_slot) || (temp_position < up_higher_slot))
            {
                touch_logic_ref_position += WHEEL_TICK_INCREMENT;
                return 1;
            }
        }
        else
        {
            if ((temp_position > up_lower_slot) && (temp_position < up_higher_slot))
            {
                touch_logic_ref_position += WHEEL_TICK_INCREMENT;
                return 1;
            }
        }
        
        // Same
        if (down_lower_slot > down_higher_slot)
        {
            if ((temp_position < down_higher_slot) || (temp_position > down_lower_slot))
            {
                touch_logic_ref_position -= WHEEL_TICK_INCREMENT;
                return -1;
            }
        }
        else
        {
            if ((temp_position < down_higher_slot) && (temp_position > down_lower_slot))
            {
                touch_logic_ref_position -= WHEEL_TICK_INCREMENT;
                return -1;
            }
        }
    }
    else if(touch_detection_result & RETURN_WHEEL_RELEASED)
    {
        touch_logic_press = FALSE;
    }
    
    return 0;
}
#endif

/*! \fn     guiMainLoop(void)
*   \brief  Main user interface loop
*/
void guiMainLoop(void)
{
    RET_TYPE input_interface_result;
    uint8_t screenSaverOnCopy;
    uint8_t isScreenOnCopy;
    
    #if defined(HARDWARE_OLIVIER_V1)
        // Set led mask depending on our current screen
        switch(getCurrentScreen())
        {
            case SCREEN_DEFAULT_NINSERTED :         currentLedMask = LED_MASK_LEFT|LED_MASK_RIGHT|LED_MASK_WHEEL; break;
            case SCREEN_DEFAULT_INSERTED_LCK :      currentLedMask = LED_MASK_LEFT|LED_MASK_RIGHT|LED_MASK_WHEEL; break;
            case SCREEN_DEFAULT_INSERTED_NLCK :     currentLedMask = LED_MASK_LEFT|LED_MASK_RIGHT; break;
            case SCREEN_DEFAULT_INSERTED_INVALID :  currentLedMask = LED_MASK_LEFT|LED_MASK_RIGHT|LED_MASK_WHEEL; break;
            case SCREEN_SETTINGS :                  currentLedMask = LED_MASK_LEFT|LED_MASK_RIGHT; break;
            case SCREEN_MEMORY_MGMT :               currentLedMask = LED_MASK_LEFT|LED_MASK_RIGHT|LED_MASK_WHEEL; break;
            default: break;
        }
    #endif
    
    // Make a copy of the screen on & screensaver on bools
    screenSaverOnCopy = screenSaverOn;
    isScreenOnCopy = oledIsOn();
    
    #if defined(HARDWARE_OLIVIER_V1)
        // Launch touch detection routine to check for interactions
        input_interface_result = touchDetectionRoutine(currentLedMask);
    #elif defined(MINI_VERSION)
        input_interface_result = miniGetWheelAction(FALSE, FALSE);
    #endif
    
    #if defined(HARDWARE_OLIVIER_V1)
        // No activity, switch off LEDs and activate prox detection
        if (hasTimerExpired(TIMER_LIGHT, TRUE) == TIMER_EXPIRED)
        {
            setPwmDc(0x0000);
            areLightsOn = FALSE;
            activateProxDetection();
        }
    #endif
    
    // No activity, switch off screen
    if (hasTimerExpired(TIMER_SCREEN, TRUE) == TIMER_EXPIRED)
    {
        #ifndef MINI_DEMO_VIDEO
            guiDisplayGoingToSleep();
            userViewDelay();
            if (getMooltipassParameterInEeprom(SCREENSAVER_PARAM) != FALSE)
            {
                screenSaverOn = TRUE;
                #ifndef MINI_VERSION
                oledWriteInactiveBuffer();
                oledClear();
                oledDisplayOtherBuffer();
                oledClear();
                #endif
            }
            else
            {
                oledOff();
                #ifndef MINI_VERSION
                oledDisplayOtherBuffer();
                #else
                guiGetBackToCurrentScreen();
                #endif
            }
        #else
            oledBitmapDrawFlash(0, 0, BITMAP_MOOLTIPASS, OLED_SCROLL_UP);
        #endif
    }
    
    #if defined(HARDWARE_OLIVIER_V1)
        // If there was some activity and we are showing the screen saver
        if ((input_interface_result & TOUCH_PRESS_MASK) && (screenSaverOnCopy == TRUE))
        {
            guiGetBackToCurrentScreen();
        }
    
        // If the screen just got turned on, don't call the guiScreenLoop() function
        if ((input_interface_result & TOUCH_PRESS_MASK) && (((isScreenOnCopy != FALSE) && (screenSaverOnCopy == FALSE)) || (getCurrentScreen() == SCREEN_DEFAULT_INSERTED_LCK)))
        {
            guiScreenLoop(input_interface_result);
        }
    #elif defined(MINI_VERSION)
        // If there was some activity and we are showing the screen saver
        if ((input_interface_result != WHEEL_ACTION_NONE) && (screenSaverOnCopy == TRUE))
        {
            guiGetBackToCurrentScreen();
        }

        if ((input_interface_result != WHEEL_ACTION_NONE) && (((isScreenOnCopy != FALSE) && (screenSaverOnCopy == FALSE)) || (getCurrentScreen() == SCREEN_DEFAULT_INSERTED_LCK)))
        {
            guiScreenLoop(input_interface_result);
        }
    #endif
}
#endif