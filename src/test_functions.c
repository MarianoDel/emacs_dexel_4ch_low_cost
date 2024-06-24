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
#include "i2c.h"

#include "screen.h"
#include "ssd1306_params.h"
#include "ssd1306_display.h"
#include "ssd1306_gfx.h"
// #include "dmx_receiver.h"
// #include "temperatures.h"


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
void TF_Led (void);
void TF_S1 (void);
void TF_S2 (void);

void TF_S1_S2_Fan(void);

void TF_Usart2_Tx (void);
void TF_Usart2_Tx_Rx (void);

void TF_MenuFunction (void);
void TF_Dmx_Break_Detect (void);
void TF_Dmx_Packet (void);
void TF_Dmx_Packet_Data (void);
void TF_Temp_Channel (void);

#ifndef I2C_WITH_INTS
void TF_I2C_Send_Data (void);
void TF_I2C_Send_Addr (void);
void TF_Oled_Screen (void);
#endif

void TF_Oled_Screen_Int (void);

// Module Functions ------------------------------------------------------------
void TF_Hardware_Tests (void)
{
    // TF_Led ();    //simple led functionality
    // TF_S1();
    // TF_S2();

    // TF_S1_S2_Fan();

    // TF_Usart2_Tx ();
    // TF_Usart2_Tx_Rx ();

    // TF_Dmx_Break_Detect ();
    // TF_Dmx_Packet ();    
    // TF_Dmx_Packet_Data ();
    // TF_Temp_Channel ();    

    // TF_CheckSET ();
    // TF_CheckCCW ();
    // TF_CheckCW ();

    TF_I2C_Send_Addr ();
    // TF_Oled_Screen ();
    
    // TF_Oled_Screen_Int ();
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


void TF_S1 (void)
{
    while (1)
    {
        if (S1)
            LED_ON;
        else
            LED_OFF;
    }    
}


void TF_S2 (void)
{
    while (1)
    {
        if (S2)
            LED_ON;
        else
            LED_OFF;
    }    
}


void TF_S1_S2_Fan(void)
{
    while (1)
    {
	if (S1 || S2)
	{
	    LED_ON;
	    CTRL_FAN_ON;
	}
	else
	{
	    LED_OFF;
	    CTRL_FAN_OFF;
	}
    }
}


void TF_Usart2_Tx (void)
{
    Wait_ms(100);
    Usart2Config();

    while (1)
    {
	Wait_ms(2000);
	Usart2Send("Mariano\r\n");
    }
}


void TF_Usart2_Tx_Rx (void)
{
    char buff [100];
    unsigned char len = 0;
    
    Wait_ms(100);
    Usart2Config();
    Wait_ms(100);
    Usart2Send("\r\nUsart2 Tx Rx loopback test!\r\n");    
    Wait_ms(100);
    
    while (1)
    {
	if (Usart2HaveData())
	{
	    Usart2HaveDataReset();
	    len = Usart2ReadBuffer((unsigned char *) buff, 100);
	    Usart2Send("\r\nnew buff: ");
	}

	if (len)
	{
	    Wait_ms(2000);
	    // len comes with '\0'
	    buff[len - 1] = '\r';
	    buff[len + 0] = '\n';
	    buff[len + 1] = '\0';	    
	    Usart2Send(buff);
	    len = 0;
	}
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
    // LCD_UtilsInit();
    // CTRL_BKL_ON;
    // LCD_ClearScreen();
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
                    // LCD_Writel1(s_lcd);
                    dmx_data1 = dmx_buff_data[1];
                }

                if (dmx_data2 != dmx_buff_data[2])
                {
                    sprintf(s_lcd, "ch2: %03d", dmx_buff_data[2]);
                    // LCD_Writel2(s_lcd);
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
    // LCD_UtilsInit();
    // CTRL_BKL_ON;
    // LCD_ClearScreen();
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
        // LCD_ClearScreen ();
        
        temp_degrees = Temp_TempToDegrees(Temp_Channel);
        
        sprintf(s_lcd, "Ch: %04d T: %d",
                Temp_Channel,
                temp_degrees);

        // LCD_Writel1(s_lcd);

        sprintf(s_lcd, "convert: %04d",
                Temp_DegreesToTemp(temp_degrees));

        // LCD_Writel2(s_lcd);
    }
}


// Test with I2C without ints
// disable irqs on i2c.h
#ifndef I2C_WITH_INTS
void TF_I2C_Send_Data (void)
{
    I2C1_Init();
    
    while (1)
    {
        LED_ON;
        I2C1_SendByte (I2C_ADDRESS_SLV, 0x55);
        Wait_ms (30);
        LED_OFF;
        Wait_ms(970);
    }
    
}


void TF_I2C_Send_Addr (void)
{
    I2C1_Init();
    
    while (1)
    {
        LED_ON;
        I2C1_SendAddr (OLED_ADDRESS);
        // I2C1_SendAddr (0x3D);        
        Wait_ms (30);
        LED_OFF;
        Wait_ms(970);
    }
}


void TF_Oled_Screen (void)
{
    // OLED Init
    Wait_ms(500);    //for supply stability
    I2C1_Init();
    Wait_ms(10);

    //primer pantalla
    LED_ON;
    SCREEN_Init();
    LED_OFF;
    
    while (1)
    {
        LED_ON;
        display_contrast (255);        
	SCREEN_Text2_Line1 ("Primera   ");    
	SCREEN_Text2_Line2 ("  Pantalla");

        LED_OFF;
        Wait_ms(5000);

        LED_ON;
        display_contrast (10);
	SCREEN_Text2_Line1 ("Segunda   ");    
	SCREEN_Text2_Line2 ("  Pantalla");
        LED_OFF;
        Wait_ms(5000);

        display_contrast (255);        
        LED_ON;
        display_invert(1);
	SCREEN_Text2_Line1 ("Tercera   ");    
	SCREEN_Text2_Line2 ("  Pantalla");
        LED_OFF;
        Wait_ms(5000);

        LED_ON;
        display_invert(0);
	SCREEN_Text2_Line1 ("Cuarta    ");    
	SCREEN_Text2_Line2 ("  Pantalla");
        LED_OFF;
        Wait_ms(5000);
    }
}
#endif

// Test with I2C with ints
// enable irqs on i2c.h
#ifdef I2C_WITH_INTS
extern volatile unsigned short timer_standby;
extern void display_update_int_state_machine (void);
void TF_Oled_Screen_Int (void)
{
    // OLED Init
    Wait_ms(500);    //for supply stability
    I2C1_Init();
    Wait_ms(10);

    //primer pantalla
    SCREEN_Init();

    unsigned char a = 0;
    while (1)
    {
        if (!timer_standby)
        {
            LED_ON;
            timer_standby = 1000;
            if (a)
            {
		SCREEN_Text2_Line1 ("Infinity  ");    
		SCREEN_Text2_Line2 ("  Clinics ");
                a = 0;
            }
            else
            {
		SCREEN_Text2_Line1 ("Second    ");    
		SCREEN_Text2_Line2 ("  Screen  ");
                a = 1;
            }
            LED_OFF;
        }
        display_update_int_state_machine();
    }
}
#endif


//--- end of file ---//
