#include <xc.h>
#include "I2C_LCD.h"
void main(void) {
    TRISAbits.TRISA1=0;//設定ra1是輸出腳
    I2C_Master_Init();//初始化i2c
    LCD_Init(0x4E); //初始化lcd，並傳送slave的位置
    while(1) {
        PORTAbits.RA1=0;//讓ra1輸出低位元
        LCD_Set_Cursor(1, 1);//在第一行的第一個字元寫
        LCD_Write_String(" Khaled Magdy");//要寫的字串
        LCD_Set_Cursor(2, 1);
        LCD_Write_String(" DeepBlue");
    } 
    return;
}