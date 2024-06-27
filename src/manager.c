//----------------------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### MANAGER.C ##########################################
//----------------------------------------------------------

// Includes --------------------------------------------------------------------
#include "manager.h"
// #include "dmx_transceiver.h"
#include "adc.h"
#include "usart.h"
#include "tim.h"

// #include "stm32f10x.h"

#include "filters_and_offsets.h"
#include "flash_program.h"

// #include "comm.h"
#include "dsp.h"

// linked modules
#include "screen.h"
#include "ssd1306_display.h"
#include "parameters.h"
// #include "dmx1_mode.h"
// #include "dmx2_mode.h"
// #include "master_slave_mode.h"
// #include "manual_mode.h"
#include "temperatures.h"
// #include "options_menu.h"
// #include "reset_mode.h"
#include "main_menu.h"

#ifdef DMX_ONLY_MODE
#include "hardware_mode.h"
#endif
#ifdef DMX_AND_CCT_MODE
#include "cct_hardware_mode.h"
#endif

#include <stdio.h>
#include <string.h>
// Module Private Types & Macros -----------------------------------------------
typedef enum {
    INIT,
    GET_CONF,
    MNGR_DMX1_MODE,
    MNGR_DMX2_MODE,
    MNGR_MASTER_SLAVE_MODE,
    MNGR_MANUAL_MODE,
    MNGR_RESET_MODE,
    MNGR_ENTERING_MAIN_MENU,
    MNGR_ENTERING_MAIN_MENU_WAIT_FREE,
    MNGR_IN_MAIN_MENU,
    MNGR_ENTERING_HARDWARE_MENU,
    MNGR_ENTERING_HARDWARE_MENU_WAIT_FREE,
    MNGR_IN_HARDWARE_MENU
    
} manager_states_e;


// Externals -------------------------------------------------------------------
extern volatile unsigned short DMX_channel_selected;
extern volatile unsigned char DMX_channel_quantity;
extern volatile unsigned char Packet_Detected_Flag;
// extern volatile unsigned char DMX_packet_flag;

extern void (* ptFTT ) (void);

extern volatile unsigned short adc_ch [];

// -- Externals for the modes
unsigned char mode_state = 0;
volatile unsigned short mode_effect_timer = 0;

// -- Externals for the menues
unsigned char menu_state = 0;
unsigned char menu_selected = 0;
unsigned char menu_need_display_update = 0;
unsigned char menu_selection_show = 0;
volatile unsigned short menu_menu_timer = 0;
// options_menu_st mem_options;
unsigned char menu_counter_out = 0;

extern volatile unsigned short dac_chnls [];
extern volatile unsigned char pwm_chnls[];

// -- for temp sense
extern ma16_u16_data_obj_t temp_filter;


// Globals ---------------------------------------------------------------------
manager_states_e mngr_state = INIT;
char s_to_send [100];
unsigned char ch_values [6] = { 0 };
unsigned char need_to_save = 0;

// module timeouts
volatile unsigned short need_to_save_timer = 0;
volatile unsigned short timer_mngr = 0;

#if (defined USE_OVERTEMP_PROT) || \
    (defined USE_VOLTAGE_PROT) || \
    (defined USE_NTC_DETECTION)
volatile unsigned char protections_sample_timer = 0;
// volatile unsigned short protections_sample_timer = 5000;    //start with this if initial overtemp fails exists
#endif

// -- for temp sense
unsigned char check_ntc = 0;


// Module Private Functions ----------------------------------------------------
unsigned char CheckTempGreater (unsigned short temp_sample, unsigned short temp_prot);
void DisconnectByVoltage (void);
void DisconnectChannels (void);

// Module Functions ------------------------------------------------------------
void Manager (parameters_typedef * pmem)
{
//     sw_actions_t action = do_nothing;
//     resp_t resp = resp_continue;
    
//     switch (mngr_state)
//     {
//     case INIT:
//         // hardware reset
//         DMX_Disable();

//         // channels reset
//         FiltersAndOffsets_Filters_Reset();

//         // start and clean filters
//         DisconnectChannels();
        
// #ifdef USART_DEBUG_MODE            
//         sprintf(s_to_send, "prog type: %d\n", pmem->program_type);
//         UsartDebug(s_to_send);
//         Wait_ms(100);
// #endif

//         // Init Program Screen
//         strcpy(s_to_send, "         ");
//         strcpy(s_to_send + 20, "         ");
//         switch (pmem->program_type)
//         {
//         case DMX1_MODE:
//             strcpy(s_to_send, "  DMX1 ");
//             break;
//         case DMX2_MODE:
//             strcpy(s_to_send, "  DMX2 ");
//             break;
//         case MASTER_SLAVE_MODE:
//             strcpy(s_to_send, "  Master ");
//             strcpy(s_to_send + 20, "    Slave");                
//             break;
//         case MANUAL_MODE:
//             strcpy(s_to_send, "  Manual ");
//             break;

//         case RESET_MODE:
//             strcpy(s_to_send, "   Reset ");
//             break;
                
//         }
            
//         SCREEN_ShowText2(
//             " Running ",
//             " on      ",
//             s_to_send,
//             s_to_send + 20
//             );
//         timer_mngr = 500;
    
//         while (timer_mngr)
//             display_update_int_state_machine();

//         mngr_state++;            
//         break;

//     case GET_CONF:

//         if (pmem->program_type == DMX1_MODE)
//         {
//             //reception variables
//             Packet_Detected_Flag = 0;
//             DMX_channel_selected = pmem->dmx_first_channel;
//             DMX_channel_quantity = pmem->dmx_channel_quantity;

//             //Mode Timeout enable
//             ptFTT = &DMX1Mode_UpdateTimers;

//             //packet reception enable
//             DMX_EnableRx();

//             //enable int outputs
//             FiltersAndOffsets_Enable_Outputs();
                
//             DMX1ModeReset();
//             mngr_state = MNGR_DMX1_MODE;
//         }

//         if (pmem->program_type == DMX2_MODE)
//         {
//             //reception variables
//             Packet_Detected_Flag = 0;
//             DMX_channel_selected = pmem->dmx_first_channel;
//             DMX_channel_quantity = 4 + pmem->dmx_channel_quantity;

//             //Mode Timeout enable
//             ptFTT = &DMX2Mode_UpdateTimers;

//             //packet reception enable
//             DMX_EnableRx();

//             //enable int outputs
//             FiltersAndOffsets_Enable_Outputs();            
                
//             DMX2ModeReset();
//             mngr_state = MNGR_DMX2_MODE;
//         }
            
//         if (pmem->program_type == MASTER_SLAVE_MODE)
//         {
//             //reception variables for slave mode
//             Packet_Detected_Flag = 0;
//             DMX_channel_selected = pmem->dmx_first_channel;
//             DMX_channel_quantity = pmem->dmx_channel_quantity;

//             //enable int outputs
//             FiltersAndOffsets_Enable_Outputs();            

//             //Mode Timeout enable
//             ptFTT = &MasterSlaveMode_UpdateTimers;
                
//             MasterSlaveModeReset();
                
//             mngr_state = MNGR_MASTER_SLAVE_MODE;
//         }

//         if (pmem->program_type == MANUAL_MODE)
//         {
//             //enable int outputs
//             FiltersAndOffsets_Enable_Outputs();

//             //Mode Timeout enable
//             ptFTT = &ManualMode_UpdateTimers;
                
//             ManualModeReset();
                
//             mngr_state = MNGR_MANUAL_MODE;
//         }

//         if (pmem->program_type == RESET_MODE)
//         {                
//             ResetModeReset();                
//             mngr_state = MNGR_RESET_MODE;
//         }
//         break;

//     case MNGR_DMX1_MODE:
//         // Check encoder first
//         action = CheckActions();
            
//         resp = DMX1Mode (ch_values, action);

//         if (resp == resp_change)
//         {
//             FiltersAndOffsets_Channels_to_Backup(ch_values);
//         }

//         if (resp == resp_need_to_save)
//         {
//             need_to_save_timer = 10000;
//             need_to_save = 1;
//         }

//         if ((CheckSET() > SW_MIN) &&
//             (!Hard_Enter_Is_Block()))
//             mngr_state = MNGR_ENTERING_MAIN_MENU;
            
//         UpdateEncoder();

            
// #ifdef USART_DEBUG_MODE
//         if (!timer_mngr)
//         {
//             timer_mngr = 1000;

//             sprintf(s_to_send, "c1: %d, c2: %d, c3: %d, c4: %d, c5: %d, c6: %d\n",
//                     *(ch_values + 0),
//                     *(ch_values + 1),
//                     *(ch_values + 2),
//                     *(ch_values + 3),
//                     *(ch_values + 4),
//                     *(ch_values + 5));
//             UsartDebug(s_to_send);

//             sprintf(s_to_send, "d1: %d, d2: %d, d3: %d, d4: %d, d5: %d, d6: %d\n",
//                     dac_chnls[0],
//                     dac_chnls[1],
//                     dac_chnls[2],
//                     dac_chnls[3],
//                     dac_chnls[4],
//                     dac_chnls[5]);
//             UsartDebug(s_to_send);

//             sprintf(s_to_send, "p1: %d, p2: %d, p3: %d, p4: %d, p5: %d, p6: %d\n",
//                     pwm_chnls[0],
//                     pwm_chnls[1],
//                     pwm_chnls[2],
//                     pwm_chnls[3],
//                     pwm_chnls[4],
//                     pwm_chnls[5]);
//             UsartDebug(s_to_send);
//         }
// #endif
            
//         break;

//     case MNGR_DMX2_MODE:
//         // Check encoder first
//         action = CheckActions();
            
//         resp = DMX2Mode (ch_values, action);

//         if (resp == resp_change)
//         {
//             FiltersAndOffsets_Channels_to_Backup(ch_values);
//         }

//         if (resp == resp_need_to_save)
//         {
//             need_to_save_timer = 10000;
//             need_to_save = 1;
//         }

//         if ((CheckSET() > SW_MIN) &&
//             (!Hard_Enter_Is_Block()))
//             mngr_state = MNGR_ENTERING_MAIN_MENU;
            
//         UpdateEncoder();            

            
// #ifdef USART_DEBUG_MODE
//         if (!timer_mngr)
//         {
//             timer_mngr = 1000;

//             sprintf(s_to_send, "c1: %d, c2: %d, c3: %d, c4: %d, c5: %d, c6: %d\n",
//                     *(ch_values + 0),
//                     *(ch_values + 1),
//                     *(ch_values + 2),
//                     *(ch_values + 3),
//                     *(ch_values + 4),
//                     *(ch_values + 5));
//             UsartDebug(s_to_send);

//             sprintf(s_to_send, "d1: %d, d2: %d, d3: %d, d4: %d, d5: %d, d6: %d\n",
//                     dac_chnls[0],
//                     dac_chnls[1],
//                     dac_chnls[2],
//                     dac_chnls[3],
//                     dac_chnls[4],
//                     dac_chnls[5]);
//             UsartDebug(s_to_send);

//             sprintf(s_to_send, "p1: %d, p2: %d, p3: %d, p4: %d, p5: %d, p6: %d\n",
//                     pwm_chnls[0],
//                     pwm_chnls[1],
//                     pwm_chnls[2],
//                     pwm_chnls[3],
//                     pwm_chnls[4],
//                     pwm_chnls[5]);
//             UsartDebug(s_to_send);
            
//         }
// #endif
            
//         break;
//     case MNGR_MASTER_SLAVE_MODE:
//         // Check encoder first
//         action = CheckActions();

//         resp = MasterSlaveMode (pmem, action);

//         if ((resp == resp_change) ||
//             (resp == resp_change_all_up))    //fixed mode save and change
//         {
//             //TODO: check how to do this in the new version data512?
//             // data512[0] = 0;
//             // for (unsigned char n = 0; n < sizeof(ch_values); n++)
//             // {
//             //     ch_values[n] = pmem->fixed_channels[n];
//             //     data512[n + 1] = ch_values[n];
//             // }

//             FiltersAndOffsets_Channels_to_Backup (ch_values);
            
//             if (resp == resp_change_all_up)    //fixed mode changes will be saved
//                 resp = resp_need_to_save;                
//         }

//         if (!timer_mngr)
//         {
//             if ((pmem->program_inner_type == MASTER_INNER_FIXED_MODE) ||
//                 (pmem->program_inner_type == MASTER_INNER_SKIPPING_MODE) ||
//                 (pmem->program_inner_type == MASTER_INNER_GRADUAL_MODE) ||
//                 (pmem->program_inner_type == MASTER_INNER_STROBE_MODE))
//             {
//                 timer_mngr = 40;
//                 SendDMXPacket (PCKT_INIT);
//             }
//         }

//         if (resp == resp_need_to_save)
//         {
//             need_to_save_timer = 10000;
//             need_to_save = 1;
//         }

//         if (CheckSET() > SW_MIN)
//             mngr_state = MNGR_ENTERING_MAIN_MENU;

//         UpdateEncoder();
            
//         break;

//     case MNGR_MANUAL_MODE:
//         // Check encoder first
//         action = CheckActions();

//         resp = ManualMode (pmem, action);

//         if ((resp == resp_change) ||
//             (resp == resp_change_all_up))    //fixed mode save and change
//         {
//             for (unsigned char n = 0; n < sizeof(ch_values); n++)
//                 ch_values[n] = pmem->fixed_channels[n];

//             FiltersAndOffsets_Channels_to_Backup (ch_values);
            
//             if (resp == resp_change_all_up)
//                 resp = resp_need_to_save;
//         }

//         if (resp == resp_need_to_save)
//         {
//             need_to_save_timer = 10000;
//             need_to_save = 1;
//         }

//         if (CheckSET() > SW_MIN)
//             mngr_state = MNGR_ENTERING_MAIN_MENU;

//         UpdateEncoder();
            
//         break;


//     case MNGR_RESET_MODE:
//         // Check encoder first
//         action = CheckActions();

//         resp = ResetMode (pmem, action);

//         if (resp == resp_finish)
//         {
//             // if (pmem->program_type == RESET_MODE)    //not save, go to main menu again
//             mngr_state = MNGR_ENTERING_MAIN_MENU;
//         }

//         if (resp == resp_need_to_save)
//         {
//             need_to_save_timer = 100;    //save almost instantly
//             need_to_save = 1;
//             mngr_state = INIT;
//         }

//         if (CheckSET() > SW_MIN)
//             mngr_state = MNGR_ENTERING_MAIN_MENU;

//         UpdateEncoder();
            
//         break;

//     case MNGR_ENTERING_MAIN_MENU:
//         //deshabilitar salidas hardware
//         DMX_Disable();
            
//         //reseteo canales
//         DisconnectChannels();

//         MainMenuReset();

//         SCREEN_ShowText2(
//             "Entering ",
//             " Main    ",
//             "  Menu   ",
//             "         "
//             );
            
//         mngr_state++;
//         break;

//     case MNGR_ENTERING_MAIN_MENU_WAIT_FREE:
//         if (CheckSET() == SW_NO)
//         {
//             mngr_state++;
//         }
//         break;
            
//     case MNGR_IN_MAIN_MENU:
//         // Check encoder first
//         action = CheckActions();

//         resp = MainMenu(pmem, action);

//         if (resp == resp_need_to_save)
//         {
// #ifdef SAVE_FLASH_IMMEDIATE
//             need_to_save_timer = 0;
// #endif
// #ifdef SAVE_FLASH_WITH_TIMEOUT
//             need_to_save_timer = 10000;
// #endif
//             need_to_save = 1;
//             mngr_state = INIT;
//         }
            
//         if (resp == resp_finish)
//             mngr_state = INIT;

//         UpdateEncoder();

//         if (CheckSET() > SW_HALF)
//             mngr_state = MNGR_ENTERING_HARDWARE_MENU;
            
//         break;

//     case MNGR_ENTERING_HARDWARE_MENU:
// #if (defined DMX_ONLY_MODE)
//         HardwareModeReset();

//         //Mode Timeout enable
//         ptFTT = &HardwareMode_UpdateTimers;
// #elif (defined DMX_AND_CCT_MODE)
//         Cct_Hardware_Mode_Reset ();

//         //Mode Timeout enable
//         ptFTT = &Cct_Hardware_Mode_UpdateTimers;
// #else
// #error "No mode selected on hard.h"    
// #endif                    

//         SCREEN_ShowText2(
//             "Entering ",
//             " Hardware",
//             "  Menu   ",
//             "         "
//             );
            
//         mngr_state++;
//         break;

//     case MNGR_ENTERING_HARDWARE_MENU_WAIT_FREE:
//         if (CheckSET() == SW_NO)
//         {
//             mngr_state++;
//         }
//         break;
            
//     case MNGR_IN_HARDWARE_MENU:
//         // Check encoder first
//         action = CheckActions();

// #if (defined DMX_ONLY_MODE)

//         resp = HardwareMode(pmem, action);

// #elif (defined DMX_AND_CCT_MODE)

//         resp = Cct_Hardware_Mode (pmem, action);
//         if ((resp == resp_need_to_save) &&
//             ((pmem->program_type == CCT1_MODE) || (pmem->program_type == CCT2_MODE)))                
//         {
//             // save inmediatly and reboot
//             Flash_WriteConfigurations();
            
//             NVIC_SystemReset();
            
//         }
// #else
// #error "No mode selected on hard.h"    
// #endif

//         if ((resp == resp_need_to_save) ||
//             (resp == resp_finish))
//         {
//             //hardware config its saved instantly
//             need_to_save = 1;
//             mngr_state = INIT;
//         }

//         UpdateEncoder();
//         break;
            
//     default:
//         mngr_state = INIT;
//         break;
//     }

//     //cuestiones generales        
//     UpdateSwitches();

//     // update de LCD
//     display_update_int_state_machine();

//     // colors commands update from comms
//     UpdateCommunications();

//     // now call it by tim6 on pwm_handler int
//     // DAC_MUX_Update(dac_chnls);


// #ifdef USE_OVERTEMP_PROT
//     if (check_ntc)    //NTC NOT SHORTED
//     {
//         if ((mngr_state < MNGR_ENTERING_MAIN_MENU) &&
//             (!protections_sample_timer))
//         {
//             unsigned short temp_filtered = 0;
//             temp_filtered = MA16_U16Circular(&temp_filter, Temp_Channel);

//             if (CheckTempGreater (temp_filtered, pmem->temp_prot))
//             {
//                 //stop LEDs outputs
//                 DisconnectByVoltage();
//                 CTRL_FAN_ON;

//                 SCREEN_ShowText2(
//                     "LEDs     ",
//                     "Overtemp ",
//                     "         ",
//                     "         "
//                     );

// #ifdef USART2_DEBUG_MODE
//                 sprintf(s_to_send, "overtemp: %d\n", temp_filtered);
//                 Usart2Send(s_to_send);
// #endif

//                 do {
//                     display_update_int_state_machine();
//                 } while (CheckTempGreater (Temp_Channel, TEMP_RECONNECT));
                    
//                 //reconnect
//                 mngr_state = INIT;
//             }
//             else if (CheckTempGreater (temp_filtered, TEMP_IN_35))
//             {
//                 CTRL_FAN_ON;
//             }
//             else if (CheckTempGreater (TEMP_IN_30, temp_filtered))
//             {
//                 CTRL_FAN_OFF;
//             }
                

// #ifdef USE_NTC_DETECTION
//             // check for ntc and stop
//             if (temp_filtered > NTC_DISCONNECTED)
//             {
//                 //stop LEDs outputs
//                 DisconnectByVoltage();
//                 CTRL_FAN_ON;
            
//                 SCREEN_ShowText2(
//                     "         ",
//                     " No NTC  ",
//                     "Connected",
//                     "         "
//                     );

//                 do {
//                     display_update_int_state_machine();                    
//                 } while (Temp_Channel > NTC_DISCONNECTED);

//                 //reconnect
//                 mngr_state = INIT;
//             }
// #endif    // USE_NTC_DETECTION
//         }
//     }    // check_ntc
// #endif    // USE_OVERTEMP_PROT

// #ifdef USE_VOLTAGE_PROT
//     if ((mngr_state < MNGR_ENTERING_MAIN_MENU) &&
//         (!protections_sample_timer))
//     {
//         if (V_Sense_48V > MAX_PWR_SUPPLY)
//         {
//             DisconnectByVoltage();

//             SCREEN_ShowText2(
//                 "Power    ",
//                 "   Over  ",
//                 " Voltage!",
//                 "         "
//                 );

// #ifdef USART2_DEBUG_MODE
//             sprintf(s_to_send, "overvoltage: %d\n", V_Sense_48V);
//             Usart2Send(s_to_send);
// #endif
//             do {
//                 display_update_int_state_machine();
//             } while (V_Sense_48V > MAX_PWR_SUPPLY_RECONNECT);

//             //reconnect
//             mngr_state = INIT;
                
//         }
//         else if (V_Sense_48V < MIN_PWR_SUPPLY)
//         {
//             DisconnectByVoltage();

//             SCREEN_ShowText2(
//                 "Power    ",
//                 " is Too  ",
//                 "  Low!   ",
//                 "         "
//                 );

// #ifdef USART2_DEBUG_MODE
//             sprintf(s_to_send, "undervoltage: %d\n", V_Sense_48V);
//             Usart2Send(s_to_send);
// #endif
//             do {
//                 display_update_int_state_machine();
//             } while (V_Sense_48V < MIN_PWR_SUPPLY_RECONNECT);

//             //reconnect
//             mngr_state = INIT;
//         }
//     }
// #endif    // USE_VOLTAGE_PROT

//     if (!protections_sample_timer)
//         protections_sample_timer = 10;
        

//     // save flash after configs
//     if ((need_to_save) && (!need_to_save_timer))
//     {
//         need_to_save = Flash_WriteConfigurations();

// #ifdef USART_DEBUG_MODE
//         if (need_to_save)
//             UsartDebug((char *) "Memory Saved OK!\n");
//         else
//             UsartDebug((char *) "Memory problems\n");
// #endif

//         need_to_save = 0;
//     }
        
}


void Manager_Ntc_Set (void)
{
    check_ntc = 1;
}


void Manager_Ntc_Reset (void)
{
    check_ntc = 0;
}


void Manager_Timeouts (void)
{
    if (need_to_save_timer)
        need_to_save_timer--;

    if (timer_mngr)
        timer_mngr--;    
    
#if (defined USE_VOLTAGE_PROT) || \
    (defined USE_OVERTEMP_PROT) || \
    (defined USE_NTC_DETECTION)
    if (protections_sample_timer)
        protections_sample_timer--;
#endif

}


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


void DisconnectByVoltage (void)
{
    DMX_Disable();
    FiltersAndOffsets_Disable_Outputs ();
    FiltersAndOffsets_Channels_Reset ();
    TIM_Deactivate_Channels (0x3F);
    // CTRL_FAN_OFF;
}


void DisconnectChannels (void)
{
    FiltersAndOffsets_Disable_Outputs ();
    FiltersAndOffsets_Channels_Reset ();
    TIM_Deactivate_Channels (0x3F);
}


//--- end of file ---//

