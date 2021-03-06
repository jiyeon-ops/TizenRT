/****************************************************************************
 *
 * Copyright 2018 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/
/****************************************************************************
 * arch/arm/src/stm32/stm32_lse.c
 *
 *   Copyright (C) 2009, 2011 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.orgr>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <tinyara/config.h>

#include "up_arch.h"

#include "stm32_pwr.h"
#include "stm32_rcc.h"
#include "stm32_waste.h"

/****************************************************************************
 * Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_rcc_enablelse
 *
 * Description:
 *   Enable the External Low-Speed (LSE) oscillator and, if the RTC is
 *   configured, setup the LSE as the RTC clock source, and enable the RTC.
 *
 *   For the STM32L15X family, this will also select the LSE as the clock
 *   source of the LCD.
 *
 * Todo:
 *   Check for LSE good timeout and return with -1,
 *
 ****************************************************************************/

#ifdef CONFIG_STM32_STM32L15XX
void stm32_rcc_enablelse(void)
{
	uint16_t pwrcr;

	/* The LSE is in the RTC domain and write access is denied to this domain
	 * after reset, you have to enable write access using DBP bit in the PWR CR
	 * register before to configuring the LSE.
	 */

	pwrcr = getreg16(STM32_PWR_CR);
	putreg16(pwrcr | PWR_CR_DBP, STM32_PWR_CR);

	/* Enable the External Low-Speed (LSE) oscillator by setting the LSEON bit
	 * the RCC CSR register.
	 */

	modifyreg32(STM32_RCC_CSR, 0, RCC_CSR_LSEON);

	/* Wait for the LSE clock to be ready */

	while ((getreg32(STM32_RCC_CSR) & RCC_CSR_LSERDY) == 0) {
		up_waste();
	}

	/* The primariy purpose of the LSE clock is to drive the RTC with an accurate
	 * clock source.  In the STM32L family, the RTC and the LCD are coupled so
	 * that must use the same clock source.  Calling this function will select
	 * the LSE will be used to drive the LCD as well.
	 */

#if defined(CONFIG_STM32_LCD) || defined(CONFIG_RTC)
	/* Select LSE as RTC/LCD Clock Source by setting the RTCSEL field of the RCC
	 * CSR register.
	 */

	modifyreg32(STM32_RCC_CSR, RCC_CSR_RTCSEL_MASK, RCC_CSR_RTCSEL_LSE);

#if defined(CONFIG_RTC)
	/* Enable the RTC Clock by setting the RTCEN bit in the RCC CSR register */

	modifyreg32(STM32_RCC_CSR, 0, RCC_CSR_RTCEN);
#endif
#endif

	/* Restore the previous state of the DBP bit */

	putreg16(pwrcr, STM32_PWR_CR);
}
#else

void stm32_rcc_enablelse(void)
{
	/* Enable the External Low-Speed (LSE) oscillator by setting the LSEON bit
	 * the RCC BDCR register.
	 */

	modifyreg16(STM32_RCC_BDCR, 0, RCC_BDCR_LSEON);

	/* Wait for the LSE clock to be ready */

	while ((getreg16(STM32_RCC_BDCR) & RCC_BDCR_LSERDY) == 0) {
		up_waste();
	}

	/* The primariy purpose of the LSE clock is to drive the RTC.  The RTC could
	 * also be driven by the LSI (but that would be very inaccurate) or by the
	 * HSE (but that would prohibit low-power operation)
	 */

#ifdef CONFIG_RTC
	/* Select LSE as RTC Clock Source by setting the RTCSEL field of the RCC
	 * BDCR register.
	 */

	modifyreg16(STM32_RCC_BDCR, RCC_BDCR_RTCSEL_MASK, RCC_BDCR_RTCSEL_LSE);

	/* Enable the RTC Clock by setting the RTCEN bit in the RCC BDCR register */

	modifyreg16(STM32_RCC_BDCR, 0, RCC_BDCR_RTCEN);
#endif
}
#endif
