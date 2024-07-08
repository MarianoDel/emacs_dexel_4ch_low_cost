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
#include "dmx_receiver.h"
#include "adc.h"
#include "usart.h"
#include "tim.h"
#include "flash_program.h"

#include "filters_and_offsets.h"
// #include "comm.h"
#include "dsp.h"

// linked modules
#include "screen.h"
#include "ssd1306_display.h"
#include "parameters.h"

#include "dmx_menu.h"
#include "manual_menu.h"
#include "main_menu.h"

#include "temperatures.h"

#include <stdio.h>
#include <string.h>

// Module Private Types & Macros -----------------------------------------------
typedef enum {
    MNGR_INIT,
    MNGR_CHECK_CONF,    
    MNGR_DMX_MODE_INIT,
    MNGR_MANUAL_MODE_INIT,
    MNGR_IN_DMX_MODE,
    MNGR_IN_MANUAL_MODE,
    MNGR_ENTERING_MAIN_MENU,
    MNGR_WAIT_ENTERING_MAIN_MENU,
    MNGR_IN_MAIN_MENU
    
} manager_states_e;


// Externals -------------------------------------------------------------------
extern volatile unsigned short DMX_channel_selected;
extern volatile unsigned char DMX_channel_quantity;
extern volatile unsigned char Packet_Detected_Flag;
extern volatile unsigned char dmx_buff_data[];
extern volatile unsigned char dmx_receive_flag;
// extern volatile unsigned char DMX_packet_flag;

extern void (* ptFTT ) (void);

extern volatile unsigned short adc_ch [];

// -- Externals for the modes
unsigned char mode_state = 0;
volatile unsigned short mode_effect_timer = 0;

extern volatile unsigned short timer_standby;
// -- Externals for the menues
// unsigned char menu_state = 0;
// unsigned char menu_selected = 0;
// unsigned char menu_need_display_update = 0;
// unsigned char menu_selection_show = 0;
// volatile unsigned short menu_menu_timer = 0;
// // options_menu_st mem_options;
// unsigned char menu_counter_out = 0;

// extern volatile unsigned short dac_chnls [];
// extern volatile unsigned char pwm_chnls[];

// -- for temp sense
extern ma16_u16_data_obj_t temp_filter;
extern unsigned char dmx_local_data [];

// Globals ---------------------------------------------------------------------
manager_states_e mngr_state = MNGR_INIT;
// unsigned char ch_values [6] = { 0 };
unsigned char need_to_save = 0;

// module timeouts
volatile unsigned short need_to_save_timer = 0;
volatile unsigned short timer_mngr = 0;
volatile unsigned char protections_sample_timer = 0;


// -- for temp sense
unsigned char check_probe_temp = 0;


// Module Private Functions ----------------------------------------------------
unsigned char CheckTempGreater (unsigned short temp_sample, unsigned short temp_prot);
void DisconnectByVoltage (void);
void DisconnectChannels (void);

// Module Functions ------------------------------------------------------------
void Manager_Timeouts (void)
{
    if (need_to_save_timer)
        need_to_save_timer--;

    if (timer_mngr)
        timer_mngr--;    
    
    if (protections_sample_timer)
        protections_sample_timer--;
}

unsigned char packet_cnt = 0;
unsigned char showing_temp = 0;
void Manager (parameters_typedef * pmem)
{
    sw_actions_t action = selection_none;
    resp_t resp = resp_continue;
    
    switch (mngr_state)
    {
    case MNGR_INIT:
        // hardware reset
        DMX_DisableRx ();

        // channels reset
        FiltersAndOffsets_Filters_Reset();

        // start and clean filters
        DisconnectChannels();
        
        // packet reception enable
        DMX_EnableRx();
        timer_standby = 1000;    //one second for dmx detection

        mngr_state++;
        break;

    case MNGR_CHECK_CONF:
        if (Packet_Detected_Flag)
        {
            Packet_Detected_Flag = 0;
            packet_cnt++;
        }

        if ((packet_cnt > 5) &&
            (timer_standby))
        {
            mngr_state = MNGR_DMX_MODE_INIT;
        }
        else if (!timer_standby)
        {
            mngr_state = MNGR_MANUAL_MODE_INIT;
        }
        break;
    
    case MNGR_DMX_MODE_INIT:
        // reception variables
        DMX_channel_selected = pmem->dmx_first_channel;
        DMX_channel_quantity = pmem->dmx_channel_quantity;

        // Force first screen
        Packet_Detected_Flag = 1;
        dmx_buff_data[0] = 0;
        dmx_buff_data[1] = 0;
        dmx_buff_data[2] = 0;

        // Mode Timeout enable
        ptFTT = &Dmx_Menu_Timeouts;

        // packet reception enable
        DMX_EnableRx();

        // enable pwm outputs
        FiltersAndOffsets_Enable_Outputs ();

        Dmx_Menu_Reset ();
        mngr_state = MNGR_IN_DMX_MODE;
        packet_cnt = 0;    // reset packet counter for autodetection
        break;

    case MNGR_MANUAL_MODE_INIT:
        // packet reception disable, check for colors
        DMX_DisableRx();

        // Mode Timeout enable
        ptFTT = &Manual_Menu_Timeouts;

        // enable pwm outputs
        FiltersAndOffsets_Enable_Outputs ();
        
        Manual_Menu_Reset ();
        mngr_state = MNGR_IN_MANUAL_MODE;
        packet_cnt = 0;    // reset packet counter for autodetection
        break;
            
    case MNGR_IN_DMX_MODE:
        // Check encoder first
        action = CheckActions();

        if (action != selection_back)
        {            
            resp = Dmx_Menu (pmem, action);

            if (resp == resp_change)
            {
                FiltersAndOffsets_Channels_to_Backup (dmx_local_data);
            }

            if (resp == resp_need_to_save)
            {
                need_to_save_timer = 10000;
                need_to_save = 1;
            }
        }
        else
            mngr_state = MNGR_ENTERING_MAIN_MENU;

        // Manual mode autodetection
        if (Dmx_Menu_GetPacketsTimer () == 0)
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
                    mngr_state = MNGR_MANUAL_MODE_INIT;
                }
            }
        }
        break;

    case MNGR_IN_MANUAL_MODE:
        // Check encoder first
        action = CheckActions();
        
        if (action != selection_back)
        {
            resp = Manual_Menu (pmem, action);

            if ((resp == resp_change) ||
                (resp == resp_need_to_save))
            {
                FiltersAndOffsets_Channels_to_Backup (&(pmem->fixed_channels[0]));
            }

            if (resp == resp_need_to_save)
            {
                need_to_save_timer = 10000;
                need_to_save = 1;
// #ifdef USART2_DEBUG_MODE
//                 Usart2Send("manual ask save\r\n");
// #endif                
            }
        }
        else
            mngr_state = MNGR_ENTERING_MAIN_MENU;

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
                mngr_state = MNGR_DMX_MODE_INIT;
            }
            else
            {
                // dmx not present, reset the counter
                packet_cnt = 0;
            }
        }
        break;

    case MNGR_ENTERING_MAIN_MENU:
        // hardware outputs disable
        DMX_DisableRx();

        // channels reset
        // for (unsigned char n = 0; n < sizeof(ch_values); n++)
        //     ch_values[n] = 0;

        // Comms_Power_Send_Bright(ch_values);

        // Mode Timeout enable
        ptFTT = &Main_Menu_Timeouts;
        
        // clean display
        SCREEN_Text2_BlankLine1();
        SCREEN_Text2_BlankLine2();
        Wait_ms(250);
        mngr_state++;
        break;

    case MNGR_WAIT_ENTERING_MAIN_MENU:
        if (Check_S2() < SW_HALF)
            mngr_state = MNGR_INIT;
        else if (Check_S1() > SW_NO)
        {
            Main_Menu_Reset();
            mngr_state++;
        }
        break;
            
    case MNGR_IN_MAIN_MENU:
        action = CheckActions();
            
        resp = Main_Menu (pmem, action);

        if (resp == resp_need_to_save)
        {
            need_to_save_timer = 0;
            need_to_save = 1;
            
            mngr_state = MNGR_INIT;
        }
            
        if (resp == resp_finish)
            mngr_state = MNGR_INIT;

        break;

    default:
        mngr_state = MNGR_INIT;
        break;
    }

    // general things
    HARD_UpdateSwitches();

    // update the oled
    display_update_int_state_machine();
    
    // save flash after configs
    if ((need_to_save) && (!need_to_save_timer))
    {
        // need_to_save = Flash_WriteConfigurations();

        __disable_irq();
        need_to_save = Flash_WriteConfigurations(
            (uint32_t *) pmem,
            sizeof(parameters_typedef));
        __enable_irq();

#ifdef USART2_DEBUG_MODE
        if (need_to_save)
            Usart2Send((char *) "Memory Saved OK!\n");
        else
            Usart2Send((char *) "Memory problems\n");
#endif

        need_to_save = 0;
    }

    // Check Temp prot
    if (Manager_Probe_Temp_Get())
    {
        if ((mngr_state < MNGR_ENTERING_MAIN_MENU) &&
            (!protections_sample_timer))
        {
            unsigned short temp_filtered = 0;
            temp_filtered = MA16_U16Circular(&temp_filter, Temp_Channel);

            if (CheckTempGreater (temp_filtered, pmem->temp_prot))
            {
                //stop LEDs outputs
                DisconnectByVoltage();
                CTRL_FAN_ON;

                SCREEN_Text2_BlankLine1();
                SCREEN_Text2_BlankLine2();
                SCREEN_Text2_Line1("LEDs      ");
                SCREEN_Text2_Line2("Overtemp  ");        

#ifdef USART2_DEBUG_MODE
                char s_to_send[30];
                sprintf(s_to_send, "overtemp: %d\n", temp_filtered);
                Usart2Send(s_to_send);
#endif

                do {
                    display_update_int_state_machine();
                } while (CheckTempGreater (Temp_Channel, TEMP_RECONNECT));
                    
                //reconnect
                mngr_state = MNGR_INIT;
            }
            else if (CheckTempGreater (temp_filtered, TEMP_IN_35))
            {
                CTRL_FAN_ON;
            }
            else if (CheckTempGreater (TEMP_IN_30, temp_filtered))
            {
                CTRL_FAN_OFF;
            }

            protections_sample_timer = 10;
        }
    }
    
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


unsigned char Manager_Probe_Temp_Get (void)
{
    return check_probe_temp;
}
        

void Manager_Probe_Temp_Set (void)
{
    check_probe_temp = 1;
}


void Manager_Probe_Temp_Reset (void)
{
    check_probe_temp = 0;
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
    DMX_DisableRx();
    FiltersAndOffsets_Disable_Outputs ();
    FiltersAndOffsets_Channels_Reset ();
    // TIM_Deactivate_Channels (0x3F);
    // CTRL_FAN_OFF;
}


void DisconnectChannels (void)
{
    FiltersAndOffsets_Disable_Outputs ();
    FiltersAndOffsets_Channels_Reset ();
    // TIM_Deactivate_Channels (0x3F);
}


//--- end of file ---//

