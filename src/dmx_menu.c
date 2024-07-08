//-------------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### DMX_MENU.C #############################
//-------------------------------------------------

// Includes --------------------------------------------------------------------
#include "dmx_menu.h"
#include "screen.h"
#include "ssd1306_display.h"
#include "menu_options_oled.h"
#include "adc.h"
#include "temperatures.h"

#include <string.h>
#include <stdio.h>


// Module Private Types Constants and Macros -----------------------------------
typedef enum {
    DMX_MENU_INIT = 0,
    DMX_MENU_SHOW_FIRST,
    DMX_MENU_STANDBY,    
    DMX_MENU_CHANGE_ADDRESS,
    DMX_MENU_CHANGING_ADDRESS,
    DMX_MENU_SHOW_TEMP
    
} dmx_menu_state_e;

#define TT_DMX_RECEIVING    1000

// Externals -------------------------------------------------------------------
// - for DMX receiver
extern volatile unsigned char dmx_buff_data[];
extern volatile unsigned char Packet_Detected_Flag;
extern volatile unsigned short DMX_channel_selected;
extern volatile unsigned char DMX_channel_quantity;
extern volatile unsigned char dmx_receive_flag;

extern volatile unsigned short adc_ch [];

// Globals ---------------------------------------------------------------------
volatile unsigned short dmx_menu_timer = 0;
unsigned char dmx_menu_showing = 0;
unsigned char dmx_menu_out_cnt = 0;
dmx_menu_state_e dmx_state = DMX_MENU_INIT;
unsigned char dmx_menu_update_values = 0;
unsigned short dmx_mode_dmx_receiving_timer = 0;

unsigned char dmx_local_data [4] = { 0 };
// Module Private Functions ----------------------------------------------------


// Module Funtions -------------------------------------------------------------
void Dmx_Menu_Timeouts (void)
{
    if (dmx_menu_timer)
        dmx_menu_timer--;

    if (dmx_mode_dmx_receiving_timer)
        dmx_mode_dmx_receiving_timer--;
}

void Dmx_Menu_Reset (void)
{
    dmx_state = DMX_MENU_INIT;
}


resp_t Dmx_Menu (parameters_typedef * mem, sw_actions_t actions)
{
    unsigned char dmx_need_display_update = 0;
    resp_t resp = resp_continue;
    char s_temp[20];

    switch (dmx_state)
    {
    case DMX_MENU_INIT:
        SCREEN_Text2_BlankLine1();
        SCREEN_Text2_BlankLine2();
        SCREEN_Text2_Line1("   Dmx    ");
        SCREEN_Text2_Line2("   Mode   ");        
        dmx_menu_timer = 800;
        dmx_need_display_update = 1;
        dmx_state++;
        break;

    case DMX_MENU_SHOW_FIRST:
        if (dmx_menu_timer)
            break;
        
        SCREEN_Text2_BlankLine1();
        SCREEN_Text2_BlankLine2();

        sprintf(s_temp, "R%3d  G%3d",
                dmx_local_data[0],
                dmx_local_data[1]);
        SCREEN_Text2_Line1(s_temp);

        sprintf(s_temp, "B%3d  W%3d",
                dmx_local_data[2],
                dmx_local_data[3]);
        SCREEN_Text2_Line2(s_temp);

        dmx_need_display_update = 1;
        // resp = resp_change;    //first colors update
        dmx_state++;
        break;
        
    case DMX_MENU_STANDBY:
        if (actions == selection_enter)
        {
            Options_Up_Dwn_Select_Reset();
            dmx_menu_out_cnt = 20;
            dmx_state++;
        }

        if (actions == selection_up)
        {
            char s_temp [20];
            SCREEN_Text2_BlankLine1();
            SCREEN_Text2_BlankLine2();
            if (Manager_Probe_Temp_Get())
            {
                sprintf(s_temp, "Temp: %dC", Temp_TempToDegrees (Temp_Channel));
                SCREEN_Text2_Line1(s_temp);
            }
            else
                SCREEN_Text2_Line1("Temp: NC");
            
            dmx_menu_timer = 1200;
            dmx_state = DMX_MENU_SHOW_TEMP;
        }
        
        if (dmx_menu_update_values)
        {
            dmx_menu_update_values = 0;
            dmx_state--;
        }
        break;

    case DMX_MENU_CHANGE_ADDRESS:

        SCREEN_Text2_BlankLine1();
        if (dmx_menu_showing)
        {
            sprintf(s_temp, "Addr:  %3d", mem->dmx_first_channel);
            SCREEN_Text2_Line1(s_temp);            
        }
        else
            SCREEN_Text2_Line1("Addr:");

        dmx_need_display_update = 1;
        dmx_state++;
        break;

    case DMX_MENU_CHANGING_ADDRESS:
        
        resp = Options_Up_Dwn_Select (actions);

        if (resp != resp_continue)
        {
            dmx_need_display_update = 1;
            dmx_menu_out_cnt = 20;
            dmx_menu_timer = 500;
            dmx_menu_showing = 1;
        }
        
        if (resp == resp_up)
        {
            unsigned short * pch = &(mem->dmx_first_channel);
            // if actions selection_all_up, change fast
            if (*pch < 512 - mem->dmx_channel_quantity)
                *pch += 1;

            dmx_state--;
        }

        if (resp == resp_dwn)
        {
            unsigned short * pch = &(mem->dmx_first_channel);
            // if actions selection_all_up, change fast
            if (*pch > 1)
                *pch -= 1;

            dmx_state--;            
        }

        if (!dmx_menu_timer)
        {
            dmx_menu_timer = 500;
            if (dmx_menu_showing)
                dmx_menu_showing = 0;
            else
                dmx_menu_showing = 1;

            dmx_state--;
            dmx_menu_out_cnt--;

            if (!dmx_menu_out_cnt)
            {
                resp = resp_need_to_save;
                dmx_state = DMX_MENU_SHOW_FIRST;                
            }
        }

        if (resp == resp_ok)
        {
            resp = resp_need_to_save;
            dmx_state = DMX_MENU_SHOW_FIRST;
        }
        break;

    case DMX_MENU_SHOW_TEMP:
        if (!dmx_menu_timer)
            dmx_state = DMX_MENU_SHOW_FIRST;
        
        break;
        
    }        

    if (dmx_need_display_update)
        display_update();

    //dmx update - generally on 40Hz -
    if (Packet_Detected_Flag)
    {
        Packet_Detected_Flag = 0;

        // tell the manager we still getting packets
        dmx_mode_dmx_receiving_timer = TT_DMX_RECEIVING;
        
        if (dmx_buff_data[0] == 0)
        {
            if ((dmx_local_data[0] != dmx_buff_data[1]) ||
                (dmx_local_data[1] != dmx_buff_data[2]) ||
                (dmx_local_data[2] != dmx_buff_data[3]) ||
                (dmx_local_data[3] != dmx_buff_data[4]))
            {
                dmx_local_data[0] = dmx_buff_data[1];
                dmx_local_data[1] = dmx_buff_data[2];
                dmx_local_data[2] = dmx_buff_data[3];
                dmx_local_data[3] = dmx_buff_data[4];

                // else if (idle_pckt_cnt > 40)
                // {
                //     idle_pckt_cnt = 0;
                //     update_anyway = 1;
                // }
                // else
                //     idle_pckt_cnt++;

                resp = resp_change;
                dmx_menu_update_values = 1;
            }
        }
    }

    return resp;
    
}


unsigned short Dmx_Menu_GetPacketsTimer (void)
{
    return dmx_mode_dmx_receiving_timer;
}

//--- end of file ---//
