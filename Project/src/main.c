#include "RC522.h"
#include "string.h"
#include <stdio.h>

uint8_t status;
uint8_t data[18];

void SystemClock_Config(void);

int main(void)
{
	SystemClock_Config();
  init_RC522();
	while(1)
	{
		/*Request Answer (REQA, 0x26)*/
		while (request_card(PICC_REQALL, data)==ERR){LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_1 | LL_GPIO_PIN_2);}
		/*Wake-Up command (WUPA, 0x52)
		while (request_card(PICC_REQIDL, data)==ERR){};
		status=select_card(PICC_ANTICOLL1, PICC_ARG_SELECT, data);
		status=read_UID(PICC_ANTICOLL2, PICC_ARG_UID, data);
		status=select_card(PICC_ANTICOLL2, PICC_ARG_SELECT, data);*/
		status=read_UID(PICC_ANTICOLL1, PICC_ARG_UID, data);
		data[1] == '&' ? LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_1) : LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_2);
		//sprintf((char*)data, "");
	}
}

void SystemClock_Config(void)
{
  /* Enable ACC64 access and set FLASH latency */ 
  LL_FLASH_Enable64bitAccess();; 
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

  /* Set Voltage scale1 as MCU will run at 32MHz */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
  
  /* Poll VOSF bit of in PWR_CSR. Wait until it is reset to 0 */
  while (LL_PWR_IsActiveFlag_VOSF() != 0)
  {
  };
  
  /* Enable HSI if not already activated*/
  if (LL_RCC_HSI_IsReady() == 0)
  {
    /* HSI configuration and activation */
    LL_RCC_HSI_Enable();
    while(LL_RCC_HSI_IsReady() != 1)
    {
    };
  }
  
	
  /* Main PLL configuration and activation */
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLL_MUL_6, LL_RCC_PLL_DIV_3);

  LL_RCC_PLL_Enable();
  while(LL_RCC_PLL_IsReady() != 1)
  {
  };
  
  /* Sysclk activation on the main PLL */
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  };
  
  /* Set APB1 & APB2 prescaler*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

  /* Set systick to 1ms in using frequency set to 32MHz                             */
  /* This frequency can be calculated through LL RCC macro                          */
  /* ex: __LL_RCC_CALC_PLLCLK_FREQ (HSI_VALUE, LL_RCC_PLL_MUL_6, LL_RCC_PLL_DIV_3); */
  LL_Init1msTick(32000000);
  
  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  LL_SetSystemCoreClock(32000000);
}
