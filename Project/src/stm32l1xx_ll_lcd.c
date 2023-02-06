/**
  ******************************************************************************
  * @file    stm32l1xx_ll_lcd.c
  * @author  Mod by GORAGOD TNI
  * @brief   LCD Controller HAL module driver.
  *          This file provides firmware functions to manage the following 
  *          functionalities of the LCD Controller (LCD) peripheral:
  *           + Initialization/de-initialization methods
  *           + I/O operation methods
  *           + Peripheral State methods
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_ll_lcd.h"

/** @addtogroup STM32L1xx_HAL_Driver
  * @{
  */


#if defined (STM32L100xB) || defined (STM32L100xBA) || defined (STM32L100xC) ||\
    defined (STM32L152xB) || defined (STM32L152xBA) || defined (STM32L152xC) || defined (STM32L152xCA) || defined (STM32L152xD) || defined (STM32L152xE) || defined (STM32L152xDX) ||\
    defined (STM32L162xC) || defined (STM32L162xCA) || defined (STM32L162xD) || defined (STM32L162xE) || defined (STM32L162xDX)

/** @defgroup LCD LCD
  * @brief LCD HAL module driver
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/** @defgroup LCD_Private_Defines LCD Private Defines
  * @{
  */

#define LCD_TIMEOUT_VALUE             1000

/**
  * @}
  */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup LCD_Exported_Functions LCD Exported Functions
  * @{
  */

/** @defgroup LCD_Exported_Functions_Group1 Initialization/de-initialization methods 
  *  @brief    Initialization and Configuration functions 
  *
@verbatim    
===============================================================================
            ##### Initialization and Configuration functions #####
 ===============================================================================  
    [..]

@endverbatim
  * @{
  */
 
/**
  * @brief  DeInitializes the LCD peripheral. 
  * @param  hlcd: LCD handle
  * @retval HAL status
  */
void LL_LCD_DeInit(void)
{


  /* Disable the peripheral */
  __LL_LCD_DISABLE();

  /*Disable Highdrive by default*/
  __LL_LCD_HIGHDRIVER_DISABLE();
  
  /* DeInit the low level hardware */
  LL_LCD_MspDeInit(); 
}

/**
  * @brief  Initializes the LCD peripheral according to the specified parameters 
  *         in the LCD_InitStruct.
  * @note   This function can be used only when the LCD is disabled.  
  *         The LCD HighDrive can be enabled/disabled using related macros up to user.
  * @param  hlcd: LCD handle
  * @retval None
  */
LL_LCD_StateTypeDef LL_LCD_Init(LCD_InitTypeDef* instance)
{
  uint8_t counter = 0;
    
  
  /* Check function parameters */
	//EDITED Skipped for now
  
  
  /* Clear the LCD_RAM registers and enable the display request by setting the UDR bit
     in the LCD_SR register */
  for(counter = LCD_RAM_REGISTER0; counter <= LCD_RAM_REGISTER15; counter++)
  {
    LCD->RAM[counter] = 0;
  }
  /* Enable the display request */
  SET_BIT(LCD->SR, LCD_SR_UDR);
  
  /* Configure the LCD Prescaler, Divider, Blink mode and Blink Frequency: 
     Set PS[3:0] bits according to instance->Prescaler value 
     Set DIV[3:0] bits according to instance->Divider value
     Set BLINK[1:0] bits according to instance->BlinkMode value
     Set BLINKF[2:0] bits according to instance->BlinkFrequency value
     Set DEAD[2:0] bits according to instance->DeadTime value
     Set PON[2:0] bits according to instance->PulseOnDuration value 
     Set CC[2:0] bits according to instance->Contrast value
     Set HD[0] bit according to instance->HighDrive value */
   MODIFY_REG(LCD->FCR, \
      (LCD_FCR_PS | LCD_FCR_DIV | LCD_FCR_BLINK| LCD_FCR_BLINKF | \
       LCD_FCR_DEAD | LCD_FCR_PON | LCD_FCR_CC), \
      (instance->Prescaler | instance->Divider | instance->BlinkMode | instance->BlinkFrequency | \
             instance->DeadTime | instance->PulseOnDuration | instance->Contrast | instance->HighDrive));

  /* Wait until LCD Frame Control Register Synchronization flag (FCRSF) is set in the LCD_SR register 
     This bit is set by hardware each time the LCD_FCR register is updated in the LCDCLK
     domain. It is cleared by hardware when writing to the LCD_FCR register.*/
  LCD_WaitForSynchro();
  
  /* Configure the LCD Duty, Bias, Voltage Source, Dead Time:
     Set DUTY[2:0] bits according to instance->Duty value 
     Set BIAS[1:0] bits according to instance->Bias value
     Set VSEL bit according to instance->VoltageSource value
     Set MUX_SEG bit according to instance->MuxSegment value */
  MODIFY_REG(LCD->CR, \
    (LCD_CR_DUTY | LCD_CR_BIAS | LCD_CR_VSEL | LCD_CR_MUX_SEG), \
    (instance->Duty | instance->Bias | instance->VoltageSource | instance->MuxSegment));
  
  /* Enable the peripheral */
  __LL_LCD_ENABLE();
  
  /* Get timeout */
  //tickstart = HAL_GetTick();
      
  /* Wait Until the LCD is enabled */
  while(__LL_LCD_GET_FLAG(LCD_FLAG_ENS) == RESET)
  {
  }
  
  
  /*!< Wait Until the LCD Booster is ready */
  while(__LL_LCD_GET_FLAG(LCD_FLAG_RDY) == RESET)
  {
  }

  return LL_LCD_STATE_READY;
}

/**
  * @brief  LCD MSP DeInit.
  * @param  hlcd: LCD handle
  * @retval None
  */
 __weak void LL_LCD_MspDeInit(void)
{

  /* NOTE: This function Should not be modified, when the callback is needed,
           the HAL_LCD_MspDeInit could be implemented in the user file
   */ 
}

/**
  * @brief  LCD MSP Init.
  * @param  hlcd: LCD handle
  * @retval None
  */
 __weak void LL_LCD_MspInit(void)
{

  /* NOTE: This function Should not be modified, when the callback is needed,
           the HAL_LCD_MspInit could be implemented in the user file
   */ 
}

/**
  * @}
  */

/** @defgroup LCD_Exported_Functions_Group2 IO operation methods 
  *  @brief LCD RAM functions 
  *
@verbatim   
 ===============================================================================
                      ##### IO operation functions #####
 ===============================================================================  
 [..] Using its double buffer memory the LCD controller ensures the coherency of the 
 displayed information without having to use interrupts to control LCD_RAM 
 modification.
 (+)The application software can access the first buffer level (LCD_RAM) through 
 the APB interface. Once it has modified the LCD_RAM using the HAL_LCD_Write() API,
 it sets the UDR flag in the LCD_SR register using the HAL_LCD_UpdateDisplayRequest() API.
 This UDR flag (update display request) requests the updated information to be 
 moved into the second buffer level (LCD_DISPLAY).
 (+)This operation is done synchronously with the frame (at the beginning of the 
 next frame), until the update is completed, the LCD_RAM is write protected and 
 the UDR flag stays high.
 (+)Once the update is completed another flag (UDD - Update Display Done) is set and
 generates an interrupt if the UDDIE bit in the LCD_FCR register is set.
 The time it takes to update LCD_DISPLAY is, in the worst case, one odd and one 
 even frame.
 (+)The update will not occur (UDR = 1 and UDD = 0) until the display is 
 enabled (LCDEN = 1).
      
@endverbatim
  * @{
  */

/**
  * @brief  Writes a word in the specific LCD RAM.
  * @param  hlcd: LCD handle
  * @param  RAMRegisterIndex: specifies the LCD RAM Register.
  *   This parameter can be one of the following values:
  *     @arg LCD_RAM_REGISTER0: LCD RAM Register 0
  *     @arg LCD_RAM_REGISTER1: LCD RAM Register 1
  *     @arg LCD_RAM_REGISTER2: LCD RAM Register 2
  *     @arg LCD_RAM_REGISTER3: LCD RAM Register 3
  *     @arg LCD_RAM_REGISTER4: LCD RAM Register 4
  *     @arg LCD_RAM_REGISTER5: LCD RAM Register 5
  *     @arg LCD_RAM_REGISTER6: LCD RAM Register 6 
  *     @arg LCD_RAM_REGISTER7: LCD RAM Register 7  
  *     @arg LCD_RAM_REGISTER8: LCD RAM Register 8
  *     @arg LCD_RAM_REGISTER9: LCD RAM Register 9
  *     @arg LCD_RAM_REGISTER10: LCD RAM Register 10
  *     @arg LCD_RAM_REGISTER11: LCD RAM Register 11
  *     @arg LCD_RAM_REGISTER12: LCD RAM Register 12 
  *     @arg LCD_RAM_REGISTER13: LCD RAM Register 13 
  *     @arg LCD_RAM_REGISTER14: LCD RAM Register 14 
  *     @arg LCD_RAM_REGISTER15: LCD RAM Register 15
  * @param  RAMRegisterMask: specifies the LCD RAM Register Data Mask.
  * @param  Data: specifies LCD Data Value to be written.
  * @retval None
  */
LL_LCD_StateTypeDef LL_LCD_Write(uint32_t RAMRegisterIndex, uint32_t RAMRegisterMask, uint32_t Data)
{   
    /*!< Wait Until the LCD is ready */
    while(__LL_LCD_GET_FLAG(LCD_FLAG_UDR) != RESET)
    {
    }

    /* Copy the new Data bytes to LCD RAM register */
    MODIFY_REG(LCD->RAM[RAMRegisterIndex], ~(RAMRegisterMask), Data);
	
    return LL_LCD_STATE_READY;
}


/**
  * @brief Clears the LCD RAM registers.
  * @param hlcd: LCD handle
  * @retval None
  */
LL_LCD_StateTypeDef LL_LCD_Clear()
{

  uint32_t counter = 0;
  
  /*!< Wait Until the LCD is ready */
  while(__LL_LCD_GET_FLAG(LCD_FLAG_UDR) != RESET)
  {
  }
	/* Clear the LCD_RAM registers */
  for(counter = LCD_RAM_REGISTER0; counter <= LCD_RAM_REGISTER15; counter++)
  {
     LCD->RAM[counter] = 0;
  }
    
    /* Update the LCD display */
    LL_LCD_UpdateDisplayRequest();     
    
    return LL_LCD_STATE_READY;
}



/**
  * @brief  Enables the Update Display Request.
  * @param  hlcd: LCD handle
  * @note   Each time software modifies the LCD_RAM it must set the UDR bit to 
  *         transfer the updated data to the second level buffer. 
  *         The UDR bit stays set until the end of the update and during this 
  *         time the LCD_RAM is write protected. 
  * @note   When the display is disabled, the update is performed for all 
  *         LCD_DISPLAY locations.
  *         When the display is enabled, the update is performed only for locations 
  *         for which commons are active (depending on DUTY). For example if 
  *         DUTY = 1/2, only the LCD_DISPLAY of COM0 and COM1 will be updated.    
  * @retval None
  */
LL_LCD_StateTypeDef LL_LCD_UpdateDisplayRequest(void)
{
  
  /* Clear the Update Display Done flag before starting the update display request */
  __LL_LCD_CLEAR_FLAG(LCD_FLAG_UDD);
  
  /* Enable the display request */
  LCD->SR |= LCD_SR_UDR;

  /*!< Wait Until the LCD display is done */
  while(__LL_LCD_GET_FLAG(LCD_FLAG_UDD) == RESET)
  {
  }
  
  return LL_LCD_STATE_READY;
}

/**
  * @}
  */

/**
  * @}
  */
  
/** @defgroup LCD_Private_Functions LCD Private Functions
  * @{
  */

/**
  * @brief  Waits until the LCD FCR register is synchronized in the LCDCLK domain.
  *   This function must be called after any write operation to LCD_FCR register.
  * @retval None
  */
LL_LCD_StateTypeDef LCD_WaitForSynchro(void)
{

  /* Loop until FCRSF flag is set */
  while(__LL_LCD_GET_FLAG(LCD_FLAG_FCRSF) == RESET)
  {
  }

  return LL_LCD_STATE_READY;
}

/**
  * @}
  */

/**
  * @}
  */

#endif /* STM32L100xB || STM32L100xBA || STM32L100xC ||... || STM32L162xD || STM32L162xE || STM32L162xDX */


/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

