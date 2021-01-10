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
/*!  \file     delays.h
*    \brief    Different delays used in the mooltipass
*    Created:  09/6/2014
*    Author:   Mathieu Stephan
*/


#ifndef DELAYS_H_
#define DELAYS_H_

#include "timer_manager.h"

// Prototypes
void userViewDelay(void);
void smallForLoopBasedDelay(void);

// Macros
#define powerSettlingDelay()                timerBased130MsDelay()
#define touchSensorInterruptLineDelay()     timerBased130MsDelay()
#define screenComingOnDelay()               timerBased130MsDelay()

#endif /* DELAYS_H_ */