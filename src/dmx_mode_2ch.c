//-------------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### DMX_MODE_2CH.C ############################
//-------------------------------------------------

// Includes --------------------------------------------------------------------
#include "dmx_mode_2ch.h"
#include "screen.h"
#include "ssd1306_display.h"
// #include "menu_options_oled.h"
// #include "adc.h"
#include "temperatures.h"
// #include "hard.h"
// #include "manager.h"
#include "dmx_utils.h"

#include <string.h>
#include <stdio.h>


// Module Private Types Constants and Macros -----------------------------------
enum States {
    DMX_MODE_2CH_INIT = 0,
    DMX_MODE_2CH_CHECK_SWITCHES,
    DMX_MODE_2CH_SHOW_DEGREE,
    DMX_MODE_2CH_CHECK_SWITCHES_WAIT_END,
    DMX_MODE_2CH_SHOW_TEMP,
    DMX_MODE_2CH_SHOW_TEMP_WAIT_CHANGE,    
    DMX_MODE_2CH_SHOW_BRIGHT,
    DMX_MODE_2CH_SHOW_BRIGHT_WAIT_CHANGE,
    DMX_MODE_2CH_CHANGE_ADDRESS,
    DMX_MODE_2CH_CHANGING_ADDRESS,
    
};


#define TT_SHOW_OPTIONS    500
#define CNTR_TO_OUT    16

#define TT_DMX_RECEIVING    1000
#define CHANGE_OPT_TT    500
#define OPT_CNT_NEXT    3


// Externals -------------------------------------------------------------------
// - for DMX receiver
extern volatile unsigned char dmx_buff_data[];
extern volatile unsigned char Packet_Detected_Flag;
extern volatile unsigned short DMX_channel_selected;
extern volatile unsigned char DMX_channel_quantity;
extern volatile unsigned char dmx_receive_flag;

extern volatile unsigned short adc_ch [];
extern unsigned char dmx_local_data [];

// -- externals re-used
extern unsigned char mode_state;
extern volatile unsigned short mode_effect_timer;
extern volatile unsigned short dmx_rx_timer;
extern unsigned char mode_cntr_out;
extern unsigned char mode_show_options;

// variables re-use
#define dmx_mode_2ch_state    mode_state
#define dmx_mode_2ch_timer    mode_effect_timer
#define dmx_mode_dmx_receiving_timer    dmx_rx_timer
#define mm_menu_cntr_out    mode_cntr_out
#define show_option    mode_show_options



// Globals ---------------------------------------------------------------------
// unsigned char dmx_mode_2ch_showing = 0;
// unsigned char dmx_mode_2ch_out_cnt = 0;
unsigned char dmx_mode_2ch_update_values = 0;
// unsigned short dmx_mode_dmx_receiving_timer = 0;
// sw_actions_t dmx_mode_2ch_last_action = selection_none;


// Module Private Functions ----------------------------------------------------


// Module Funtions -------------------------------------------------------------
void Dmx_Mode_2Ch_UpdateTimers (void)
{
    if (dmx_mode_2ch_timer)
        dmx_mode_2ch_timer--;

    if (dmx_mode_dmx_receiving_timer)
        dmx_mode_dmx_receiving_timer--;
}


void Dmx_Mode_2Ch_Reset (void)
{
    dmx_mode_2ch_state = DMX_MODE_2CH_INIT;
}


unsigned char last_dmx_data = 0;
resp_t Dmx_Mode_2Ch (parameters_typedef * mem, sw_actions_t actions)
{
    unsigned char dmx_need_display_update = 0;
    resp_t resp = resp_continue;
    char s_temp[20];
    unsigned short * paddr;
    sw_actions_t sw = actions;

    switch (dmx_mode_2ch_state)
    {
    case DMX_MODE_2CH_INIT:
	SCREEN_Clear ();	
	sprintf(s_temp, "D%03d", mem->dmx_first_channel);
	SCREEN_Text4_Line1(s_temp);
	
        dmx_mode_2ch_timer = 300;
        dmx_need_display_update = 1;
        dmx_mode_2ch_state++;
	break;

    case DMX_MODE_2CH_CHECK_SWITCHES:
        if (dmx_mode_2ch_timer)
            break;

        if (sw == selection_dwn)
        {
	    // show current temp degree
	    SCREEN_Clear ();	
	    sprintf(s_temp, "%dC",
		    Temp_TempToDegreesExtended (Temp_Probe_Meas_Filtered_Get()));
	    SCREEN_Text4_Line1(s_temp);

	    dmx_need_display_update = 1;
	    dmx_mode_2ch_timer = 1500;
            dmx_mode_2ch_state = DMX_MODE_2CH_SHOW_DEGREE;
        }
	
	else if (sw == selection_up)
	{
	    dmx_mode_2ch_timer = 3000;
	    dmx_mode_2ch_state = DMX_MODE_2CH_CHECK_SWITCHES_WAIT_END;
	}
	break;

    case DMX_MODE_2CH_SHOW_DEGREE:
	if ((!dmx_mode_2ch_timer) && (sw == selection_none))
	{
	    dmx_mode_2ch_state = DMX_MODE_2CH_INIT;
	}	
        break;

    case DMX_MODE_2CH_CHECK_SWITCHES_WAIT_END:
	if (sw == selection_none) 
	{
	    // up and release, go to change address
            show_option = 0;
            dmx_mode_2ch_timer = TT_SHOW_OPTIONS;
            mm_menu_cntr_out = CNTR_TO_OUT;
	    dmx_mode_2ch_state = DMX_MODE_2CH_CHANGE_ADDRESS; 
	}
	
	if (!dmx_mode_2ch_timer)
	{
	    if (sw == selection_up)
	    {
		unsigned short color = 0;
		unsigned char temp = dmx_local_data[1];
		// still up, go to show bright
		// show_option = 0;
		// dmx_mode_2ch_timer = 3000;
		// mm_menu_cntr_out = CNTR_TO_OUT;

		// show current temp
		SCREEN_Clear ();
		
		if (mem->program_type == CCT1_MODE)
		    ColorTemp1 (temp, &color);
		else
		    ColorTemp2 (temp, &color);
	    
		sprintf(s_temp, "%dK", color);
		SCREEN_Text4_Line1(s_temp);

		last_dmx_data = dmx_local_data[1];
		dmx_mode_2ch_state = DMX_MODE_2CH_SHOW_TEMP;
		dmx_need_display_update = 1;
	    }
	}
        break;

    case DMX_MODE_2CH_SHOW_TEMP:
	// check if temp changes
	if (last_dmx_data != dmx_local_data[1])
	{
	    unsigned short color = 0;
	    unsigned char temp = dmx_local_data[1];
	    
	    SCREEN_Clear ();

	    if (mem->program_type == CCT1_MODE)
		ColorTemp1 (temp, &color);
	    else
		ColorTemp2 (temp, &color);
	    
	    sprintf(s_temp, "%dK", color);
	    SCREEN_Text4_Line1(s_temp);

	    last_dmx_data = dmx_local_data[1];
	    dmx_need_display_update = 1;
	}
	
	else if (sw == selection_up)    // and timer??
	{
	    // printf("selection_up in SHOW_TEMP\n");
	    dmx_mode_2ch_timer = 3000;
	    dmx_mode_2ch_state = DMX_MODE_2CH_SHOW_TEMP_WAIT_CHANGE;
	}
	break;
	
    case DMX_MODE_2CH_SHOW_TEMP_WAIT_CHANGE:
	if (sw == selection_none) 
	{
	    // printf("selection_none in SHOW_TEMP_WAIT_CHANGE\n");
	    // up and release, go to show temp
	    dmx_mode_2ch_state = DMX_MODE_2CH_SHOW_TEMP; 
	}

	if (!dmx_mode_2ch_timer)
	{
	    if (sw == selection_up)
	    {
		unsigned char bright_int = 0;
		unsigned char bright_dec = 0;	    
		unsigned char bright = dmx_local_data[0];

		// show current bright
		SCREEN_Clear ();
		Percentage (bright, &bright_int, &bright_dec);
		sprintf(s_temp, "%2d.%d%%", bright_int, bright_dec);
		
		if (bright_int == 100)
		    SCREEN_Text4_Line1("100\% ");
		else
		    SCREEN_Text4_Line1(s_temp);

		last_dmx_data = bright;
		dmx_mode_2ch_state = DMX_MODE_2CH_SHOW_BRIGHT;
		dmx_need_display_update = 1;		
	    }
	}
	break;

    case DMX_MODE_2CH_SHOW_BRIGHT:
	// check if bright changes
	if (last_dmx_data != dmx_local_data[0])
	{
	    unsigned char bright_int = 0;
	    unsigned char bright_dec = 0;	    
	    unsigned char bright = dmx_local_data[0];

	    // show current bright
	    SCREEN_Clear ();
	    Percentage (bright, &bright_int, &bright_dec);
	    sprintf(s_temp, "%2d.%d%%", bright_int, bright_dec);
	
	    if (bright_int == 100)
		SCREEN_Text4_Line1("100\% ");
	    else
		SCREEN_Text4_Line1(s_temp);

	    last_dmx_data = bright;
	    dmx_need_display_update = 1;	    
	}
	
	else if (sw == selection_up)    // and timer??
	{
	    dmx_mode_2ch_timer = 3000;
	    dmx_mode_2ch_state = DMX_MODE_2CH_SHOW_BRIGHT_WAIT_CHANGE;
	}
	break;

    case DMX_MODE_2CH_SHOW_BRIGHT_WAIT_CHANGE:
	if (sw == selection_none) 
	{
	    // up and release, go to show bright
	    dmx_mode_2ch_state = DMX_MODE_2CH_SHOW_BRIGHT; 
	}

	if (!dmx_mode_2ch_timer)
	{
	    if (sw == selection_up)
	    {
		dmx_mode_2ch_state = DMX_MODE_2CH_INIT;
	    }
	}
	break;
	
    case DMX_MODE_2CH_CHANGE_ADDRESS:	
        paddr = &(mem->dmx_first_channel);

	if ((sw == selection_up) ||
            (sw == selection_up_fast))
        {
            if (sw == selection_up_fast)
            {
		if (*paddr < 512 - mem->dmx_channel_quantity - 1 -10)
                    *paddr += 10;
		else
		    *paddr = 512 - mem->dmx_channel_quantity - 1;
            }
            else
            {
		if (*paddr < 512 - mem->dmx_channel_quantity - 1)
                    *paddr += 1;
            }

	    SCREEN_Clear ();	
	    sprintf(s_temp, "D%03d", mem->dmx_first_channel);
	    SCREEN_Text4_Line1(s_temp);

	    dmx_need_display_update = 1;	    
            show_option = 1;
            dmx_mode_2ch_timer = TT_SHOW_OPTIONS;
            resp = resp_change;
        }

        if ((sw == selection_dwn) ||
            (sw == selection_dwn_fast))
        {
            if (sw == selection_dwn_fast)
            {
		if (*paddr > 10)
                    *paddr -= 10;
		else
		    *paddr = 1;
            }
            else
            {
		if (*paddr > 1)
                    *paddr -= 1;
            }

	    SCREEN_Clear ();	
	    sprintf(s_temp, "D%03d", mem->dmx_first_channel);
	    SCREEN_Text4_Line1(s_temp);

	    dmx_need_display_update = 1;	    
            show_option = 1;
            dmx_mode_2ch_timer = TT_SHOW_OPTIONS;
            resp = resp_change;
        }

        if (!dmx_mode_2ch_timer)
        {
            if (show_option)
            {
                show_option = 0;
		SCREEN_Clear();
            }
            else
            {
		dmx_need_display_update = 1;
                show_option = 1;
		SCREEN_Clear ();	
		sprintf(s_temp, "D%03d", mem->dmx_first_channel);
		SCREEN_Text4_Line1(s_temp);
            }

            if (mm_menu_cntr_out)
                mm_menu_cntr_out--;
            
            dmx_mode_2ch_timer = TT_SHOW_OPTIONS;
        }

        if (!mm_menu_cntr_out)
            dmx_mode_2ch_state = DMX_MODE_2CH_INIT;

        break;
    }        

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
                dmx_mode_2ch_update_values = 1;
            }
        }
    }

    if (dmx_need_display_update)
        display_update();

    return resp;
    
}


unsigned short Dmx_Mode_2ch_GetPacketsTimer (void)
{
    return dmx_mode_dmx_receiving_timer;
}

//--- end of file ---//
