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
/*
 * mooltipass.h
 *
 * Created: 04/01/2014 20:32:07
 *  Author: Mathieu Stephan
 */


#ifndef MOOLTIPASS_H_
#define MOOLTIPASS_H_

#include "defines.h"

/* Defines */
#define CAPS_LOCK_DEL       400

/* Shared variables */
extern bootloader_f_ptr_type start_bootloader;
extern uint8_t mp_lock_unlock_shortcuts;
extern uint8_t mp_timeout_enabled;
extern uint8_t act_detected_flag;

/* Function prototypes */
void get_and_clear_button_pressed_return(uint8_t* buffer);
void programming_success(uint8_t socket_id);
void programming_failure(uint8_t socket_id);
void reboot_platform(void);

#endif /* MOOLTIPASS_H_ */
