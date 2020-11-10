#include <xc.h>
#include "I2C_LCD.h"
unsigned char RS, i2c_add, BackLight_State = LCD_BACKLIGHT;
//---------------[ I2C Routines ]-------------------
//--------------------------------------------------
void I2C_Master_Init()
{
    SSP1CON1 = 0x28; //ssp����Ȧs�� //0010 1000 
                     //bit7(WCOL�G�g�Ĭ��˴�bit)=0 =�o�e�ɵL�Ĭ�
                     //bit6(SSPOV�G�������X���ܲ�bit)=0=�L���X
                     //bit5(SSPEN�G�P�B��C���ҥ�bitSSPEN�G�P�B��C���ҥ�bit)=1= �ҥΦ�C���A�ñNSDA�MSCL�޸}�t�m����C���޸}����
                     //bit4(CKP�Gclock���ʿ��bit) master�U���ݭn
                     //bit3~0(SSPM <3�G0>�G�P�B��C���Ҧ����bit)=1000=I2C master�Ҧ��Aclock= FOSC/�]4 *�]SSPxADD + 1�^�^
    SSP1CON2 = 0x00; //ssp����Ȧs�� //0000 0000 
                     //bit7(�q�ΩI�s����bit)=0 �T�γq�ΩI�s�a�}
                     //bit6(acknowledge(�T�{) ���A��) = ack 1 =������T�{ 0 =�w����T�{
                     //bit5(acknowledge(�T�{)data bit)=�b�����Ҧ��U�G��Τ�b���������ɱҰʽT�{�ǦC�ɵo�e���� 1 =�S���T�{ 0 =�T�{
                     //bit4(ACKEN�G�����ǦC����bit)=0 =�T�{�ǦC�Ŷ�
                     //bit3(RCEN�G��������bit)=0 =��������
                     //bit2(PEN�G������󴼯�bit)=0 =��������
                     //bit1(RSEN�G���ƱҰʱ��󴼯�bit)=0=�������ƱҰʱ���
                     //bit0(SEN�G�Ұʱ��󴼯�bit)=0 =�����_�l
    
    SSP1STAT = 0x00; //���A�Ȧs�� //0000 0000 
                     //bit7(SMP:)=0 =�b�ƾڿ�X�ɶ����������J�ƾڶi��ļ�
    SSP1ADD = ((_XTAL_FREQ/4)/I2C_BaudRate) - 1;
    SCL_D = 1;
    SDA_D = 1;
}
void I2C_Master_Wait()
{
  while ((SSP1STAT & 0x04) || (SSP1CON2 & 0x1F));
}
void I2C_Master_Start()
{
  I2C_Master_Wait();
  SSP1CON2bits.SEN = 1; //�bmaster�Ҧ��U�G1 =�bSDA�MSCL�޸}�W�ҰʱҰʱ���
}
void I2C_Master_RepeatedStart()
{
  I2C_Master_Wait();
  SSP1CON2bits.RSEN = 1; //1 =�bSDA�MSCL�޸}�W�Ұʭ��ƱҰʱ���C
}
void I2C_Master_Stop()
{
  I2C_Master_Wait();
  SSP1CON2bits.PEN = 1; //SCKMSSP���񱱨�G1 =�bSDA�MSCL�޸}�W�Ұʰ������
}
void I2C_ACK(void)
{
  SSP1CON2bits.ACKDT = 0; // 0 -> ACK
  I2C_Master_Wait();
  SSP1CON2bits.ACKEN = 1; // Send ACK
}
void I2C_NACK(void)
{
  SSP1CON2bits.ACKDT = 1; // 1 -> NACK
  I2C_Master_Wait();
  SSP1CON2bits.ACKEN = 1; // Send NACK
}
unsigned char I2C_Master_Write(unsigned char data)
{
  I2C_Master_Wait();
  SSP1BUF = data;  //�����D�O1��2
  while(!SSP1IF); // Wait Until Completion  //�����D�O1��2
  SSP1IF = 0;  //�����D�O1��2
  return SSP1CON2bits.ACKSTAT;
}
unsigned char I2C_Read_Byte(void)
{
  //---[ Receive & Return A Byte ]---
  I2C_Master_Wait();
  SSP1CON2bits.RCEN = 1; // Enable & Start Reception
  while(!SSP1IF); // Wait Until Completion
  SSP1IF = 0; // Clear The Interrupt Flag Bit
  I2C_Master_Wait();
  return SSP1BUF; // Return The Received Byte
}
//======================================================
//---------------[ LCD Routines ]----------------
//-----------------------------------------------
void LCD_Init(unsigned char I2C_Add)
{
  i2c_add = I2C_Add;
  IO_Expander_Write(0x00);
  __delay_ms(30);
  LCD_CMD(0x03);
  __delay_ms(5);
  LCD_CMD(0x03);
  __delay_ms(5);
  LCD_CMD(0x03);
  __delay_ms(5);
  LCD_CMD(LCD_RETURN_HOME);
  __delay_ms(5);
  LCD_CMD(0x20 | (LCD_TYPE << 2));
  __delay_ms(50);
  LCD_CMD(LCD_TURN_ON);
  __delay_ms(50);
  LCD_CMD(LCD_CLEAR);
  __delay_ms(50);
  LCD_CMD(LCD_ENTRY_MODE_SET | LCD_RETURN_HOME);
  __delay_ms(50);
}
void IO_Expander_Write(unsigned char Data)
{
  I2C_Master_Start();
  I2C_Master_Write(i2c_add);
  I2C_Master_Write(Data | BackLight_State);
  I2C_Master_Stop();
}
void LCD_Write_4Bit(unsigned char Nibble)
{
  // Get The RS Value To LSB OF Data
  Nibble |= RS;// Nibble = Nibble | RS
  IO_Expander_Write(Nibble | 0x04);
  IO_Expander_Write(Nibble & 0xFB);
  __delay_us(50);
}
void LCD_CMD(unsigned char CMD)
{
  RS = 0; // Command Register Select
  LCD_Write_4Bit(CMD & 0xF0);
  LCD_Write_4Bit((CMD << 4) & 0xF0);
}
void LCD_Write_Char(char Data)
{
  RS = 1; // Data Register Select
  LCD_Write_4Bit(Data & 0xF0);
  LCD_Write_4Bit((Data << 4) & 0xF0);
}
void LCD_Write_String(char* Str)
{
  for(int i=0; Str[i]!='\0'; i++)
    LCD_Write_Char(Str[i]);
}
void LCD_Set_Cursor(unsigned char ROW, unsigned char COL)
{
  switch(ROW) 
  {
    case 2:
      LCD_CMD(0xC0 + COL-1);
      break;
    case 3:
      LCD_CMD(0x94 + COL-1);
      break;
    case 4:
      LCD_CMD(0xD4 + COL-1);
      break;
    // Case 1
    default:
      LCD_CMD(0x80 + COL-1);
  }
}
void Backlight()
{
  BackLight_State = LCD_BACKLIGHT;
  IO_Expander_Write(0);
}
void noBacklight()
{
  BackLight_State = LCD_NOBACKLIGHT;
  IO_Expander_Write(0);
}
void LCD_SL()
{
  LCD_CMD(0x18);
  __delay_us(40);
}
void LCD_SR()
{
  LCD_CMD(0x1C);
  __delay_us(40);
}
void LCD_Clear()
{
  LCD_CMD(0x01);
  __delay_us(40);
}