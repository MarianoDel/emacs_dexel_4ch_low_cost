//-------------------------------------------------------
// #### PROJECT: 4CH DMX RGB Low Cost - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### MAIN.C ##########################################
//-------------------------------------------------------

// Includes --------------------------------------------------------------------
#include "stm32g0xx.h"

#include "gpio.h"
#include "tim.h"
#include "usart.h"
#include "hard.h"
// #include "comm.h"
#include "parameters.h"

#include "flash_program.h"
#include "core_cm0plus.h"

#include "adc.h"
#include "dma.h"
#include "dsp.h"    // for temp sense

#include "test_functions.h"
#include "dmx_receiver.h"
#include "temperatures.h"

#include "i2c.h"
#include "screen.h"
#include "ssd1306_display.h"

// #include "dmx_mode.h"
// #include "manual_mode.h"
// #include "menues.h"
// #include "comms_power.h"

#include "manager.h"
#include "filters_and_offsets.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>



// Module Types Constants and Macros -------------------------------------------
typedef enum {
    MAIN_INIT,
    MAIN_HARD_INIT,
    MAIN_CHECK_CONF,
    MAIN_DMX_MODE_INIT,
    MAIN_MANUAL_MODE_INIT,
    MAIN_IN_DMX_MODE,
    MAIN_IN_MANUAL_MODE,
    MAIN_ENTERING_MAIN_MENU,
    MAIN_ENTERING_MAIN_MENU2,    
    MAIN_IN_MAIN_MENU
    
} main_state_e;

// Externals -------------------------------------------------------------------
// - extern from lcd
extern char s_blank [];

// - Externals from ADC Converter -------
volatile unsigned short adc_ch [ADC_CHANNEL_QUANTITY];

// - Externals for temp prot
ma16_u16_data_obj_t temp_filter;


// - Externals de la Memoria y los modos -------
parameters_typedef * pflash_mem = (parameters_typedef *) (unsigned int *) FLASH_ADDRESS_FOR_BKP;    //en flash
parameters_typedef mem_conf;

// - Externals for DMX receiver
volatile unsigned char dmx_buff_data[SIZEOF_DMX_BUFFER_DATA];
volatile unsigned char Packet_Detected_Flag = 0;
volatile unsigned short DMX_channel_selected = 0;
volatile unsigned char DMX_channel_quantity = 0;
volatile unsigned char dmx_receive_flag = 0;

// - Externals shared by modes
unsigned char mode_state;
volatile unsigned short mode_effect_timer;

// Globals ---------------------------------------------------------------------
// - Globals from timers -------
volatile unsigned short timer_standby = 0;

// -- for the timeouts in the modes ----
void (* ptFTT ) (void) = NULL;


// Module Private Functions ----------------------------------------------------
void TimingDelay_Decrement(void);
void SysTickError (void);
void EXTI4_15_IRQHandler(void);


//-------------------------------------------//
// @brief  Main program.
// @param  None
// @retval None
//------------------------------------------//
int main(void)
{
    //GPIO Configuration.
    GPIO_Config();

    //Start the SysTick Timer
#ifdef CLOCK_FREQ_64_MHZ
    if (SysTick_Config(64000))
        SysTickError();
#endif
#ifdef CLOCK_FREQ_16_MHZ
    if (SysTick_Config(16000))
        SysTickError();
#endif

    //--- Test Hardware Functions ---
    // TF_Hardware_Tests ();    
    //--- end of Test Hardware Functions ---    

    //--- Hardware Inits ---
    // OLED and Screen module Init
    Wait_ms(1000);
    I2C1_Init();
    Wait_ms(10);
    SCREEN_Init();
    Wait_ms(10);

    // Init DMX
    Usart1Config();
    TIM_14_Init();
    DMX_channel_selected = 1;
    DMX_channel_quantity = 4;
    DMX_EnableRx();

    // ADC & DMA for temp sense Init
    AdcConfig();
    DMAConfig();
    DMA1_Channel1->CCR |= DMA_CCR_EN;
    ADC1->CR |= ADC_CR_ADSTART;

    // Start filters
    FiltersAndOffsets_Filters_Reset ();
    
    // Start Tim for PWm
    TIM_3_Init ();

    // Init Usart2 for debug or colors fixed setup
    Usart2Config();
    
    // --- Welcome Code ------------
    SCREEN_Clear ();
    SCREEN_Text2_Line1 ("Kirno Tech");    
    SCREEN_Text2_Line2 ("Smart Drvr");
    timer_standby = 800;
    while (timer_standby)
        display_update_int_state_machine();

    SCREEN_Clear ();
    SCREEN_Text2_Line1 ("Dexel     ");    
    SCREEN_Text2_Line2 ("  Lighting");
    timer_standby = 1300;
    while (timer_standby)
        display_update_int_state_machine();
    

    // --- Get saved config or create one for default ---
    if (pflash_mem->program_type != 0xff)
    {
        //memory with valid data
        memcpy(&mem_conf, pflash_mem, sizeof(parameters_typedef));
#ifdef USART2_DEBUG_MODE
        Usart2Send("memory is saved with valid data\r\n");
#endif
        
    }
    else
    {
        // Default mem config
        mem_conf.dmx_first_channel = 1;
#ifdef ONE_CHANNEL_CONF_INIT
        mem_conf.dmx_channel_quantity = 1;
        mem_conf.max_current_channels[0] = 235;
        mem_conf.max_current_channels[1] = 235;
        mem_conf.max_current_channels[2] = 235;
        mem_conf.max_current_channels[3] = 235;	
#else
        mem_conf.dmx_channel_quantity = 4;	
        mem_conf.max_current_channels[0] = 255;
        mem_conf.max_current_channels[1] = 255;
        mem_conf.max_current_channels[2] = 255;
        mem_conf.max_current_channels[3] = 255;
#endif
        mem_conf.program_type = AUTODETECT_MODE;    //force mem save        
        mem_conf.temp_prot_deg = 70;    //70 degrees
        mem_conf.temp_prot = TEMP_IN_70;    //70 degrees
        mem_conf.manual_inner_mode = 0;    //manual fixed
#ifdef USART2_DEBUG_MODE
        Usart2Send("memory empty, set defaults\r\n");
#endif
        
    }

    //-- check NTC or LM335 connection on init --
#ifdef TEMP_SENSOR_NTC1K
    unsigned short temp_filtered = 0;
    MA16_U16Circular_Reset(&temp_filter);
    for (int i = 0; i < 16; i++)
    {
        temp_filtered = MA16_U16Circular(&temp_filter, Temp_Channel);
        Wait_ms(30);
    }

    if (temp_filtered < NTC_SHORTED)
    {
        CTRL_FAN_ON;
        Manager_Probe_Temp_Reset();
    }
    else
        Manager_Probe_Temp_Set();
#endif
#ifdef TEMP_SENSOR_LM335
    unsigned short temp_filtered = 0;
    MA16_U16Circular_Reset(&temp_filter);
    for (int i = 0; i < 16; i++)
    {
        temp_filtered = MA16_U16Circular(&temp_filter, Temp_Channel);
        Wait_ms(30);
    }

    if (temp_filtered < LM335_SHORTED)
    {
        CTRL_FAN_ON;
        Manager_Probe_Temp_Reset();
    }
    else
        Manager_Probe_Temp_Set();
#endif
    // -- end of check NTC or LM335 connection on init --
    
    // main program
    while (1)
    {
        Manager(&mem_conf);
    }

    return 0;
}

//--- End of Main ---//


// Module Private Functions ----------------------------------------------------
void EXTI4_15_IRQHandler(void)
{
    DMX_Int_Break_Handler();
    EXTI->RPR1 |= 0x00000100;    //PA8 on rising
    EXTI->FPR1 |= 0x00000100;    //PA8 on falling
}


void TimingDelay_Decrement(void)
{
    TIM_Timeouts ();

    if (timer_standby)
        timer_standby--;

    HARD_Timeouts();

    DMX_Int_Millis_Handler ();

    Manager_Timeouts ();
    
    // Modes Menus Timers
    if (ptFTT != NULL)
        ptFTT();

    FiltersAndOffsets_Calc_SM ();
    // USART_Timeouts();
    // Comms_Power_Timeouts ();

// #if (defined USE_TEMP_PROT) || (defined USE_NTC_DETECTION)
//     if (timer_temp)
//         timer_temp--;
// #endif

}

void SysTickError (void)
{
    //Capture systick error...
    while (1)
    {
        if (LED)
            LED_OFF;
        else
            LED_ON;

        for (unsigned char i = 0; i < 255; i++)
        {
            asm ("nop \n\t"
                 "nop \n\t"
                 "nop \n\t" );
        }
    }
}


//--- end of file ---//

