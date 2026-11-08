#include "system_clock.h"

#include "stm32f4xx.h"

// configuracao do clock

#define CLOCK_READY_TIMEOUT 1000000UL
#define CLOCK_HSE_HZ        8000000UL
#define CLOCK_PLL_M         4UL
#define CLOCK_PLL_N         180UL
#define CLOCK_PLL_Q         8UL

// espera com limite de tempo

static bool wait_for_value(volatile uint32_t *reg,
					   uint32_t mask,
					   uint32_t expected)
{
	uint32_t timeout = CLOCK_READY_TIMEOUT;

	while (((*reg & mask) != expected) && (timeout > 0U))
	{
		timeout--;
	}

	return ((*reg & mask) == expected);
}

// retorno ao clock interno em caso de falha

static void restore_safe_hsi(void)
{
	SET_BIT(RCC->CR, RCC_CR_HSION);
	(void)wait_for_value(&RCC->CR, RCC_CR_HSIRDY, RCC_CR_HSIRDY);

	MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_HSI);
	(void)wait_for_value(&RCC->CFGR, RCC_CFGR_SWS, RCC_CFGR_SWS_HSI);

	CLEAR_BIT(RCC->CR, RCC_CR_CSSON | RCC_CR_PLLON | RCC_CR_HSEON);
	MODIFY_REG(RCC->CFGR,
			   RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2,
			   RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV1 | RCC_CFGR_PPRE2_DIV1);
	SystemCoreClockUpdate();
}

// clock principal em 180 mhz

bool system_clock_configure_180mhz(void)
{
	uint32_t pll_configuration;

	SET_BIT(RCC->CR, RCC_CR_HSION);
	if (!wait_for_value(&RCC->CR, RCC_CR_HSIRDY, RCC_CR_HSIRDY))
	{
		return false;
	}

	MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_HSI);
	if (!wait_for_value(&RCC->CFGR, RCC_CFGR_SWS, RCC_CFGR_SWS_HSI))
	{
		return false;
	}

	CLEAR_BIT(RCC->CR, RCC_CR_PLLON);
	if (!wait_for_value(&RCC->CR, RCC_CR_PLLRDY, 0U))
	{
		return false;
	}

	CLEAR_BIT(RCC->CR, RCC_CR_HSEON);
	if (!wait_for_value(&RCC->CR, RCC_CR_HSERDY, 0U))
	{
		return false;
	}

	SET_BIT(RCC->CR, RCC_CR_HSEBYP | RCC_CR_HSEON);
	if (!wait_for_value(&RCC->CR, RCC_CR_HSERDY, RCC_CR_HSERDY))
	{
		restore_safe_hsi();
		return false;
	}

	SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);
	(void)READ_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);
	MODIFY_REG(PWR->CR, PWR_CR_VOS, PWR_CR_VOS);

	SET_BIT(PWR->CR, PWR_CR_ODEN);
	if (!wait_for_value(&PWR->CSR, PWR_CSR_ODRDY, PWR_CSR_ODRDY))
	{
		restore_safe_hsi();
		return false;
	}

	SET_BIT(PWR->CR, PWR_CR_ODSWEN);
	if (!wait_for_value(&PWR->CSR, PWR_CSR_ODSWRDY, PWR_CSR_ODSWRDY))
	{
		restore_safe_hsi();
		return false;
	}

	MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, FLASH_ACR_LATENCY_5WS);
	SET_BIT(FLASH->ACR, FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN);
	if ((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_5WS)
	{
		restore_safe_hsi();
		return false;
	}

	MODIFY_REG(RCC->CFGR,
			   RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2,
			   RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV4 | RCC_CFGR_PPRE2_DIV2);

	pll_configuration =
		(CLOCK_PLL_M << RCC_PLLCFGR_PLLM_Pos) |
		(CLOCK_PLL_N << RCC_PLLCFGR_PLLN_Pos) |
		RCC_PLLCFGR_PLLSRC_HSE |
		(CLOCK_PLL_Q << RCC_PLLCFGR_PLLQ_Pos);
	RCC->PLLCFGR = pll_configuration;

	SET_BIT(RCC->CR, RCC_CR_PLLON);
	if (!wait_for_value(&RCC->CR, RCC_CR_PLLRDY, RCC_CR_PLLRDY))
	{
		restore_safe_hsi();
		return false;
	}

	MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL);
	if (!wait_for_value(&RCC->CFGR, RCC_CFGR_SWS, RCC_CFGR_SWS_PLL))
	{
		restore_safe_hsi();
		return false;
	}

	__DSB();
	__ISB();
	SystemCoreClockUpdate();
	if ((CLOCK_HSE_HZ != HSE_VALUE) || (SystemCoreClock != SYSTEM_CLOCK_HZ))
	{
		restore_safe_hsi();
		return false;
	}

	SET_BIT(RCC->CR, RCC_CR_CSSON);
	return true;
}
