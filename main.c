#include <xc.h>
#include "I2C_LCD.h"
void main(void) {
    TRISAbits.TRISA1=0;//�]�wra1�O��X�}
    PORTAbits.RA1=0;//��ra1��X�C�줸
    I2C_Master_Init();//��l��i2c
    LCD_Init(0x4E); //��l��lcd�A�öǰeslave����m
    LCD_Set_Cursor(1, 1);//�b�Ĥ@�檺�Ĥ@�Ӧr���g
    LCD_Write_String(" Khaled Magdy");//�n�g���r��
    LCD_Set_Cursor(2, 1);
    LCD_Write_String(" DeepBlue");
    while(1) {
      
    } 
    return;
}

