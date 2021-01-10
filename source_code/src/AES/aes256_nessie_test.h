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
/*!	\file 	aes256_nessie_test.h
*	\brief	Different functions to check AES256 against nessie test vectors
*
*	Created: 16/02/2014 13:54:34
*	Author: Miguel A. Borrego
*/

#ifndef __AES256_NESSIE_TEST_H__
#define __AES256_NESSIE_TEST_H__

#include <stdint.h>

#ifdef NESSIE_TEST_VECTORS
/*! \brief function pointer to the output function */
int8_t (*nessieOutput)(uint8_t ch);

// prototype functions
void nessieTest(uint8_t setnum);
#endif

#endif /*__AES256_NESSIE_TEST_H__*/
