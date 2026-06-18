//-------------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### MANUAL_MODE_2CH.C #########################
//-------------------------------------------------

// Includes --------------------------------------------------------------------
#include "manual_mode_2ch.h"
#include "screen.h"
#include "ssd1306_display.h"

#include "dmx_utils.h"
// #include "parameters.h"
#include "temperatures.h"

#include <stdio.h>
#include <string.h>


// Private Types Constants and Macros ------------------------------------------
enum States {
    MM_2CH_INIT = 0,
    MM_2CH_SHOW_TEMP,
    MM_2CH_SHOW_TEMP_WAIT_END,    
    MM_2CH_SHOW_BRIGHT,
    MM_2CH_SHOW_BRIGHT_WAIT_END,    
    MM_2CH_SELECT_TEMP,
    MM_2CH_SELECT_BRIGHT,
    MM_2CH_SHOW_DEGREE
    
};


enum To_Show {
    SHOW_TEMP_MODE1,
    SHOW_TEMP_MODE2,
    SHOW_BRIGHT,
    SHOW_DEGREE
};


#define TT_SHOW_OPTIONS    500
#define CNTR_TO_OUT    16
#define DUMMY_DEGREE    1000


// Externals -------------------------------------------------------------------
// -- externals re-used
extern unsigned char mode_state;
extern volatile unsigned short mode_effect_timer;
extern unsigned char mode_cntr_out;
extern unsigned char mode_show_options;
extern volatile unsigned char sw_wait_free_timer;
extern unsigned char sw_fast;

// variables re-use
#define manual_mode_2ch_state    mode_state
#define manual_mode_2ch_timer    mode_effect_timer
#define mm_menu_cntr_out    mode_cntr_out
#define show_option    mode_show_options



// Globals ---------------------------------------------------------------------
// volatile unsigned short mm_serial_timer = 0;
// volatile unsigned short mm_serial_need_to_save_timer = 0;
// unsigned char mm_serial_need_to_save_flag = 0;



// Module Private Functions ----------------------------------------------------
void DataShow_2Ch (unsigned char to_show,
		   unsigned char bright,
		   unsigned char temp,
		   unsigned short degree);


// Module Functions ------------------------------------------------------------
void ManualMode_2Ch_UpdateTimers (void)
{
    if (manual_mode_2ch_timer)
        manual_mode_2ch_timer--;

    if (sw_wait_free_timer)
        sw_wait_free_timer--;

    // if (mm_serial_need_to_save_timer)
    //     mm_serial_need_to_save_timer--;
    
}


void ManualMode_2Ch_Reset (void)
{
    manual_mode_2ch_state = MM_2CH_INIT;
}



resp_t ManualMode_2Ch (parameters_typedef * mem, sw_actions_t action)
{
    unsigned char manual_need_display_update = 0;
    resp_t resp = resp_continue;
    unsigned char * ch = &mem->fixed_channels[0];
    unsigned char program = mem->program_type;
    sw_actions_t sw = action;
    
    switch (manual_mode_2ch_state)
    {
    case MM_2CH_INIT:
	if (program == CCT1_MODE)
	{
	    DataShow_2Ch (SHOW_TEMP_MODE1,
			  *(ch + 0),
			  *(ch + 1),
			  DUMMY_DEGREE);
	}
	else
	{
	    DataShow_2Ch (SHOW_TEMP_MODE2,
			  *(ch + 0),
			  *(ch + 1),
			  DUMMY_DEGREE);
	}        

	manual_need_display_update = 1;
        manual_mode_2ch_state++;
	// first update
	resp = resp_change;
        break;

    case MM_2CH_SHOW_TEMP:
        if (sw == selection_dwn)
        {
	    // show current temp degree
            DataShow_2Ch (SHOW_DEGREE,
			  *(ch + 0),
			  *(ch + 1),
			  Temp_Probe_Meas_Filtered_Get());

	    manual_need_display_update = 1;
	    manual_mode_2ch_timer = 1500;
            manual_mode_2ch_state = MM_2CH_SHOW_DEGREE;
        }
	
	else if (sw == selection_up)
	{
	    manual_mode_2ch_timer = 3000;
	    manual_mode_2ch_state = MM_2CH_SHOW_TEMP_WAIT_END;
	}
        break;

    case MM_2CH_SHOW_TEMP_WAIT_END:
	if (sw == selection_none) 
	{
	    // up and release, go to change temp
            show_option = 0;
            manual_mode_2ch_timer = TT_SHOW_OPTIONS;
            mm_menu_cntr_out = CNTR_TO_OUT;
	    manual_mode_2ch_state = MM_2CH_SELECT_TEMP; 
	}
	
	if (!manual_mode_2ch_timer)
	{
	    if (sw == selection_up)
	    {
		// still up, go for show bright
		DataShow_2Ch (SHOW_BRIGHT,
			      *(ch + 0),
			      *(ch + 1),
			      DUMMY_DEGREE);
		
	        manual_need_display_update = 1;
		manual_mode_2ch_state = MM_2CH_SHOW_BRIGHT;
	    }
	}
        break;

    case MM_2CH_SHOW_BRIGHT:
        if (sw == selection_dwn)
        {
	    // show current temp degree
            DataShow_2Ch (SHOW_DEGREE,
			  *(ch + 0),
			  *(ch + 1),
			  Temp_Probe_Meas_Filtered_Get());

	    manual_need_display_update = 1;
	    manual_mode_2ch_timer = 1500;
            manual_mode_2ch_state = MM_2CH_SHOW_DEGREE;
        }
	
	else if (sw == selection_up)
	{
	    manual_mode_2ch_timer = 3000;
	    manual_mode_2ch_state = MM_2CH_SHOW_BRIGHT_WAIT_END;
	}
	break;

    case MM_2CH_SHOW_BRIGHT_WAIT_END:
	if (sw == selection_none) 
	{
	    // up and release, go to change bright
            show_option = 0;
            manual_mode_2ch_timer = TT_SHOW_OPTIONS;
            mm_menu_cntr_out = CNTR_TO_OUT;
	    manual_mode_2ch_state = MM_2CH_SELECT_BRIGHT; 
	}
	
	if (!manual_mode_2ch_timer)
	{
	    if (sw == selection_up)
	    {
		// still up, go to init
		manual_mode_2ch_state = MM_2CH_INIT;
	    }
	}
        break;

    case MM_2CH_SELECT_BRIGHT:
	if (sw == selection_none)
	    sw_fast = 0;
	
        if (sw == selection_up)
        {
	    if (sw_wait_free_timer)
		break;
	    
	    sw_wait_free_timer = 200;
	    sw_fast++;

	    if (sw_fast > 10)
	    {
		if (*(ch + 0) < 255 - 10)
		    *(ch + 0) += 10;
		else
		    *(ch + 0) = 255;
	    }
	    else if (sw_fast > 5)
	    {
		if (*(ch + 0) < 255 - 5)
		    *(ch + 0) += 5;
		else
		    *(ch + 0) = 255;
	    }
	    else
	    {
		if (*(ch + 0) < 255)
		    *(ch + 0) += 1;
	    }
	    
            DataShow_2Ch (SHOW_BRIGHT,
			  *(ch + 0),
			  *(ch + 1),
			  DUMMY_DEGREE);
            
	    manual_need_display_update = 1;
	    show_option = 1;
            mm_menu_cntr_out = CNTR_TO_OUT;	    
            manual_mode_2ch_timer = TT_SHOW_OPTIONS;
            resp = resp_change;
        }

        if (sw == selection_dwn)
        {
	    if (sw_wait_free_timer)
		break;
	    
	    sw_wait_free_timer = 200;
	    sw_fast++;

	    if (sw_fast > 10)
	    {
		if (*(ch + 0) > 10)
		    *(ch + 0) -= 10;
		else
		    *(ch + 0) = 0;
	    }
	    else if (sw_fast > 5)
	    {
		if (*(ch + 0) > 5)
		    *(ch + 0) -= 5;
		else
		    *(ch + 0) = 0;
	    }
	    else
	    {
		if (*(ch + 0) > 0)
		    *(ch + 0) -= 1;
	    }
	    
            DataShow_2Ch (SHOW_BRIGHT,
			  *(ch + 0),
			  *(ch + 1),
			  DUMMY_DEGREE);
            
	    manual_need_display_update = 1;
	    show_option = 1;
            mm_menu_cntr_out = CNTR_TO_OUT;	    
            manual_mode_2ch_timer = TT_SHOW_OPTIONS;
            resp = resp_change;
        }

        if (!manual_mode_2ch_timer)
        {
            if (show_option)
            {
                show_option = 0;
		SCREEN_Clear();
		manual_need_display_update = 1;
            }
            else
            {
                show_option = 1;
                DataShow_2Ch (SHOW_BRIGHT,
			      *(ch + 0),
			      *(ch + 1),
			      DUMMY_DEGREE);
		manual_need_display_update = 1;
            }

            if (mm_menu_cntr_out)
                mm_menu_cntr_out--;
            
            manual_mode_2ch_timer = TT_SHOW_OPTIONS;
        }

        if (!mm_menu_cntr_out)
	{
	    DataShow_2Ch (SHOW_BRIGHT,
			  *(ch + 0),
			  *(ch + 1),
			  DUMMY_DEGREE);

	    manual_need_display_update = 1;
            manual_mode_2ch_state = MM_2CH_SHOW_BRIGHT;
	    resp = resp_need_to_save;
	}
        break;

    case MM_2CH_SELECT_TEMP:
	if (sw == selection_none)
	    sw_fast = 0;

        if (sw == selection_up)
        {
	    if (sw_wait_free_timer)
		break;
	    
	    sw_wait_free_timer = 200;
	    sw_fast++;
	    
	    if (sw_fast > 10)
	    {
		if (*(ch + 1) < 255 - 10)
		    *(ch + 1) += 10;
		else
		    *(ch + 1) = 255;
	    }
	    else if (sw_fast > 5)
	    {
		if (*(ch + 1) < 255 - 5)
		    *(ch + 1) += 5;
		else
		    *(ch + 1) = 255;
	    }
	    else
	    {
		if (*(ch + 1) < 255)
		    *(ch + 1) += 1;
	    }

	    if (program == CCT1_MODE)
	    {
		DataShow_2Ch (SHOW_TEMP_MODE1,
			      *(ch + 0),
			      *(ch + 1),
			      DUMMY_DEGREE);
	    }
	    else
	    {
		DataShow_2Ch (SHOW_TEMP_MODE2,
			      *(ch + 0),
			      *(ch + 1),
			      DUMMY_DEGREE);
	    }
	    manual_need_display_update = 1;
            show_option = 1;
            mm_menu_cntr_out = CNTR_TO_OUT;	    
            manual_mode_2ch_timer = TT_SHOW_OPTIONS;
            resp = resp_change;            
        }

        if (sw == selection_dwn)
        {
	    if (sw_wait_free_timer)
		break;
	    
	    sw_wait_free_timer = 200;
	    sw_fast++;
	    
	    if (sw_fast > 10)
	    {
		if (*(ch + 1) > 10)
		    *(ch + 1) -= 10;
		else
		    *(ch + 1) = 0;
	    }
	    else if (sw_fast > 5)
	    {
		if (*(ch + 1) > 5)
		    *(ch + 1) -= 5;
		else
		    *(ch + 1) = 0;
	    }
	    else
	    {
		if (*(ch + 1) > 0)
		    *(ch + 1) -= 1;
	    }

	    if (program == CCT1_MODE)
	    {
		DataShow_2Ch (SHOW_TEMP_MODE1,
			      *(ch + 0),
			      *(ch + 1),
			      DUMMY_DEGREE);
	    }
	    else
	    {
		DataShow_2Ch (SHOW_TEMP_MODE2,
			      *(ch + 0),
			      *(ch + 1),
			      DUMMY_DEGREE);
	    }
	    manual_need_display_update = 1;
            show_option = 1;
	    mm_menu_cntr_out = CNTR_TO_OUT;
            manual_mode_2ch_timer = TT_SHOW_OPTIONS;
            resp = resp_change;            
        }

        if (!manual_mode_2ch_timer)
        {
            if (show_option)
            {
                show_option = 0;
		SCREEN_Clear();
            }
            else
            {
		if (program == CCT1_MODE)
		{
		    DataShow_2Ch (SHOW_TEMP_MODE1,
				  *(ch + 0),
				  *(ch + 1),
				  DUMMY_DEGREE);
		}
		else
		{
		    DataShow_2Ch (SHOW_TEMP_MODE2,
				  *(ch + 0),
				  *(ch + 1),
				  DUMMY_DEGREE);
		}
                show_option = 1;
            }

            if (mm_menu_cntr_out)
                mm_menu_cntr_out--;
            
	    manual_need_display_update = 1;
            manual_mode_2ch_timer = TT_SHOW_OPTIONS;
        }

        if (!mm_menu_cntr_out)
	{
            manual_mode_2ch_state = MM_2CH_INIT;
	    resp = resp_need_to_save;
	}
        break;

    case MM_2CH_SHOW_DEGREE:
	if ((!manual_mode_2ch_timer) && (sw == selection_none))
	{
	    manual_mode_2ch_state = MM_2CH_INIT;
	}	
        break;
	
    default:
        manual_mode_2ch_state = MM_2CH_INIT;
        break;
    }

    if (manual_need_display_update)
        display_update();
    
    return resp;
    
}


void DataShow_2Ch (unsigned char to_show,
		   unsigned char bright,
		   unsigned char temp,
		   unsigned short degree)
{
    char s_temp[10] = { 0 };    //16 chars per line + '\0'    
    unsigned short color = 0;
    unsigned char bright_int = 0;
    unsigned char bright_dec = 0;
    
    switch (to_show)
    {
    case SHOW_TEMP_MODE1:
	SCREEN_Clear ();
	ColorTemp1 (temp, &color);
	sprintf(s_temp, "%dK", color);
	SCREEN_Text4_Line1(s_temp);
	break;

    case SHOW_TEMP_MODE2:
	SCREEN_Clear ();
	ColorTemp2 (temp, &color);
	sprintf(s_temp, "%dK", color);
	SCREEN_Text4_Line1(s_temp);
	break;

    case SHOW_BRIGHT:
	SCREEN_Clear ();
	
	Percentage (bright, &bright_int, &bright_dec);
	sprintf(s_temp, "%2d.%d%%", bright_int, bright_dec);
	
	if (bright_int == 100)
	    SCREEN_Text4_Line1("100\% ");
	else
	    SCREEN_Text4_Line1(s_temp);
	
        break;

    case SHOW_DEGREE:
	SCREEN_Clear ();

	if (Temp_Probe_Present_Get())
	{
	    sprintf(s_temp, "%dC",
		    Temp_TempToDegreesExtended (degree));
	    SCREEN_Text4_Line1(s_temp);
	}
	else
	    SCREEN_Text4_Line1("No Con");

        break;

    default:
	break;
    }
}


//--- end of file ---//
