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



// Module Functions ------------------------------------------------------------
void TF_Hardware_Tests (void)
{
    // TF_Led ();    //simple led functionality
    // TF_S1();
    // TF_S2();

    // TF_S1_S2_Fan();

    // TF_Usart2_Tx ();
    TF_Usart2_Tx_Rx ();

    // TF_Dmx_Break_Detect ();
    // TF_Dmx_Packet ();    
    // TF_Dmx_Packet_Data ();
    // TF_Temp_Channel ();    

    // TF_CheckSET ();
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
	    len = Usart2ReadBuffer(buff, 100);
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


//--- end of file ---//
