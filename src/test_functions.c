//------------------------------------------------
// ## Internal Test Functions Module
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### TEST_FUNCTIONS.C #########################
//------------------------------------------------

// Includes --------------------------------------------------------------------
#include "test_functions.h"
#include "stm32g0xx.h"
#include "hard.h"
#include "tim.h"
#include "usart.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "flash_program.h"

#include "lcd_utils.h"
#include "dmx_receiver.h"
#include "temperatures.h"


#include <stdio.h>
#include <string.h>



// Externals -------------------------------------------------------------------

// - for DMX receiver
extern volatile unsigned char dmx_buff_data[];
extern volatile unsigned char Packet_Detected_Flag;
extern volatile unsigned short DMX_channel_selected;
extern volatile unsigned char DMX_channel_quantity;
extern volatile unsigned char dmx_receive_flag;

// - for ADC
extern volatile unsigned short adc_ch [];

// Globals ---------------------------------------------------------------------


// Module Private Types & Macros -----------------------------------------------


// Module Private Functions ----------------------------------------------------
void TF_Led_Lock (void);
void TF_Led (void);
void TF_lcdE (void);
void TF_lcdRS (void);
void TF_lcdBklight (void);
void TF_lcdData (void);
void TF_lcdBlink (void);
void TF_lcdScroll (void);
void TF_MenuFunction (void);
void TF_Dmx_Break_Detect (void);
void TF_Dmx_Packet (void);
void TF_Dmx_Packet_Data (void);
void TF_Temp_Channel (void);

#ifdef HARDWARE_VERSION_2_0
void TF_SW_UP (void);
void TF_SW_DWN (void);
void TF_SW_SEL (void);
#endif

#ifdef HARDWARE_VERSION_2_1
void TF_CheckCCW (void);
void TF_CheckCW (void);
void TF_CheckSET (void);
#endif


// Module Functions ------------------------------------------------------------
void TF_Hardware_Tests (void)
{
    // TF_Led ();    //simple led functionality
    // TF_SW_UP();
    // TF_SW_DWN();
    // TF_SW_SEL();    
    // TF_lcdE();
    // TF_lcdRS();
    // TF_lcdBklight();    
    // TF_lcdData();
    // TF_lcdBlink();
    // TF_lcdScroll();
    // TF_Dmx_Break_Detect ();
    // TF_Dmx_Packet ();    
    // TF_Dmx_Packet_Data ();
    // TF_Temp_Channel ();    

    TF_CheckSET ();
    // TF_CheckCCW ();
    // TF_CheckCW ();
    
}


void TF_Led (void)
{
    while (1)
    {
        if (LED)
            LED_OFF;
        else
            LED_ON;

        Wait_ms(300);
    }
}


void TF_lcdE (void)
{
    while (1)
    {
        if (LCD_E)
            LCD_E_OFF;
        else
            LCD_E_ON;

        Wait_ms(10);
    }
}


void TF_lcdRS (void)
{
    while (1)
    {
        if (LCD_RS)
            LCD_RS_OFF;
        else
            LCD_RS_ON;

        Wait_ms(10);
    }
}


void TF_lcdBklight (void)
{
    while (1)
    {
        if (CTRL_BKL)
            CTRL_BKL_OFF;
        else
            CTRL_BKL_ON;

        Wait_ms(1000);
    }
}


void TF_lcdData (void)
{
    while (1)
    {
        //pa4 a pa7        
        GPIOA->BSRR = 0x000000F0;
        for (int i = 0; i < 5; i++)
        {
            LED_ON;
            Wait_ms(100);
            LED_OFF;
            Wait_ms(1400);
        }        
        GPIOA->BSRR = 0x00F00000;

        Wait_ms(2000);

        GPIOA->BSRR = 0x00000050;
        for (int i = 0; i < 5; i++)
        {
            LED_ON;
            Wait_ms(100);
            LED_OFF;
            Wait_ms(100);            
            LED_ON;
            Wait_ms(100);
            LED_OFF;
            Wait_ms(1200);
        }        
        GPIOA->BSRR = 0x00500000;
        
        Wait_ms(2000);

        GPIOA->BSRR = 0x000000A0;
        for (int i = 0; i < 5; i++)
        {
            LED_ON;
            Wait_ms(100);
            LED_OFF;
            Wait_ms(100);            
            LED_ON;
            Wait_ms(100);
            LED_OFF;
            Wait_ms(100);            
            LED_ON;
            Wait_ms(100);
            LED_OFF;
            Wait_ms(1000);
        }
        GPIOA->BSRR = 0x00A00000;
        
        Wait_ms(2000);
    }
}


void TF_lcdBlink (void)
{
    // needs a timeout call to LCD_UpdateTimer()
    LCD_UtilsInit();
    CTRL_BKL_ON;

    while (1)
    {
        while (LCD_ShowBlink("Kirno Technology",
                             "  Smart Driver  ",
                             2,
                             BLINK_DIRECT) != resp_finish);

        LCD_ClearScreen();
        Wait_ms(1000);
    }
}


void TF_lcdScroll (void)
{
   // needs a timeout call to LCD_UpdateTimer()
    resp_t resp = resp_continue;

    LCD_UtilsInit();
    CTRL_BKL_ON;
    
    while (1)
    {
        // LCD_ClearScreen();
        // Wait_ms(2000);
        do {
            resp = LCD_Scroll1 ("Dexel Lighting DMX 2 channels 8 amps controller.");
        } while (resp != resp_finish);

        Wait_ms(2000);
    }
}


void TF_Dmx_Break_Detect (void)
{
    // needs a timeout call to DMX_Int_Millis_Handler ()
    TIM_14_Init();
    DMX_channel_selected = 1;
    DMX_channel_quantity = 2;
    DMX_EnableRx();

    while (1)
    {
        if (dmx_receive_flag)
        {
            dmx_receive_flag = 0;
            LED_ON;
            Wait_ms(10);
            LED_OFF;
        }
    }
}


void TF_Dmx_Packet (void)
{
    // needs a timeout call to DMX_Int_Millis_Handler ()
    Usart1Config();
    TIM_14_Init();
    DMX_channel_selected = 1;
    DMX_channel_quantity = 2;
    DMX_EnableRx();

    while (1)
    {
        if (Packet_Detected_Flag)
        {
            Packet_Detected_Flag = 0;
            LED_ON;
            Wait_ms(2);
            LED_OFF;
        }
    }
}


void TF_Dmx_Packet_Data (void)
{
    // Init LCD
    LCD_UtilsInit();
    CTRL_BKL_ON;
    LCD_ClearScreen();
    Wait_ms(1000);

    // Init DMX
    Usart1Config();
    TIM_14_Init();
    DMX_channel_selected = 1;
    DMX_channel_quantity = 2;
    DMX_EnableRx();

    unsigned char dmx_data1 = 0;
    unsigned char dmx_data2 = 0;    

    while (1)
    {
        if (Packet_Detected_Flag)
        {
            Packet_Detected_Flag = 0;
            LED_ON;

            if (dmx_buff_data[0] == 0)
            {
                char s_lcd [20] = { 0 };

                if (dmx_data1 != dmx_buff_data[1])
                {
                    sprintf(s_lcd, "ch1: %03d", dmx_buff_data[1]);
                    LCD_Writel1(s_lcd);
                    dmx_data1 = dmx_buff_data[1];
                }

                if (dmx_data2 != dmx_buff_data[2])
                {
                    sprintf(s_lcd, "ch2: %03d", dmx_buff_data[2]);
                    LCD_Writel2(s_lcd);
                    dmx_data2 = dmx_buff_data[2];
                }
            }
            
            LED_OFF;
        }
    }
}


void TF_Temp_Channel (void)
{
    // Init LCD
    LCD_UtilsInit();
    CTRL_BKL_ON;
    LCD_ClearScreen();
    Wait_ms(1000);

    // Init ADC and DMA
    AdcConfig();
    DMAConfig();
    DMA1_Channel1->CCR |= DMA_CCR_EN;
    ADC1->CR |= ADC_CR_ADSTART;
    
    char s_lcd [30] = { 0 };
    unsigned char temp_degrees = 0;
    while (1)
    {
        Wait_ms (500);
        LCD_ClearScreen ();
        
        temp_degrees = Temp_TempToDegrees(Temp_Channel);
        
        sprintf(s_lcd, "Ch: %04d T: %d",
                Temp_Channel,
                temp_degrees);

        LCD_Writel1(s_lcd);

        sprintf(s_lcd, "convert: %04d",
                Temp_DegreesToTemp(temp_degrees));

        LCD_Writel2(s_lcd);
    }
}


#ifdef HARDWARE_VERSION_2_0
void TF_SW_UP (void)
{
    while (1)
    {
        if (SW_UP)
            LED_ON;
        else
            LED_OFF;
    }    
}


void TF_SW_SEL (void)
{
    while (1)
    {
        if (SW_SEL)
            LED_ON;
        else
            LED_OFF;
    }    
}


void TF_SW_DWN (void)
{
    while (1)
    {
        if (SW_DWN)
            LED_ON;
        else
            LED_OFF;
    }    
}
#endif

#ifdef HARDWARE_VERSION_2_1
void TF_CheckSET (void)
{
    while (1)
    {
        if (CheckSET() > SW_NO)
            LED_ON;
        else
            LED_OFF;

        UpdateSwitches();
    }    
}
#endif
//--- end of file ---//
