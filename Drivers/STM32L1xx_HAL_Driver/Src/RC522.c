#include "RC522.h"

#define MAXRLEN 18

void init_SPI_RC522()
{
  LL_AHB1_GRP1_EnableClock(RCC_RC522);
	
  LL_GPIO_InitTypeDef RC522_SPI_OUTPUT;
  RC522_SPI_OUTPUT.Pin = RC522_MOSI|RC522_SCK|RC522_SS|RST_RC522|SUPPLY_RC522;
  RC522_SPI_OUTPUT.Mode = LL_GPIO_MODE_OUTPUT;
  RC522_SPI_OUTPUT.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  RC522_SPI_OUTPUT.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  
  LL_GPIO_InitTypeDef RC522_SPI_INPUT;
  RC522_SPI_INPUT.Pin = RC522_MISO;
  RC522_SPI_INPUT.Mode = LL_GPIO_MODE_INPUT;
  RC522_SPI_INPUT.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  RC522_SPI_INPUT.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  RC522_SPI_INPUT.Pull = LL_GPIO_PULL_UP;
  
  LL_GPIO_Init(RC522_GPIO, &RC522_SPI_INPUT);
  LL_GPIO_Init(RC522_GPIO, &RC522_SPI_OUTPUT);
  
  RC522_PIN_set(SUPPLY_RC522);
  RC522_PIN_set(RC522_SS);
  RC522_PIN_clr(RST_RC522);
  RC522_PIN_clr(RC522_SCK);
}

uint8_t trans_SPI_RC522(uint8_t DataIN)
{
  uint8_t DataOUT=0;
  for (uint8_t i=8; i>0; i--)
  {
    //volatile uint8_t DelaySPI=5;
    //while(DelaySPI--);
    if (DataIN&0x80) 
      RC522_PIN_set(RC522_MOSI);
    else
      RC522_PIN_clr(RC522_MOSI);
    DataIN<<=1;
    RC522_PIN_set(RC522_SCK);
    DataOUT<<=1;
    if (RC522_PIN_MISO) 
      DataOUT|=1; 
    RC522_PIN_clr(RC522_SCK);
  }
  return DataOUT;
}

void Write_Reg_RC522(uint8_t Address, uint8_t Data)
{
  RC522_PIN_clr (RC522_SCK);
  RC522_PIN_clr (RC522_SS);
  Address=(Address<<1)&(0x7E);
  trans_SPI_RC522(Address);
  trans_SPI_RC522(Data);
  RC522_PIN_set (RC522_SS);
}

uint8_t Read_Reg_RC522 (uint8_t Address)
{
  uint8_t Data;
  RC522_PIN_clr(RC522_SCK);
  RC522_PIN_clr(RC522_SS);
  Address=(Address<<1)|(1<<7);
  trans_SPI_RC522(Address);
  Data=trans_SPI_RC522(0);
  RC522_PIN_set(RC522_SS);
  return Data;
}

void set_bit_mask (uint8_t RegisterAddress, uint8_t mask)
{
  uint8_t RegisterData;
  RegisterData=Read_Reg_RC522(RegisterAddress);
  Write_Reg_RC522(RegisterAddress, RegisterData|mask);
}

void clear_bit_mask (uint8_t RegisterAddress, uint8_t mask)
{
  uint8_t RegisterData;
  RegisterData=Read_Reg_RC522(RegisterAddress);
  Write_Reg_RC522(RegisterAddress, RegisterData&~mask); 
}


void init_RC522()
{
  init_SPI_RC522();
  RC522_PIN_clr(RST_RC522); 
  RC522_PIN_set(RST_RC522); 
  Write_Reg_RC522(CommandReg, 0x0F);
  Write_Reg_RC522(TxAutoReg, 0x40); 
  Write_Reg_RC522(ModeReg, 0x3D); 
  set_bit_mask(TxControlReg, 0x03);
}

void calculate_CRC(uint8_t* Input_Data, 
                   uint8_t Length_Input, 
                   uint8_t* Out_Data) 
{
  uint8_t i;
  uint8_t temp;
  clear_bit_mask(DivIrqReg, (1<<2)); 
  Write_Reg_RC522(CommandReg, PCD_IDLE);
  set_bit_mask(FIFOLevelReg, 0x80);
  for (i=0; i<Length_Input; i++)
  {
    Write_Reg_RC522(FIFODataReg, Input_Data[i]);
  }
  Write_Reg_RC522(CommandReg, PCD_CALCCRC);
  i=255;
  do
  {
    temp=Read_Reg_RC522(DivIrqReg);
    i--;
  }
  while ((i!=0) &&!(temp&0x04)); 
  Out_Data[0]=Read_Reg_RC522(CRCResultRegL); 
  Out_Data[1]=Read_Reg_RC522(CRCResultRegM); 
}

uint8_t RC522_comm_light (uint8_t* Input_Data, uint8_t Length_Byte_Input,
                          uint8_t* Out_Data, uint8_t* Length_Bit_Out)
{
  uint8_t status=ERR;
  uint8_t lastBits=0;
  uint8_t temp;
  uint16_t i;
  
  clear_bit_mask(ComIrqReg, (1<<7));
  Write_Reg_RC522(CommandReg, PCD_IDLE);
  clear_bit_mask(FIFOLevelReg, (1<<7));
  for (i=0; i<Length_Byte_Input; i++)
  {
    Write_Reg_RC522(FIFODataReg, Input_Data[i]);
  }
   Write_Reg_RC522(CommandReg, PCD_TRANSCEIVE);
  set_bit_mask(BitFramingReg, (1<<7));
  i=1000;
  do
  {
    temp=Read_Reg_RC522(ComIrqReg);
    i--;
  }
  while ((i!=0)&&!(temp&((1<<5)|(1<<4))));
  clear_bit_mask(BitFramingReg, (1<<7));
  if(i!=0)
  {
    if (!(Read_Reg_RC522(ErrorReg)&0x1B))
    {
      status=OK;
      temp=Read_Reg_RC522(FIFOLevelReg);
      lastBits=Read_Reg_RC522(ControlReg)&0x07;
      if (lastBits) *Length_Bit_Out=(temp-1)*8+lastBits; 
      else *Length_Bit_Out=temp*8;
      if (temp==0) temp=1;
      if (temp>MAXRLEN) temp=MAXRLEN;
      for (i=0; i<temp; i++)
      {
        Out_Data[i]=Read_Reg_RC522(FIFODataReg);
      }
    }
      else 
      {
        status=ERR;
      }
    }
    Write_Reg_RC522(CommandReg, PCD_IDLE);
    return status;
}

uint8_t halt()
{
  uint8_t status;
  uint8_t Buffer[4];
  uint8_t LengthBit;
  Buffer[0]=PICC_HALT; 
  Buffer[1]=0x00;
  calculate_CRC(Buffer, 2, &Buffer[2]); 
  status=RC522_comm_light(Buffer, 4, Buffer, &LengthBit);
  return status;
}


uint8_t request_card(uint8_t ReqCode, uint8_t *TypeCard)
{
  uint8_t status;
  uint8_t LengthBit;
  Write_Reg_RC522(BitFramingReg, 0x07); 
  status=RC522_comm_light(&ReqCode, 1, TypeCard, &LengthBit);
  if ((status==OK)&&(LengthBit==2*8))
    status=OK;
  else
    status=ERR;
  return status;
}


uint8_t read_UID(uint8_t Anticoll_CMD,
                 uint8_t Anticoll_ARG, 
                 uint8_t *Answer) 
{
  uint8_t i;
  uint8_t xor=0;
  uint8_t status;
  uint8_t LengthBit;
  uint8_t Buffer[MAXRLEN]; 
  Write_Reg_RC522(BitFramingReg, 0x00);
  Buffer[0]=Anticoll_CMD; 
  Buffer[1]=Anticoll_ARG; 
  status=RC522_comm_light(Buffer, 2, Buffer, &LengthBit);
  if (status==OK)
  {
    for(i=0; i<5; i++) 
    {
      Answer[i]=Buffer[i];
      xor^=Buffer[i];
    }
    if (xor) status=ERR; 
  }
  return status;
}

uint8_t select_card(uint8_t Anticoll_CMD, uint8_t Anticoll_ARG,
                    uint8_t *SerialNum) 
{
  uint8_t i;
  uint8_t status;
  uint8_t LengthBit;
  uint8_t BufferRC522[MAXRLEN];
  BufferRC522[0]=Anticoll_CMD; 
  BufferRC522[1]=Anticoll_ARG; 
  BufferRC522[6]=0;
  for (i=0; i<4; i++) 
  {
    BufferRC522[i+2]=SerialNum[i];
    BufferRC522[6]^=SerialNum[i]; 
  }
  calculate_CRC(BufferRC522, 7, &BufferRC522[7]); 
  status=RC522_comm_light(BufferRC522, 9, &SerialNum[0], &LengthBit);
  if ((status==OK)&&(LengthBit==3*8)) 
    //(1 SAK + 2 CRC)
    status=OK;
  else
    status=ERR;
  return status;
  }
 
 
uint8_t write_page (uint8_t AddrPage, uint8_t* array)
{
   uint8_t i;
   uint8_t status;
   uint8_t LengthBit;
   uint8_t BufferRC522[MAXRLEN];
  
   BufferRC522[0]=PICC_WRITE_4BYTE; 
   BufferRC522[1]=AddrPage; 
   for (i=0; i<4; i++)
   {
      BufferRC522[i+2]=array[i];
   }
   calculate_CRC (BufferRC522, 6, &BufferRC522[6]);
   status=RC522_comm_light (BufferRC522, 8, BufferRC522, &LengthBit);
   if ((status!=OK)||(LengthBit!=4)||((BufferRC522[0]&0x0F)!=ACK))
   {
     status=ERR;
   }
   return status;
}

uint8_t read_page (uint8_t AddrPage,
                   uint8_t *Data)
{
  uint8_t i;
  uint8_t status;
  uint8_t LengthBit;
  uint8_t BufferRC522[MAXRLEN];
  BufferRC522[0]=PICC_READ_4BYTE; 
  BufferRC522[1]=AddrPage; 
  calculate_CRC (BufferRC522, 2, &BufferRC522[2]); 
  status=RC522_comm_light(BufferRC522, 4, BufferRC522, &LengthBit);
  if ((status==OK)&&(LengthBit==18*8)) 
  {
    for (i=0; i<16; i++)
    {
      Data[i]=BufferRC522[i]; 
    }
  }
  else
    status=ERR;
  return status;
}
