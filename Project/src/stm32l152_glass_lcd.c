/**
  ******************************************************************************
  * @file    stm32l152_lcd.c
  * @author  GORAGOD (powered by MCD Application Team)
  * @brief   This file includes the LCD Glass driver for LCD Module of 
  *          STM32L152C-Discovery board.
  ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32l152_glass_lcd.h"
#include "stm32l1xx_ll_lcd.h"

#include "stm32l1xx_ll_system.h"
#include "stm32l1xx_ll_bus.h"
#include "stm32l1xx_ll_utils.h"
#include "stm32l1xx_ll_rcc.h"
#include "stm32l1xx_ll_pwr.h"
#include "stm32l1xx_ll_gpio.h"
/** @defgroup STM32L152C-Discovery_GLASS_LCD_Private_Defines Private Defines
  * @{
  */
#define ASCII_CHAR_0                  0x30  /* 0 */
#define ASCII_CHAR_AT_SYMBOL          0x40  /* @ */
#define ASCII_CHAR_LEFT_OPEN_BRACKET  0x5B  /* [ */
#define ASCII_CHAR_APOSTROPHE         0x60  /* ` */
#define ASCII_CHAR_LEFT_OPEN_BRACE    0x7B  /* ( */
/**
  * @}
  */   


/** @defgroup STM32L152C-Discovery_GLASS_LCD_Private_Variables Private Variables
  * @{
  */ 

/* this variable can be used for accelerate the scrolling exit when push user button */
__IO uint8_t bLCDGlass_KeyPressed = 0; 

/**
  @verbatim
================================================================================
                              GLASS LCD MAPPING
================================================================================
LCD allows to display informations on six 14-segment digits and 4 bars:

  1       2       3       4       5       6
-----   -----   -----   -----   -----   -----   
|\|/| o |\|/| o |\|/| o |\|/| o |\|/|   |\|/|   BAR3
-- --   -- --   -- --   -- --   -- --   -- --   BAR2
|/|\| o |/|\| o |/|\| o |/|\| o |/|\|   |/|\|   BAR1
----- * ----- * ----- * ----- * -----   -----   BAR0

LCD segment mapping:
--------------------
  -----A-----        _ 
  |\   |   /|   COL |_|
  F H  J  K B          
  |  \ | /  |        _ 
  --G-- --M--   COL |_|
  |  / | \  |          
  E Q  P  N C          
  |/   |   \|        _ 
  -----D-----   DP  |_|

 An LCD character coding is based on the following matrix:
COM           0   1   2     3
SEG(n)      { E , D , P ,   N   }
SEG(n+1)    { M , C , COL , DP  }
SEG(23-n-1) { B , A , K ,   J   }
SEG(23-n)   { G , F , Q ,   H   }
with n positif odd number.

 The character 'A' for example is:
  -------------------------------
LSB   { 1 , 0 , 0 , 0   }
      { 1 , 1 , 0 , 0   }
      { 1 , 1 , 0 , 0   }
MSB   { 1 , 1 , 0 , 0   }
      -------------------
  'A' =  F    E   0   0 hexa

  @endverbatim
*/

LCD_InitTypeDef LCD_InitStructure;

/* Constant table for cap characters 'A' --> 'Z' */
const uint16_t CapLetterMap[26]=
    {
        /* A      B      C      D      E      F      G      H      I  */
        0xFE00, 0x6714, 0x1D00, 0x4714, 0x9D00, 0x9C00, 0x3F00, 0xFA00, 0x0014,
        /* J      K      L      M      N      O      P      Q      R  */
        0x5300, 0x9841, 0x1900, 0x5A48, 0x5A09, 0x5F00, 0xFC00, 0x5F01, 0xFC01,
        /* S      T      U      V      W      X      Y      Z  */
        0xAF00, 0x0414, 0x5b00, 0x18C0, 0x5A81, 0x00C9, 0x0058, 0x05C0
    };

/* Constant table for number '0' --> '9' */
const uint16_t NumberMap[10]=
    {
        /* 0      1      2      3      4      5      6      7      8      9  */
        0x5F00,0x4200,0xF500,0x6700,0xEa00,0xAF00,0xBF00,0x04600,0xFF00,0xEF00
    };

uint32_t Digit[4];     /* Digit frame buffer */

/* LCD BAR status: To save the bar setting after writing in LCD RAM memory */
uint8_t LCDBar = BATTERYLEVEL_FULL;

/**
  * @}
  */

/** @defgroup STM32L152C-Discovery_LCD_Private_Functions Private Functions
  * @{
  */
static void Convert(uint8_t* Char, Point_Typedef Point, DoublePoint_Typedef DoublePoint);
static void LCD_MspInit(void);

		
/**
  * @}
  */ 

/** @defgroup STM32L152C-Discovery_LCD_Exported_Functions Exported Functions
  * @{
  */ 

/**
  * @brief  Configures the LCD GLASS relative GPIO port IOs and LCD peripheral.
  * @retval None
  */
void LCD_GLASS_Init(void)
{
  LCD_InitStructure.Prescaler        = LCD_PRESCALER_1;
  LCD_InitStructure.Divider          = LCD_DIVIDER_31;
  LCD_InitStructure.Duty             = LCD_DUTY_1_4;
  LCD_InitStructure.Bias             = LCD_BIAS_1_3;
  LCD_InitStructure.VoltageSource    = LCD_VOLTAGESOURCE_INTERNAL;
  LCD_InitStructure.Contrast         = LCD_CONTRASTLEVEL_5;
  LCD_InitStructure.DeadTime         = LCD_DEADTIME_0;
  LCD_InitStructure.PulseOnDuration  = LCD_PULSEONDURATION_4;
  LCD_InitStructure.BlinkMode        = LCD_BLINKMODE_OFF;
  LCD_InitStructure.BlinkFrequency   = LCD_BLINKFREQUENCY_DIV32;
  LCD_InitStructure.MuxSegment       = LCD_MUXSEGMENT_ENABLE;
  /* Initialize the LCD */
  LCD_MspInit();
  LL_LCD_Init(&LCD_InitStructure);

  LCD_GLASS_Clear();
}

/**
  * @brief  Configures the LCD Blink mode and Blink frequency.
  * @param  BlinkMode: specifies the LCD blink mode.
  *   This parameter can be one of the following values:
  *     @arg LCD_BLINKMODE_OFF:           Blink disabled
  *     @arg LCD_BLINKMODE_SEG0_COM0:     Blink enabled on SEG[0], COM[0] (1 pixel)
  *     @arg LCD_BLINKMODE_SEG0_ALLCOM:   Blink enabled on SEG[0], all COM (up to 8 
  *                                       pixels according to the programmed duty)
  *     @arg LCD_BLINKMODE_ALLSEG_ALLCOM: Blink enabled on all SEG and all COM 
  *                                       (all pixels)
  * @param  BlinkFrequency: specifies the LCD blink frequency.
  *     @arg LCD_BLINKFREQUENCY_DIV8:    The Blink frequency = fLcd/8
  *     @arg LCD_BLINKFREQUENCY_DIV16:   The Blink frequency = fLcd/16
  *     @arg LCD_BLINKFREQUENCY_DIV32:   The Blink frequency = fLcd/32
  *     @arg LCD_BLINKFREQUENCY_DIV64:   The Blink frequency = fLcd/64 
  *     @arg LCD_BLINKFREQUENCY_DIV128:  The Blink frequency = fLcd/128
  *     @arg LCD_BLINKFREQUENCY_DIV256:  The Blink frequency = fLcd/256
  *     @arg LCD_BLINKFREQUENCY_DIV512:  The Blink frequency = fLcd/512
  *     @arg LCD_BLINKFREQUENCY_DIV1024: The Blink frequency = fLcd/1024
  * @retval None
  */
void LCD_GLASS_BlinkConfig(uint32_t BlinkMode, uint32_t BlinkFrequency)
{
  __LL_LCD_BLINK_CONFIG(BlinkMode, BlinkFrequency);
}

/**
  * @brief  LCD contrast setting
  * @param  Contrast: specifies the LCD Contrast.
  *   This parameter can be one of the following values:
  *     @arg LCD_CONTRASTLEVEL_0: Maximum Voltage = 2.60V
  *     @arg LCD_CONTRASTLEVEL_1: Maximum Voltage = 2.73V
  *     @arg LCD_CONTRASTLEVEL_2: Maximum Voltage = 2.86V
  *     @arg LCD_CONTRASTLEVEL_3: Maximum Voltage = 2.99V
  *     @arg LCD_CONTRASTLEVEL_4: Maximum Voltage = 3.12V
  *     @arg LCD_CONTRASTLEVEL_5: Maximum Voltage = 3.25V
  *     @arg LCD_CONTRASTLEVEL_6: Maximum Voltage = 3.38V
  *     @arg LCD_CONTRASTLEVEL_7: Maximum Voltage = 3.51V
  * @retval None
  */
void LCD_GLASS_Contrast(uint32_t Contrast)
{
  __LL_LCD_CONTRAST_CONFIG(Contrast);
}

/**
  * @brief Display one or several bar on LCD frame buffer 
  * @param BarId: specifies the LCD GLASS Bar to display
  *     This parameter can be one of the following values:
  *     @arg LCD_BAR_0: LCD GLASS Bar 0
  *     @arg LCD_BAR_1: LCD GLASS Bar 1
  *     @arg LCD_BAR_2: LCD GLASS Bar 2
  *     @arg LCD_BAR_3: LCD GLASS Bar 3
  * @retval None
  */
void LCD_GLASS_DisplayBar(uint32_t BarId)
{
  uint32_t position = 0;

  /* Check which bar is selected */
  while ((BarId) >> position)
  {
    /* Check if current bar is selected */
    switch(BarId & (1 << position))
    {
      /* Bar 0 */
      case LCD_BAR_0:
        /* Set BAR0 */
        LL_LCD_Write(LCD_BAR0_2_COM, ~(LCD_BAR0_SEG), LCD_BAR0_SEG);
        break;
        
      /* Bar 1 */
      case LCD_BAR_1:
        /* Set BAR1 */
        LL_LCD_Write(LCD_BAR1_3_COM, ~(LCD_BAR1_SEG), LCD_BAR1_SEG);
        break;
        
      /* Bar 2 */
      case LCD_BAR_2:
        /* Set BAR2 */
        LL_LCD_Write(LCD_BAR0_2_COM, ~(LCD_BAR2_SEG), LCD_BAR2_SEG);
        break;
        
      /* Bar 3 */
      case LCD_BAR_3:
        /* Set BAR3 */
        LL_LCD_Write( LCD_BAR1_3_COM, ~(LCD_BAR3_SEG), LCD_BAR3_SEG);
        break;
        
      default:
        break;
    }
    position++;
  }
  
  /* Update the LCD display */
  LL_LCD_UpdateDisplayRequest();
}

/**
  * @brief Clear one or several bar on LCD frame buffer 
  * @param BarId: specifies the LCD GLASS Bar to display
  *     This parameter can be combination of one of the following values:
  *     @arg LCD_BAR_0: LCD GLASS Bar 0
  *     @arg LCD_BAR_1: LCD GLASS Bar 1
  *     @arg LCD_BAR_2: LCD GLASS Bar 2
  *     @arg LCD_BAR_3: LCD GLASS Bar 3
  * @retval None
  */
void LCD_GLASS_ClearBar(uint32_t BarId)
{
  uint32_t position = 0;

  /* Check which bar is selected */
  while ((BarId) >> position)
  {
    /* Check if current bar is selected */
    switch(BarId & (1 << position))
    {
      /* Bar 0 */
      case LCD_BAR_0:
        /* Set BAR0 */
        LL_LCD_Write( LCD_BAR0_2_COM, ~(LCD_BAR0_SEG) , 0);
        break;
        
      /* Bar 1 */
      case LCD_BAR_1:
        /* Set BAR1 */
        LL_LCD_Write( LCD_BAR1_3_COM, ~(LCD_BAR1_SEG), 0);
        break;
        
      /* Bar 2 */
      case LCD_BAR_2:
        /* Set BAR2 */
        LL_LCD_Write( LCD_BAR0_2_COM, ~(LCD_BAR2_SEG), 0);
        break;
        
      /* Bar 3 */
      case LCD_BAR_3:
        /* Set BAR3 */
        LL_LCD_Write( LCD_BAR1_3_COM, ~(LCD_BAR3_SEG), 0);
        break;
        
      default:
        break;
    }
    position++;
  }
  
  /* Update the LCD display */
  LL_LCD_UpdateDisplayRequest();
}

/**
  * @brief Setting bar on LCD, writes bar value in LCD frame buffer 
  * @param BarLevel: specifies the LCD GLASS Batery Level.
  *     This parameter can be one of the following values:
  *     @arg BATTERYLEVEL_OFF: LCD GLASS Batery Empty
  *     @arg BATTERYLEVEL_1_4: LCD GLASS Batery 1/4 Full
  *     @arg BATTERYLEVEL_1_2: LCD GLASS Batery 1/2 Full
  *     @arg BATTERYLEVEL_3_4: LCD GLASS Batery 3/4 Full
  *     @arg BATTERYLEVEL_FULL: LCD GLASS Batery Full
  * @retval None
  */
void LCD_GLASS_BarLevelConfig(uint8_t BarLevel)
{
  switch (BarLevel)
  {
  /* BATTERYLEVEL_OFF */
  case BATTERYLEVEL_OFF:
    /* Set BAR0 & BAR2 off */
    LL_LCD_Write( LCD_BAR0_2_COM, ~(LCD_BAR0_SEG | LCD_BAR2_SEG), 0);
    /* Set BAR1 & BAR3 off */
    LL_LCD_Write( LCD_BAR1_3_COM, ~(LCD_BAR1_SEG | LCD_BAR3_SEG), 0);
    LCDBar = BATTERYLEVEL_OFF;
    break;
    
  /* BARLEVEL 1/4 */
  case BATTERYLEVEL_1_4:
    /* Set BAR0 on & BAR2 off */
    LL_LCD_Write( LCD_BAR0_2_COM, ~(LCD_BAR0_SEG | LCD_BAR2_SEG), LCD_BAR0_SEG);
    /* Set BAR1 & BAR3 off */
    LL_LCD_Write( LCD_BAR1_3_COM, ~(LCD_BAR1_SEG | LCD_BAR3_SEG), 0);
    LCDBar = BATTERYLEVEL_1_4;
    break;
    
  /* BARLEVEL 1/2 */
  case BATTERYLEVEL_1_2:
    /* Set BAR0 on & BAR2 off */
    LL_LCD_Write( LCD_BAR0_2_COM, ~(LCD_BAR0_SEG | LCD_BAR2_SEG), LCD_BAR0_SEG);
    /* Set BAR1 on & BAR3 off */
    LL_LCD_Write( LCD_BAR1_3_COM, ~(LCD_BAR1_SEG | LCD_BAR3_SEG), LCD_BAR1_SEG);
    LCDBar = BATTERYLEVEL_1_2;
    break;
    
  /* Battery Level 3/4 */
  case BATTERYLEVEL_3_4:
    /* Set BAR0 & BAR2 on */
    LL_LCD_Write( LCD_BAR0_2_COM, ~(LCD_BAR0_SEG | LCD_BAR2_SEG), (LCD_BAR0_SEG | LCD_BAR2_SEG));
    /* Set BAR1 on & BAR3 off */
    LL_LCD_Write( LCD_BAR1_3_COM, ~(LCD_BAR1_SEG | LCD_BAR3_SEG), LCD_BAR1_SEG);
    LCDBar = BATTERYLEVEL_3_4;
    break;
    
  /* BATTERYLEVEL_FULL */
  case BATTERYLEVEL_FULL:
    /* Set BAR0 & BAR2 on */
    LL_LCD_Write( LCD_BAR0_2_COM, ~(LCD_BAR0_SEG | LCD_BAR2_SEG), (LCD_BAR0_SEG | LCD_BAR2_SEG));
    /* Set BAR1 on & BAR3 on */
    LL_LCD_Write( LCD_BAR1_3_COM, ~(LCD_BAR1_SEG | LCD_BAR3_SEG), (LCD_BAR1_SEG | LCD_BAR3_SEG));
    LCDBar = BATTERYLEVEL_FULL;
    break;
    
  default:
    break;
  }
  
  /* Update the LCD display */
  LL_LCD_UpdateDisplayRequest();
}

/**
  * @brief  This function writes a char in the LCD frame buffer and update BAR LEVEL.
  * @param  ch: the character to display.
  * @param Point: a point to add in front of char
  *         This parameter can be: POINT_OFF or POINT_ON
  * @param Column: flag indicating if a column has to be add in front
  *         of displayed character.
  *         This parameter can be: DOUBLEPOINT_OFF or DOUBLEPOINT_ON.           
  * @param Position: position in the LCD of the character to write (DigitPosition_Typedef)
  * @retval None
  * @note  Required preconditions: The LCD should be cleared before to start the
  *         write operation.  
  */
void LCD_GLASS_WriteChar(uint8_t* ch, uint8_t Point, uint8_t Column, uint8_t Position)
{
  LCD_GLASS_DisplayChar(ch, (Point_Typedef)Point, (DoublePoint_Typedef)Column, (DigitPosition_Typedef)Position);
}

/**
  * @brief  This function writes a char in the LCD RAM and do not update BAR level.
  * @param  ch: The character to display.
  * @param  Point: A point to add in front of char.
  *          This parameter  can be one of the following values:  
  *              @arg POINT_OFF: No point to add in front of char.
  *              @arg POINT_ON: Add a point in front of char.
  * @param  Column: Flag indicating if a apostrophe has to be add in front 
  *                     of displayed character.
  *          This parameter  can be one of the following values:
  *              @arg DOUBLEPOINT_OFF: No colon to add in back of char.
  *              @arg DOUBLEPOINT_ON: Add an colon in back of char.
  * @param  Position: Position in the LCD of the caracter to write.
  *                   This parameter can be any value in DigitPosition_Typedef.
  * @retval None
  */
void LCD_GLASS_DisplayChar(uint8_t* ch, Point_Typedef Point, DoublePoint_Typedef Column, DigitPosition_Typedef Position)
{
  uint32_t data =0x00;
  /* To convert displayed character in segment in array digit */
  Convert(ch, (Point_Typedef)Point, (DoublePoint_Typedef)Column);

  switch (Position)
  {
    /* Position 1 on LCD (Digit1)*/
    case LCD_DIGIT_POSITION_1:
      data = ((Digit[0] & 0x1) << LCD_SEG0_SHIFT) | (((Digit[0] & 0x2) >> 1) << LCD_SEG1_SHIFT)
          | (((Digit[0] & 0x4) >> 2) << LCD_SEG22_SHIFT) | (((Digit[0] & 0x8) >> 3) << LCD_SEG23_SHIFT);
      LL_LCD_Write( LCD_DIGIT1_COM0, LCD_DIGIT1_COM0_SEG_MASK, data); /* 1G 1B 1M 1E */
      
      data = ((Digit[1] & 0x1) << LCD_SEG0_SHIFT) | (((Digit[1] & 0x2) >> 1) << LCD_SEG1_SHIFT)
          | (((Digit[1] & 0x4) >> 2) << LCD_SEG22_SHIFT) | (((Digit[1] & 0x8) >> 3) << LCD_SEG23_SHIFT);
      LL_LCD_Write( LCD_DIGIT1_COM1, LCD_DIGIT1_COM1_SEG_MASK, data) ; /* 1F 1A 1C 1D  */
      
      data = ((Digit[2] & 0x1) << LCD_SEG0_SHIFT) | (((Digit[2] & 0x2) >> 1) << LCD_SEG1_SHIFT)
          | (((Digit[2] & 0x4) >> 2) << LCD_SEG22_SHIFT) | (((Digit[2] & 0x8) >> 3) << LCD_SEG23_SHIFT);
      LL_LCD_Write( LCD_DIGIT1_COM2, LCD_DIGIT1_COM2_SEG_MASK, data) ; /* 1Q 1K 1Col 1P  */
      
      data = ((Digit[3] & 0x1) << LCD_SEG0_SHIFT) | (((Digit[3] & 0x2) >> 1) << LCD_SEG1_SHIFT)
          | (((Digit[3] & 0x4) >> 2) << LCD_SEG22_SHIFT) | (((Digit[3] & 0x8) >> 3) << LCD_SEG23_SHIFT);
      LL_LCD_Write( LCD_DIGIT1_COM3, LCD_DIGIT1_COM3_SEG_MASK, data) ; /* 1H 1J 1DP 1N  */
      break;

    /* Position 2 on LCD (Digit2)*/
    case LCD_DIGIT_POSITION_2:
      data = ((Digit[0] & 0x1) << LCD_SEG2_SHIFT) | (((Digit[0] & 0x2) >> 1) << LCD_SEG3_SHIFT)
          | (((Digit[0] & 0x4) >> 2) << LCD_SEG20_SHIFT) | (((Digit[0] & 0x8) >> 3) << LCD_SEG21_SHIFT);
      LL_LCD_Write( LCD_DIGIT2_COM0, LCD_DIGIT2_COM0_SEG_MASK, data); /* 2G 2B 2M 2E */
      
      data = ((Digit[1] & 0x1) << LCD_SEG2_SHIFT) | (((Digit[1] & 0x2) >> 1) << LCD_SEG3_SHIFT)
          | (((Digit[1] & 0x4) >> 2) << LCD_SEG20_SHIFT) | (((Digit[1] & 0x8) >> 3) << LCD_SEG21_SHIFT);
      LL_LCD_Write( LCD_DIGIT2_COM1, LCD_DIGIT2_COM1_SEG_MASK, data) ; /* 2F 2A 2C 2D  */
      
      data = ((Digit[2] & 0x1) << LCD_SEG2_SHIFT) | (((Digit[2] & 0x2) >> 1) << LCD_SEG3_SHIFT)
          | (((Digit[2] & 0x4) >> 2) << LCD_SEG20_SHIFT) | (((Digit[2] & 0x8) >> 3) << LCD_SEG21_SHIFT);
      LL_LCD_Write( LCD_DIGIT2_COM2, LCD_DIGIT2_COM2_SEG_MASK, data) ; /* 2Q 2K 2Col 2P  */
      
      data = ((Digit[3] & 0x1) << LCD_SEG2_SHIFT) | (((Digit[3] & 0x2) >> 1) << LCD_SEG3_SHIFT)
          | (((Digit[3] & 0x4) >> 2) << LCD_SEG20_SHIFT) | (((Digit[3] & 0x8) >> 3) << LCD_SEG21_SHIFT);
      LL_LCD_Write( LCD_DIGIT2_COM3, LCD_DIGIT2_COM3_SEG_MASK, data) ; /* 2H 2J 2DP 2N  */
      break;
    
    /* Position 3 on LCD (Digit3)*/
    case LCD_DIGIT_POSITION_3:
      data = ((Digit[0] & 0x1) << LCD_SEG4_SHIFT) | (((Digit[0] & 0x2) >> 1) << LCD_SEG5_SHIFT)
          | (((Digit[0] & 0x4) >> 2) << LCD_SEG18_SHIFT) | (((Digit[0] & 0x8) >> 3) << LCD_SEG19_SHIFT);
      LL_LCD_Write( LCD_DIGIT3_COM0, LCD_DIGIT3_COM0_SEG_MASK, data); /* 3G 3B 3M 3E */
      
      data = ((Digit[1] & 0x1) << LCD_SEG4_SHIFT) | (((Digit[1] & 0x2) >> 1) << LCD_SEG5_SHIFT)
          | (((Digit[1] & 0x4) >> 2) << LCD_SEG18_SHIFT) | (((Digit[1] & 0x8) >> 3) << LCD_SEG19_SHIFT);
      LL_LCD_Write( LCD_DIGIT3_COM1, LCD_DIGIT3_COM1_SEG_MASK, data) ; /* 3F 3A 3C 3D  */
      
      data = ((Digit[2] & 0x1) << LCD_SEG4_SHIFT) | (((Digit[2] & 0x2) >> 1) << LCD_SEG5_SHIFT)
          | (((Digit[2] & 0x4) >> 2) << LCD_SEG18_SHIFT) | (((Digit[2] & 0x8) >> 3) << LCD_SEG19_SHIFT);
      LL_LCD_Write( LCD_DIGIT3_COM2, LCD_DIGIT3_COM2_SEG_MASK, data) ; /* 3Q 3K 3Col 3P  */
      
      data = ((Digit[3] & 0x1) << LCD_SEG4_SHIFT) | (((Digit[3] & 0x2) >> 1) << LCD_SEG5_SHIFT)
          | (((Digit[3] & 0x4) >> 2) << LCD_SEG18_SHIFT) | (((Digit[3] & 0x8) >> 3) << LCD_SEG19_SHIFT);
      LL_LCD_Write( LCD_DIGIT3_COM3, LCD_DIGIT3_COM3_SEG_MASK, data) ; /* 3H 3J 3DP 3N  */
      break;
    
    /* Position 4 on LCD (Digit4)*/
    case LCD_DIGIT_POSITION_4:
      data = ((Digit[0] & 0x1) << LCD_SEG6_SHIFT) | (((Digit[0] & 0x8) >> 3) << LCD_SEG17_SHIFT);
      LL_LCD_Write( LCD_DIGIT4_COM0, LCD_DIGIT4_COM0_SEG_MASK, data); /* 4G 4B 4M 4E */
      
      data = (((Digit[0] & 0x2) >> 1) << LCD_SEG7_SHIFT) | (((Digit[0] & 0x4) >> 2) << LCD_SEG16_SHIFT);
      LL_LCD_Write( LCD_DIGIT4_COM0_1, LCD_DIGIT4_COM0_1_SEG_MASK, data); /* 4G 4B 4M 4E */
      
      data = ((Digit[1] & 0x1) << LCD_SEG6_SHIFT) | (((Digit[1] & 0x8) >> 3) << LCD_SEG17_SHIFT);
      LL_LCD_Write( LCD_DIGIT4_COM1, LCD_DIGIT4_COM1_SEG_MASK, data) ; /* 4F 4A 4C 4D  */
      
      data = (((Digit[1] & 0x2) >> 1) << LCD_SEG7_SHIFT) | (((Digit[1] & 0x4) >> 2) << LCD_SEG16_SHIFT);
      LL_LCD_Write( LCD_DIGIT4_COM1_1, LCD_DIGIT4_COM1_1_SEG_MASK, data) ; /* 4F 4A 4C 4D  */
      
      data = ((Digit[2] & 0x1) << LCD_SEG6_SHIFT) | (((Digit[2] & 0x8) >> 3) << LCD_SEG17_SHIFT);
      LL_LCD_Write( LCD_DIGIT4_COM2, LCD_DIGIT4_COM2_SEG_MASK, data) ; /* 4Q 4K 4Col 4P  */
      
      data = (((Digit[2] & 0x2) >> 1) << LCD_SEG7_SHIFT) | (((Digit[2] & 0x4) >> 2) << LCD_SEG16_SHIFT);
      LL_LCD_Write( LCD_DIGIT4_COM2_1, LCD_DIGIT4_COM2_1_SEG_MASK, data) ; /* 4Q 4K 4Col 4P  */
      
      data = ((Digit[3] & 0x1) << LCD_SEG6_SHIFT) | (((Digit[3] & 0x8) >> 3) << LCD_SEG17_SHIFT);
      LL_LCD_Write( LCD_DIGIT4_COM3, LCD_DIGIT4_COM3_SEG_MASK, data) ; /* 4H 4J 4DP 4N  */
      
      data = (((Digit[3] & 0x2) >> 1) << LCD_SEG7_SHIFT) | (((Digit[3] & 0x4) >> 2) << LCD_SEG16_SHIFT);
      LL_LCD_Write( LCD_DIGIT4_COM3_1, LCD_DIGIT4_COM3_1_SEG_MASK, data) ; /* 4H 4J 4DP 4N  */
      break;
    
    /* Position 5 on LCD (Digit5)*/
    case LCD_DIGIT_POSITION_5:
       data = (((Digit[0] & 0x2) >> 1) << LCD_SEG9_SHIFT) | (((Digit[0] & 0x4) >> 2) << LCD_SEG14_SHIFT);
      LL_LCD_Write( LCD_DIGIT5_COM0, LCD_DIGIT5_COM0_SEG_MASK, data); /* 5G 5B 5M 5E */
      
      data = ((Digit[0] & 0x1) << LCD_SEG8_SHIFT) | (((Digit[0] & 0x8) >> 3) << LCD_SEG15_SHIFT);
      LL_LCD_Write( LCD_DIGIT5_COM0_1, LCD_DIGIT5_COM0_1_SEG_MASK, data); /* 5G 5B 5M 5E */
      
      data = (((Digit[1] & 0x2) >> 1) << LCD_SEG9_SHIFT) | (((Digit[1] & 0x4) >> 2) << LCD_SEG14_SHIFT);
      LL_LCD_Write( LCD_DIGIT5_COM1, LCD_DIGIT5_COM1_SEG_MASK, data) ; /* 5F 5A 5C 5D */
      
       data = ((Digit[1] & 0x1) << LCD_SEG8_SHIFT) | (((Digit[1] & 0x8) >> 3) << LCD_SEG15_SHIFT);
      LL_LCD_Write( LCD_DIGIT5_COM1_1, LCD_DIGIT5_COM1_1_SEG_MASK, data) ; /* 5F 5A 5C 5D */
      
      data = (((Digit[2] & 0x2) >> 1) << LCD_SEG9_SHIFT) | (((Digit[2] & 0x4) >> 2) << LCD_SEG14_SHIFT);
      LL_LCD_Write( LCD_DIGIT5_COM2, LCD_DIGIT5_COM2_SEG_MASK, data) ; /* 5Q 5K 5P */
      
      data = ((Digit[2] & 0x1) << LCD_SEG8_SHIFT) | (((Digit[2] & 0x8) >> 3) << LCD_SEG15_SHIFT);
      LL_LCD_Write( LCD_DIGIT5_COM2_1, LCD_DIGIT5_COM2_1_SEG_MASK, data) ; /* 5Q 5K 5P */
      
      data = (((Digit[3] & 0x2) >> 1) << LCD_SEG9_SHIFT) | (((Digit[3] & 0x4) >> 2) << LCD_SEG14_SHIFT);
      LL_LCD_Write( LCD_DIGIT5_COM3, LCD_DIGIT5_COM3_SEG_MASK, data) ; /* 5H 5J 5N */
      
      data = ((Digit[3] & 0x1) << LCD_SEG8_SHIFT) | (((Digit[3] & 0x8) >> 3) << LCD_SEG15_SHIFT);
      LL_LCD_Write( LCD_DIGIT5_COM3_1, LCD_DIGIT5_COM3_1_SEG_MASK, data) ; /* 5H 5J 5N */
      break;
    
    /* Position 6 on LCD (Digit6)*/
    case LCD_DIGIT_POSITION_6:
      data = ((Digit[0] & 0x1) << LCD_SEG10_SHIFT) | (((Digit[0] & 0x2) >> 1) << LCD_SEG11_SHIFT)
          | (((Digit[0] & 0x4) >> 2) << LCD_SEG12_SHIFT) | (((Digit[0] & 0x8) >> 3) << LCD_SEG13_SHIFT);
      LL_LCD_Write( LCD_DIGIT6_COM0, LCD_DIGIT6_COM0_SEG_MASK, data); /* 6G 6B 6M 6E */
      
      data = ((Digit[1] & 0x1) << LCD_SEG10_SHIFT) | (((Digit[1] & 0x2) >> 1) << LCD_SEG11_SHIFT)
          | (((Digit[1] & 0x4) >> 2) << LCD_SEG12_SHIFT) | (((Digit[1] & 0x8) >> 3) << LCD_SEG13_SHIFT);
      LL_LCD_Write( LCD_DIGIT6_COM1, LCD_DIGIT6_COM1_SEG_MASK, data) ; /* 6G 6B 6M 6E */
      
      data = ((Digit[2] & 0x1) << LCD_SEG10_SHIFT) | (((Digit[2] & 0x2) >> 1) << LCD_SEG11_SHIFT)
          | (((Digit[2] & 0x4) >> 2) << LCD_SEG12_SHIFT) | (((Digit[2] & 0x8) >> 3) << LCD_SEG13_SHIFT);
      LL_LCD_Write( LCD_DIGIT6_COM2, LCD_DIGIT6_COM2_SEG_MASK, data) ; /* 6Q 6K 6P */
      
      data = ((Digit[3] & 0x1) << LCD_SEG10_SHIFT) | (((Digit[3] & 0x2) >> 1) << LCD_SEG11_SHIFT)
          | (((Digit[3] & 0x4) >> 2) << LCD_SEG12_SHIFT) | (((Digit[3] & 0x8) >> 3) << LCD_SEG13_SHIFT);
      LL_LCD_Write(LCD_DIGIT6_COM3, LCD_DIGIT6_COM3_SEG_MASK, data) ; /* 6Q 6K 6P */
      break;
    
     default:
      break;
  }

  /* Update the LCD display */
  LL_LCD_UpdateDisplayRequest();
}

/**
  * @brief  This function writes a char in the LCD RAM.
  * @param  ptr: Pointer to string to display on the LCD Glass.
  * @retval None
  */
void LCD_GLASS_DisplayString(uint8_t* ptr)
{
  DigitPosition_Typedef position = LCD_DIGIT_POSITION_1;

  /* Send the string character by character on lCD */
  while ((*ptr != 0) & (position <= LCD_DIGIT_POSITION_6))
  {
    /* Display one character on LCD */
    LCD_GLASS_WriteChar(ptr, POINT_OFF, DOUBLEPOINT_OFF, position);

    /* Point on the next character */
    ptr++;

    /* Increment the character counter */
    position++;
  }
	LL_LCD_UpdateDisplayRequest();
}

/**
  * @brief  This function writes a char in the LCD RAM.
  * @param  ptr: Pointer to string to display on the LCD Glass.
  * @retval None
  * @note Required preconditions: Char is ASCCI value "Ored" with decimal point or Column flag
  */
void LCD_GLASS_DisplayStrDeci(uint16_t* ptr)
{
  DigitPosition_Typedef index = LCD_DIGIT_POSITION_1;
  uint8_t tmpchar = 0;
  
  /* Send the string character by character on lCD */
  while((*ptr != 0) & (index <= LCD_DIGIT_POSITION_6))
  {      
    tmpchar = (*ptr) & 0x00FF;
    
    switch((*ptr) & 0xF000)
    {
    case DOT:
      /* Display one character on LCD with decimal point */
      LCD_GLASS_WriteChar(&tmpchar, POINT_ON, DOUBLEPOINT_OFF, index);
      break;
    case DOUBLE_DOT:
      /* Display one character on LCD with decimal point */
      LCD_GLASS_WriteChar(&tmpchar, POINT_OFF, DOUBLEPOINT_ON, index);
      break;
    default:
      LCD_GLASS_WriteChar(&tmpchar, POINT_OFF, DOUBLEPOINT_OFF, index);    
      break;
    }/* Point on the next character */
    ptr++;
    
    /* Increment the character counter */
    index++;
  }
}

/**
  * @brief  This function Clear the whole LCD RAM.
  * @retval None
  */
void LCD_GLASS_Clear(void)
{
  LL_LCD_Clear(); 
}

/**
  * @brief  Display a string in scrolling mode
  * @param  ptr: Pointer to string to display on the LCD Glass.
  * @param  nScroll: Specifies how many time the message will be scrolled
  * @param  ScrollSpeed : Specifies the speed of the scroll, low value gives
  *         higher speed 
  * @retval None
  * @note    Required preconditions: The LCD should be cleared before to start the
  *         write operation.
  */
void LCD_GLASS_ScrollSentence(uint8_t* ptr, uint16_t nScroll, uint16_t ScrollSpeed)
{
  uint8_t repetition = 0, nbrchar = 0, sizestr = 0;
  uint8_t* ptr1;
  uint8_t str[6] = "";
  
  if(ptr == 0)
  {
    return;
  }
  
  /* To calculate end of string */
  for(ptr1 = ptr, sizestr = 0; *ptr1 != 0; sizestr++, ptr1++);
  
  ptr1 = ptr;
  
  LCD_GLASS_DisplayString(str);
  LL_mDelay(ScrollSpeed);
  
  /* To shift the string for scrolling display*/
  for (repetition = 0; repetition < nScroll; repetition++)
  {
    for(nbrchar = 0; nbrchar < sizestr; nbrchar++)
    {
      *(str) =* (ptr1+((nbrchar+1)%sizestr));
      *(str+1) =* (ptr1+((nbrchar+2)%sizestr));
      *(str+2) =* (ptr1+((nbrchar+3)%sizestr));
      *(str+3) =* (ptr1+((nbrchar+4)%sizestr));
      *(str+4) =* (ptr1+((nbrchar+5)%sizestr));
      *(str+5) =* (ptr1+((nbrchar+6)%sizestr));
      
      LCD_GLASS_DisplayString(str);
      LL_mDelay(ScrollSpeed);
			LCD_GLASS_Clear();
    }  
  }
}

/**
  * @}
  */

/** @addtogroup STM32L152C-Discovery_LCD_Private_Functions
  * @{
  */

/**
  * @brief  LCD MSP Init.
  * @param  hlcd: LCD handle
  * @retval None
  */
static void LCD_MspInit(void)
{
  LL_GPIO_InitTypeDef  gpioinitstruct = {0};
  
  /*##-1- Enable PWR  peripheral Clock #######################################*/
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  LL_PWR_EnableBkUpAccess();
  /*##-2- Configue LSE as RTC clock soucre ###################################*/
  /*##-3- select LSE as RTC clock source.##########################*/
  /* Backup domain management is done in RCC function */
  /* Enable LSE only if disabled.*/
  if (LL_RCC_LSE_IsReady() == 0)
  {
    LL_RCC_ForceBackupDomainReset();
    LL_RCC_ReleaseBackupDomainReset();
    LL_RCC_LSE_Enable();
    while (LL_RCC_LSE_IsReady() != 1);
    LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
    
    LL_RCC_EnableRTC();
  }

  /*##-4- Enable LCD GPIO Clocks #############################################*/
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
  
  /*##-5- Configure peripheral GPIO ##########################################*/
  /* Configure Output for LCD */
  /* Port A */
  gpioinitstruct.Pin        = LCD_GPIO_BANKA_PINS;
  gpioinitstruct.Mode       = LL_GPIO_MODE_ALTERNATE;
  gpioinitstruct.Pull       = LL_GPIO_PULL_NO;
  gpioinitstruct.Speed      = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  gpioinitstruct.Alternate  = LL_GPIO_AF_11;
  LL_GPIO_Init(GPIOA, &gpioinitstruct);

  /* Port B */
  gpioinitstruct.Pin        = LCD_GPIO_BANKB_PINS;
  LL_GPIO_Init(GPIOB, &gpioinitstruct);
  
  /* Port C*/
  gpioinitstruct.Pin        = LCD_GPIO_BANKC_PINS;
  LL_GPIO_Init(GPIOC, &gpioinitstruct);

  /* Wait for the external capacitor Cext which is connected to the VLCD pin is charged
  (approximately 2ms for Cext=1uF) */
  LL_mDelay(2);

  /*##-6- Enable LCD peripheral Clock ########################################*/
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_LCD);
}


/**
  @verbatim
================================================================================
                              GLASS LCD MAPPING
================================================================================
LCD allows to display informations on six 14-segment digits and 4 bars:

  1       2       3       4       5       6
-----   -----   -----   -----   -----   -----   
|\|/| o |\|/| o |\|/| o |\|/| o |\|/|   |\|/|   BAR3
-- --   -- --   -- --   -- --   -- --   -- --   BAR2
|/|\| o |/|\| o |/|\| o |/|\| o |/|\|   |/|\|   BAR1
----- * ----- * ----- * ----- * -----   -----   BAR0

LCD segment mapping:
--------------------
  -----A-----        _ 
  |\   |   /|   COL |_|
  F H  J  K B          
  |  \ | /  |        _ 
  --G-- --M--   COL |_|
  |  / | \  |          
  E Q  P  N C          
  |/   |   \|        _ 
  -----D-----   DP  |_|

 An LCD character coding is based on the following matrix:
COM           0   1   2     3
SEG(n)      { E , D , P ,   N   }
SEG(n+1)    { M , C , COL , DP  }
SEG(23-n-1) { B , A , K ,   J   }
SEG(23-n)   { G , F , Q ,   H   }
with n positif odd number.

 The character 'A' for example is:
  -------------------------------
LSB   { 1 , 0 , 0 , 0   }
      { 1 , 1 , 0 , 0   }
      { 1 , 1 , 0 , 0   }
MSB   { 1 , 1 , 0 , 0   }
      -------------------
  'A' =  F    E   0   0 hexa

  @endverbatim
*/
/**
  * @brief  Converts an ascii char to the a LCD digit.
  * @param  Char: a char to display.
  * @param  Point: a point to add in front of char
  *         This parameter can be: POINT_OFF or POINT_ON
  * @param  DoublePoint : flag indicating if a column has to be add in front
  *         of displayed character.
  *         This parameter can be: DOUBLEPOINT_OFF or DOUBLEPOINT_ON.
  * @retval None
  */
static void Convert(uint8_t* Char, Point_Typedef Point, DoublePoint_Typedef DoublePoint)
{
  uint16_t ch = 0 ;
  uint8_t loop = 0, index = 0;
  
  switch (*Char)
    {
		case '^':
			ch = 0x0181;
			break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':      
      ch = NumberMap[*Char - ASCII_CHAR_0];    
      break;
          
    default:
      /* The character Char is one letter in upper case*/
      if((*Char < ASCII_CHAR_LEFT_OPEN_BRACKET) && (*Char > ASCII_CHAR_AT_SYMBOL) )
      {
        ch = CapLetterMap[*Char - 'A'];
      }
      /* The character Char is one letter in lower case*/
      if ( (*Char < ASCII_CHAR_LEFT_OPEN_BRACE) && ( *Char > ASCII_CHAR_APOSTROPHE) )
      {
        ch = CapLetterMap[*Char - 'a'];
      }
      break;
  }
       
  /* Set the digital point can be displayed if the point is on */
  if (Point == POINT_ON)
  {
    ch |= 0x0002;
  }

  /* Set the "COL" segment in the character that can be displayed if the column is on */
  if (DoublePoint == DOUBLEPOINT_ON)
  {
    ch |= 0x0020;
  }    

  for (loop = 12,index=0 ;index < 4; loop -= 4,index++)
  {
    Digit[index] = (ch >> loop) & 0x0f; /*To isolate the less signifiant dibit */
  }
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

