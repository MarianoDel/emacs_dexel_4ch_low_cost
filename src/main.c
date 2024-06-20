//--------------------------------------------------
// #### PROJECT: 2CH 4A - 8A DMX - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### MAIN.C #####################################
//--------------------------------------------------

// Includes --------------------------------------------------------------------
#include "stm32g0xx.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "gpio.h"
#include "tim.h"
#include "usart.h"
#include "hard.h"
// #include "comm.h"
#include "parameters.h"

#include "core_cm0plus.h"
#include "adc.h"
#include "dma.h"
#include "flash_program.h"

#include "test_functions.h"
#include "lcd_utils.h"
#include "dmx_receiver.h"
#include "temperatures.h"

#include "dmx_mode.h"
#include "manual_mode.h"
#include "menues.h"
#include "comms_power.h"

#include "dsp.h"


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
volatile unsigned short need_to_save_timer = 0;

#ifdef USE_TEMP_PROT
unsigned short timer_temp = 0;
ma16_u16_data_obj_t temp_filter;
#endif

// -- for the timeouts in the modes ----
void (* ptFTT ) (void) = NULL;

// -- for the memory -------------------
unsigned char need_to_save = 0;


// Module Private Functions ----------------------------------------------------
void TimingDelay_Decrement(void);
void SysTickError (void);
unsigned char CheckTempGreater (unsigned short temp_sample, unsigned short temp_prot);


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

    //--- Funciones de Test Hardware ---
    // TF_Hardware_Tests ();    
    //--- Fin Funciones de Test Hardware ---    

    
    //--- Welcome code ---//
    // Usart and Timer for DMX
    Usart1Config ();
    TIM_14_Init ();
    DMX_DisableRx ();

    // Usart for comms with power
    Usart2Config ();
    
    // ADC & DMA for temp sense
    AdcConfig();
    DMAConfig();
    DMA1_Channel1->CCR |= DMA_CCR_EN;
    ADC1->CR |= ADC_CR_ADSTART;
    
    // LCD Init and Welcome Code
    LCD_UtilsInit();
    CTRL_BKL_ON;

    while (LCD_ShowBlink("Kirno Technology",
                         "  Smart Driver  ",
                         1,
                         BLINK_NO) != resp_finish);

    while (LCD_ShowBlink(" Dexel          ",
                         "  Lighting      ",
                         2,
                         BLINK_NO) != resp_finish);

    
    // Production Program ---------------------------
    sw_actions_t action = selection_none;
    resp_t resp = resp_continue;
    unsigned char ch_values [2] = { 0 };
    main_state_e main_state = MAIN_INIT;
    unsigned char packet_cnt = 0;

    // check NTC connection on init
    unsigned char check_ntc = 0;
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
        check_ntc = 0;
    }
    else
        check_ntc = 1;
    // check NTC connection on init    
    
    while (1)
    {
        switch (main_state)
        {
        case MAIN_INIT:
            // get saved config or create one for default
            if (pflash_mem->program_type != 0xff)
            {
                //memory with valid data
                memcpy(&mem_conf, pflash_mem, sizeof(parameters_typedef));
            }
            else
            {
                //hardware defaults
                mem_conf.temp_prot = TEMP_IN_70;    //70 degrees
                mem_conf.temp_prot_deg = TEMP_DEG_DEFAULT;    //70 degrees
                mem_conf.current_int = 4;
                mem_conf.current_dec = 0;
                mem_conf.channels_operation_mode = CCT1_MODE;

                mem_conf.dmx_first_channel = 1;
                mem_conf.dmx_channel_quantity = 2;                
                mem_conf.program_type = AUTODETECT_MODE;

#if (defined USE_ENCODER_DIRECT)
                mem_conf.encoder_direction = 0;
#elif (defined USE_ENCODER_INVERT)
                mem_conf.encoder_direction = 1;
#else
#error "Please select default encoder direction on hard.h"
#endif
                
            }

#ifdef USE_TEMP_PROT
            for (int i = 0; i < 16; i++)
                temp_filtered = MA16_U16Circular(&temp_filter, Temp_Channel);
#endif    //USE_TEMP_PROT

            main_state++;
            break;

        case MAIN_HARD_INIT:

            //reseteo hardware
            DMX_DisableRx();

            // //reseteo canales
            // PWMChannelsReset();

            // //limpio los filtros
            // FiltersAndOffsets_Filters_Reset();

            // check and set conf current
            unsigned char cint = mem_conf.current_int;
            unsigned char cdec = mem_conf.current_dec;            
            char config = 1;
            while (config)
            {
                resp = Comms_Power_Send_Current_Conf (cint, cdec);

                if (resp == resp_ok)
                    config = 0;

                if (resp == resp_timeout)
                {
                    config = 0;
                    while (LCD_ShowBlink("No comms wth pwr",
                                         "currnt no config",
                                         1,
                                         BLINK_NO) != resp_finish);
                    
                }
            }

            // check power board version
            config = 1;
            while (config)
            {
                resp = Comms_Power_Get_Version ();

                if (resp == resp_ok)
                    config = 0;

                if (resp == resp_timeout)
                {
                    config = 0;
                    while (LCD_ShowBlink("No comms wth pwr",
                                         " version unknown",
                                         1,
                                         BLINK_NO) != resp_finish);
                    
                }
            }
            

            // packet reception enable
            DMX_EnableRx();
            timer_standby = 1000;    //one second for dmx detection
            mem_conf.program_type = AUTODETECT_MODE;

            main_state++;
            break;

        case MAIN_CHECK_CONF:
            if (Packet_Detected_Flag)
            {
                Packet_Detected_Flag = 0;
                packet_cnt++;
            }

            if ((packet_cnt > 5) &&
                (timer_standby))
            {
                mem_conf.program_type = DMX_MODE;
                main_state = MAIN_DMX_MODE_INIT;
            }
            else if (!timer_standby)
            {
                mem_conf.program_type = MANUAL_MODE;
                main_state = MAIN_MANUAL_MODE_INIT;
            }
            break;

        case MAIN_DMX_MODE_INIT:
            // reception variables
            DMX_channel_selected = mem_conf.dmx_first_channel;
            DMX_channel_quantity = mem_conf.dmx_channel_quantity;

            // Force first screen
            Packet_Detected_Flag = 1;
            dmx_buff_data[0] = 0;
            dmx_buff_data[1] = 0;
            dmx_buff_data[2] = 0;

            // Mode Timeout enable
            ptFTT = &DMXMode_UpdateTimers;

            // packet reception enable
            DMX_EnableRx();

            DMXModeReset();
            main_state = MAIN_IN_DMX_MODE;
            packet_cnt = 0;    // reset packet counter for autodetection
            break;

        case MAIN_MANUAL_MODE_INIT:
            // packet reception disable, check for colors
            DMX_DisableRx();

            // Mode Timeout enable
            ptFTT = &ManualMode_UpdateTimers;

            for (unsigned char n = 0; n < sizeof(ch_values); n++)
                ch_values[n] = mem_conf.fixed_channels[n];

            Comms_Power_Send_Bright(ch_values);
                
            ManualModeReset();                
            main_state = MAIN_IN_MANUAL_MODE;
            packet_cnt = 0;    // reset packet counter for autodetection
            break;
            
        case MAIN_IN_DMX_MODE:
            // Check encoder first
            action = CheckActions();

            if (action != selection_back)
            {
                
                resp = DMXMode (ch_values, action);

                if (resp == resp_change)
                {
                    Comms_Power_Send_Bright(ch_values);
                }

                if (resp == resp_need_to_save)
                {
                    need_to_save_timer = 10000;
                    need_to_save = 1;
                }
            }
            else
                main_state = MAIN_ENTERING_MAIN_MENU;

            // Manual mode autodetection
            if (DMXGetPacketTimer () == 0)
            {
                if (!timer_standby)
                {
                    if (packet_cnt < 2)
                    {
                        packet_cnt++;
                        timer_standby = 200;
                    }
                    else
                    {
                        // manual detection
                        main_state = MAIN_MANUAL_MODE_INIT;
                        mem_conf.program_type = MANUAL_MODE;
                    }
                }
            }

#ifdef HARDWARE_VERSION_2_1
            UpdateEncoder();
#endif
            break;

        case MAIN_IN_MANUAL_MODE:
            // Check encoder first
            action = CheckActions();

            if (action != selection_back)
            {
                resp = ManualMode (ch_values, action);

                if (resp == resp_change)
                {
                    for (unsigned char n = 0; n < sizeof(ch_values); n++)
                        mem_conf.fixed_channels[n] = ch_values[n];

                    Comms_Power_Send_Bright(ch_values);
                }

                if (resp == resp_need_to_save)
                {
                    need_to_save_timer = 10000;
                    need_to_save = 1;
                }
            }
            else
                main_state = MAIN_ENTERING_MAIN_MENU;

            // Dmx presence autodetection
            if (dmx_receive_flag)
            {
                dmx_receive_flag = 0;
                packet_cnt++;
                timer_standby = 1000;
            }

            if (packet_cnt > 5)
            {
                if (timer_standby)
                {
                    // dmx detection
                    main_state = MAIN_DMX_MODE_INIT;
                    mem_conf.program_type = DMX_MODE;                    
                }
                else
                {
                    // dmx not present, reset the counter
                    packet_cnt = 0;
                }
            }

#ifdef HARDWARE_VERSION_2_1
            UpdateEncoder();
#endif
            break;

        case MAIN_ENTERING_MAIN_MENU:
            // hardware outputs disable
            DMX_DisableRx();

            // channels reset
            for (unsigned char n = 0; n < sizeof(ch_values); n++)
                ch_values[n] = 0;

            Comms_Power_Send_Bright(ch_values);

            // clean display
            LCD_Writel1(s_blank);
            LCD_Writel2(s_blank);
            Wait_ms(250);
            main_state++;
            break;

        case MAIN_ENTERING_MAIN_MENU2:
#ifdef HARDWARE_VERSION_2_1
            if (CheckSET() < SW_HALF)
                main_state = MAIN_HARD_INIT;
            else
            {
                MENU_Main_Reset();
                main_state++;
            }
#endif
#ifdef HARDWARE_VERSION_2_0
            if (Check_SW_SEL() < SW_HALF)
                main_state = MAIN_HARD_INIT;
            else if ((Check_SW_UP() > SW_NO) &&
                     (Check_SW_DWN() > SW_NO))
            {
                MENU_Main_Reset();
                main_state++;
            }
#endif
            break;
            
        case MAIN_IN_MAIN_MENU:
            action = CheckActions();
            
            resp = MENU_Main(&mem_conf, action);

            if (resp == resp_need_to_save)
            {
                need_to_save_timer = 0;
                need_to_save = 1;
                
                main_state = MAIN_HARD_INIT;
            }
            
            if (resp == resp_finish)
                main_state = MAIN_HARD_INIT;

#ifdef HARDWARE_VERSION_2_1
            UpdateEncoder();
#endif            
            break;

        default:
            main_state = MAIN_INIT;
            break;
        }

        // memory savings after config
        if ((need_to_save) && (!need_to_save_timer))
        {
            __disable_irq();
            need_to_save = Flash_WriteConfigurations(
                (uint32_t *) &mem_conf,
                sizeof(parameters_typedef));
            __enable_irq();

            // if (need_to_save != FLASH_COMPLETE)
            // {
            //     while (LCD_ShowBlink("Flash: error    ",
            //                          "   not saved!!! ",
            //                          1,
            //                          BLINK_NO) != resp_finish);
            // }
            // else
            // {
            //     while (LCD_ShowBlink("Flash:          ",
            //                          "   saved ok!    ",
            //                          1,
            //                          BLINK_NO) != resp_finish);                
            // }
                
            need_to_save = 0;
        }

        // things that not depends on the main status
        UpdateSwitches();

        Comms_Power_Update();

#ifdef USE_TEMP_PROT
        if (check_ntc)    //NTC NOT SHORTED
        {
            if ((main_state < MAIN_ENTERING_MAIN_MENU) &&
                (!timer_temp))
            {
                timer_temp = 100;
                temp_filtered = MA16_U16Circular(&temp_filter, Temp_Channel);

                if (CheckTempGreater (temp_filtered, mem_conf.temp_prot))            
                {
                    //deshabilitar salidas hardware
                    DMX_DisableRx();

                    // reset the channels
                    for (unsigned char n = 0; n < sizeof(ch_values); n++)
                        ch_values[n] = 0;

                    Comms_Power_Send_Bright(ch_values);
                    
                    CTRL_FAN_ON;

                    while (LCD_ShowBlink("  Overtemp!!!   ",
                                         " LEDs shutdown  ",
                                         1,
                                         BLINK_NO) != resp_finish);

                    // go out of here without filter or jumper
                    unsigned char loop = 1;
                    while (loop)
                    {
                        if (Temp_Channel < NTC_SHORTED)    //sensor with jumper
                            loop = 0;
                        else if (CheckTempGreater (TEMP_RECONNECT, Temp_Channel))
                            loop = 0;
                        else
                            Wait_ms(100);
                    }

                    //reconnect
                    main_state = MAIN_INIT;
                
                }
                else if (CheckTempGreater (temp_filtered, TEMP_IN_35))
                {
                    CTRL_FAN_ON;
                }
                else if (CheckTempGreater (TEMP_IN_30, temp_filtered))
                {
                    CTRL_FAN_OFF;
                }

                // check for ntc and stop                
                if (temp_filtered > NTC_DISCONNECTED)
                {
                    //deshabilitar salidas hardware
                    DMX_DisableRx();

                    // reset the channels
                    for (unsigned char n = 0; n < sizeof(ch_values); n++)
                        ch_values[n] = 0;

                    Comms_Power_Send_Bright(ch_values);

                    CTRL_FAN_ON;

                    while (LCD_ShowBlink("  No NTC        ",
                                         "    connected!  ",
                                         1,
                                         BLINK_NO) != resp_finish);

                    // go out of here without filter
                    while (Temp_Channel > NTC_DISCONNECTED)
                    {
                        Wait_ms(100);
                    }

                    //reconnect
                    main_state = MAIN_INIT;
                }
            }
        }    // ntc not shorted
#endif    //USE_TEMP_PROT        
        
#ifdef USE_CTROL_FAN_ALWAYS_ON
        CTRL_FAN_ON;
#endif
        
    }    //end of while 1

    return 0;
}

//--- End of Main ---//


// Module Private Functions ----------------------------------------------------
unsigned char CheckTempGreater (unsigned short temp_sample, unsigned short temp_prot)
{
    unsigned char is_greater = 0;

#ifdef TEMP_SENSOR_LM335
    if (temp_sample > temp_prot)
        is_greater = 1;
#endif
#ifdef TEMP_SENSOR_NTC1K
    if (temp_sample < temp_prot)    // see it in voltage
        is_greater = 1;
#endif
    
    return is_greater;
}


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

    if (need_to_save_timer)
        need_to_save_timer--;

    LCD_UpdateTimer ();

    HARD_Timeouts();

    DMX_Int_Millis_Handler ();
    
    // Modes Menus Timers
    if (ptFTT != NULL)
        ptFTT();

    // USART_Timeouts();
    Comms_Power_Timeouts ();

#if (defined USE_TEMP_PROT) || (defined USE_NTC_DETECTION)
    if (timer_temp)
        timer_temp--;
#endif

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

