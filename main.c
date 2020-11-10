#include <xc.h>
//#include "config.h"
#include "I2C_LCD.h"
void main(void) {
    TRISAbits.TRISA1=0;
    I2C_Master_Init();
    LCD_Init(0x4E); // Initialize LCD module with I2C address = 0x4E
    while(1)
    {
        PORTAbits.RA1=0;
        LCD_Set_Cursor(1, 1);
        LCD_Write_String(" Khaled Magdy");
        LCD_Set_Cursor(2, 1);
        LCD_Write_String(" DeepBlue");
    } 
    return;
}