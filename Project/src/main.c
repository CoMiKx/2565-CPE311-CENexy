#include "stm32l1xx.h"
#include "stm32l1xx_ll_system.h"
#include "stm32l1xx_ll_bus.h"
#include "stm32l1xx_ll_gpio.h"
#include "stm32l1xx_ll_pwr.h"
#include "stm32l1xx_ll_rcc.h"
#include "stm32l1xx_ll_utils.h"
#include "stm32l1xx_ll_spi.h"
#include "RC522.h"


/* Default key for authentication */
const uint8_t defaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
/* Block number to write and read data */
const uint8_t blockNum = 2;
/* Buffer to store data to be written to the block */
uint8_t blockData[16] = {"TEST_DATA"};
/* Buffer to store data read from the block */
uint8_t readBlockData[18];
uint8_t status, i, j;
uint8_t data[18], data2[100];

void SystemClock_Config(void);
void WriteDataToBlock(uint8_t blockNum, uint8_t blockData[]);
void ReadDataFromBlock(uint8_t blockNum, uint8_t readBlockData[]);

int main(void)
{
	SystemClock_Config();
  init_RC522();
	while(1)
	{
		/*Request Answer (REQA, 0x26)*/
		while (request_card(PICC_REQALL, data)==ERR){};
		/*Wake-Up command (WUPA, 0x52)*/
		while (request_card(PICC_REQIDL, data)==ERR){};
		status=read_UID(PICC_ANTICOLL1, PICC_ARG_UID, data);
		status=select_card(PICC_ANTICOLL1, PICC_ARG_SELECT, data);
		status=read_UID(PICC_ANTICOLL2, PICC_ARG_UID, data);
		status=select_card(PICC_ANTICOLL2, PICC_ARG_SELECT, data);
		for (i=0; i<4 && read_page(i*4, data)==OK; i++)
		{
//			status=read_page(i*4, data);
//			if (status==ERR){break;}
			for (j=0; j<4; j++)
			{
				data2[j*4] = data[j*4];
			}
		}
	}
}

void WriteDataToBlock(uint8_t blockNum, uint8_t blockData[])
{
	/* Set SS (PB10) pin low to select the RC522 module */
	LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_10);
	/* Send the authentication command */
	LL_SPI_TransmitData8(SPI2, 0x0A);
	/* Send the block number */
	LL_SPI_TransmitData8(SPI2, blockNum);
	/* Send the default key */
	for(int i = 0; i < 6; i++)
	{
			LL_SPI_TransmitData8(SPI2, defaultKey[i]);
	}
	/* Send the data to be written */
	for(int i = 0; i < 16; i++)
	{
			LL_SPI_TransmitData8(SPI2, blockData[i]);
	}
	/* Wait until the transmission is complete */
	while(!LL_SPI_IsActiveFlag_TXE(SPI2));
	/* Set SS (PB10) pin high to deselect the RC522 module */
	LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_10);
}

void ReadDataFromBlock(uint8_t blockNum, uint8_t readBlockData[])
{
	/* Set SS (PB10) pin low to select the RC522 module */
	LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_10);
	/* Send the read command */
	LL_SPI_TransmitData8(SPI2, 0x03);
	/* Send the block number */
	LL_SPI_TransmitData8(SPI2, blockNum);
	/* Send the buffer length */
	LL_SPI_TransmitData8(SPI2, 18);
	/* Receive the data from the block */
	for(int i = 0; i < 18; i++)
	{
			readBlockData[i] = LL_SPI_ReceiveData8(SPI2);
	}
	/* Wait until the transmission is complete */
	while(!LL_SPI_IsActiveFlag_RXNE(SPI2));
	/* Set SS (PB10) pin high to deselect the RC522 module */
	LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_10);
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
