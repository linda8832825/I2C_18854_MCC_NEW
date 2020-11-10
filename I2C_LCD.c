#include <xc.h>
#include "I2C_LCD.h"
unsigned char RS, i2c_add, BackLight_State = LCD_BACKLIGHT;
//---------------[ I2C Routines ]-------------------
//--------------------------------------------------
void I2C_Master_Init()
{
    SSP1CON1 = 0x28; //ssp控制暫存器 //0010 1000 
                     //bit7(WCOL：寫衝突檢測bit)=0 =發送時無衝突
                     //bit6(SSPOV：接收溢出指示符bit)=0=無溢出
                     //bit5(SSPEN：同步串列阜啟用bitSSPEN：同步串列阜啟用bit)=1= 啟用串列阜，並將SDA和SCL引腳配置為串列阜引腳的源
                     //bit4(CKP：clock極性選擇bit) master下不需要
                     //bit3~0(SSPM <3：0>：同步串列阜模式選擇bit)=1000=I2C master模式，clock= FOSC/（4 *（SSPxADD + 1））
    SSP1CON2 = 0x00; //ssp控制暫存器 //0000 0000 
                     //bit7(通用呼叫智能bit)=0 禁用通用呼叫地址
                     //bit6(acknowledge(確認) 狀態位) = ack 1 =未收到確認 0 =已收到確認
                     //bit5(acknowledge(確認)data bit)=在接收模式下：當用戶在接收結束時啟動確認序列時發送的值 1 =沒有確認 0 =確認
                     //bit4(ACKEN：應答序列智能bit)=0 =確認序列空閒
                     //bit3(RCEN：接收智能bit)=0 =先不接收
                     //bit2(PEN：停止條件智能bit)=0 =先不停止
                     //bit1(RSEN：重複啟動條件智能bit)=0=先不重複啟動條件
                     //bit0(SEN：啟動條件智能bit)=0 =先不起始
    
    SSP1STAT = 0x00; //狀態暫存器 //0000 0000 
                     //bit7(SMP:)=0 =在數據輸出時間的中間對輸入數據進行採樣
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
  SSP1CON2bits.SEN = 1; //在master模式下：1 =在SDA和SCL引腳上啟動啟動條件
}
void I2C_Master_RepeatedStart()
{
  I2C_Master_Wait();
  SSP1CON2bits.RSEN = 1; //1 =在SDA和SCL引腳上啟動重複啟動條件。
}
void I2C_Master_Stop()
{
  I2C_Master_Wait();
  SSP1CON2bits.PEN = 1; //SCKMSSP釋放控制：1 =在SDA和SCL引腳上啟動停止條件
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
  SSP1BUF = data;  //不知道是1還2
  while(!SSP1IF); // Wait Until Completion  //不知道是1還2
  SSP1IF = 0;  //不知道是1還2
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