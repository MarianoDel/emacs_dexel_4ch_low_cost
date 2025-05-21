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
// #include "hard.h"

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
    MNGR_WAIT_ENTERING_MAIN_MENU_WAIT_FREE,    
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
// unsigned char mode_state = 0;
// volatile unsigned short mode_effect_timer = 0;

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

unsigned char mngr_main_menu_cnt = 0;

// -- for temp sense
unsigned char check_probe_temp = 0;


// Module Private Functions ----------------------------------------------------
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
		if (pmem->dmx_channel_quantity == 1)
		{
		    dmx_local_data[1] = dmx_local_data[0];
		    dmx_local_data[2] = dmx_local_data[0];
		    dmx_local_data[3] = dmx_local_data[0];		    
		}
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

#ifdef USART2_DEBUG_MODE
        if (action == selection_back)
            Usart2Send("selection back\n");
#endif                
        
        if (action != selection_back)
        {
            resp = Manual_Menu (pmem, action);

            if ((resp == resp_change) ||
                (resp == resp_need_to_save))
            {
		if (pmem->dmx_channel_quantity == 1)
		{
		    dmx_local_data[1] = dmx_local_data[0];
		    dmx_local_data[2] = dmx_local_data[0];
		    dmx_local_data[3] = dmx_local_data[0];		    
		}
                // FiltersAndOffsets_Channels_to_Backup (&(pmem->fixed_channels[0]));
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
        DisconnectByVoltage();

        // Mode Timeout enable
        ptFTT = &Main_Menu_Timeouts;
        
        // clean display
        SCREEN_Text2_BlankLine1();
        SCREEN_Text2_BlankLine2();
        Wait_ms(250);
        mngr_main_menu_cnt = 0;
        mngr_state++;
        break;

    case MNGR_WAIT_ENTERING_MAIN_MENU:
        if (Check_S2() < SW_HALF)
            mngr_state = MNGR_INIT;
        else if (Check_S1() > SW_NO)
        {
            char s_temp[20];
            if (mngr_main_menu_cnt)
            {
                SCREEN_Text2_BlankLine1();
                sprintf(s_temp, "%d", mngr_main_menu_cnt);
                SCREEN_Text2_Line1(s_temp);                
            }
            mngr_state++;
        }
        break;

    case MNGR_WAIT_ENTERING_MAIN_MENU_WAIT_FREE:
        if (Check_S2() < SW_HALF)
            mngr_state = MNGR_INIT;
        else if (Check_S1() == SW_NO)
        {
            if (mngr_main_menu_cnt < 5 - 1)
            {
                mngr_main_menu_cnt++;
                mngr_state--;
            }
            else
                mngr_state++;

        }
        break;
        
    case MNGR_IN_MAIN_MENU:
        action = CheckActions();
            
        resp = Main_Menu (pmem, action);

        // end with some config changes
        if (resp == resp_need_to_save)
        {
            need_to_save_timer = 0;
            need_to_save = 1;
            
            mngr_state = MNGR_INIT;
        }

        // end without changes
        if (resp == resp_ok)
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
            unsigned char temp_deg = 0;
            temp_filtered = MA16_U16Circular(&temp_filter, Temp_Channel);
            temp_deg = Temp_TempToDegreesExtended (temp_filtered);

            if (temp_deg > pmem->temp_prot_deg)
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
                sprintf(s_to_send, "overtemp: %dC %d\n", temp_deg, temp_filtered);
                Usart2Send(s_to_send);
#endif

                do {
                    display_update_int_state_machine();

                    if (!protections_sample_timer)
                    {
                        temp_filtered = MA16_U16Circular(&temp_filter, Temp_Channel);
                        temp_deg = Temp_TempToDegreesExtended (temp_filtered);
                        protections_sample_timer = 10;
                    }
                    
                } while (temp_deg > 48);
                    
                //reconnect
                mngr_state = MNGR_INIT;
            }
            else if (temp_deg > 35)
            {
                CTRL_FAN_ON;
            }
            else if (temp_deg < 30)
            {
                CTRL_FAN_OFF;
            }

            protections_sample_timer = 10;
        }
    }            
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

