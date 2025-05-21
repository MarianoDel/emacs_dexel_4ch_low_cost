//-------------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### MANUAL_MENU.C #############################
//-------------------------------------------------

// Includes --------------------------------------------------------------------
#include "manual_menu.h"
#include "screen.h"
#include "ssd1306_display.h"
#include "menu_options_oled.h"
#include "hard.h"

#include "temperatures.h"
#include "adc.h"

#include "colors_functions.h"
#include "manager.h"

#include <string.h>
#include <stdio.h>


// Module Private Types Constants and Macros -----------------------------------
typedef enum {
    MANUAL_MENU_INIT = 0,
    MANUAL_MENU_SELECT_INNER,
    MANUAL_MENU_WAIT_INNER,    
    MANUAL_MENU_INNER_FADING,
    MANUAL_MENU_INNER_SKIPPING,    
    MANUAL_MENU_INNER_FIXED
    
} manual_menu_state_e;


#define CHANGE_OPT_TT    500
#define CHANGE_MOD_TT    800

#define OPT_CNT_NEXT    3

// Externals -------------------------------------------------------------------
extern volatile unsigned short adc_ch [];
extern unsigned char dmx_local_data [];


// Globals ---------------------------------------------------------------------
volatile unsigned short manual_menu_timer = 0;
volatile unsigned short manual_effect_timer = 0;
unsigned char manual_menu_showing = 0;
unsigned char manual_menu_out_cnt = 0;
unsigned char manual_menu_last_inner_mode = 0;
manual_menu_state_e manual_state = MANUAL_MENU_INIT;



// Module Private Functions ----------------------------------------------------
void Manual_Menu_Inner_Reset (void);

resp_t Manual_Menu_Fixed_Colors (parameters_typedef * mem,
                                 sw_actions_t actions,
                                 unsigned char * need_display_update);

resp_t Manual_Menu_Fading (parameters_typedef * mem,
                            sw_actions_t actions,
                            unsigned char * need_display_update);

resp_t Manual_Menu_Skipping (parameters_typedef * mem,
                             sw_actions_t actions,
                             unsigned char * need_display_update);

void Manual_Menu_Change_Inner_Mode_Reset (void);

resp_t Manual_Menu_Change_Inner_Mode (parameters_typedef * mem,
                                      sw_actions_t actions,
                                      unsigned char * need_display_update);

// Module Funtions -------------------------------------------------------------
void Manual_Menu_Timeouts (void)
{
    if (manual_menu_timer)
        manual_menu_timer--;
    
    if (manual_effect_timer)
        manual_effect_timer--;
}

void Manual_Menu_Reset (void)
{
    manual_state = MANUAL_MENU_INIT;
}


resp_t Manual_Menu (parameters_typedef * mem, sw_actions_t actions)
{
    unsigned char manual_need_display_update = 0;
    resp_t resp = resp_continue;
    
    switch (manual_state)
    {
    case MANUAL_MENU_INIT:
        SCREEN_Text2_BlankLine1();
        SCREEN_Text2_BlankLine2();
        SCREEN_Text2_Line1("  Manual  ");
        SCREEN_Text2_Line2("   Mode   ");        
        manual_menu_timer = 800;
        manual_need_display_update = 1;
        manual_state++;
        break;

    case MANUAL_MENU_SELECT_INNER:
        if (manual_menu_timer)
            break;

        SCREEN_Text2_BlankLine1();
        SCREEN_Text2_BlankLine2();

	if (mem->dmx_channel_quantity == 1)
	{
            SCREEN_Text2_Line1("  Fixed   ");
            SCREEN_Text2_Line2("  Colors  ");
	    mem->manual_inner_mode = 0;
	}
        else if (mem->manual_inner_mode == 2)
        {
            SCREEN_Text2_Line1("  Fading  ");
            SCREEN_Text2_Line2("  Colors  ");            
        }
        else if (mem->manual_inner_mode == 1)
        {
            SCREEN_Text2_Line1(" Skipping ");
            SCREEN_Text2_Line2("  Colors  ");
        }
        else
        {
            SCREEN_Text2_Line1("  Fixed   ");
            SCREEN_Text2_Line2("  Colors  ");
        }
        
        manual_menu_timer = 800;
        manual_need_display_update = 1;
        manual_state++;
        break;

    case MANUAL_MENU_WAIT_INNER:
        if (manual_menu_timer)
            break;

        if (mem->manual_inner_mode == 2)
            manual_state = MANUAL_MENU_INNER_FADING;
        else if (mem->manual_inner_mode == 1)
            manual_state = MANUAL_MENU_INNER_SKIPPING;
        else
            manual_state = MANUAL_MENU_INNER_FIXED;

        Manual_Menu_Inner_Reset ();
        break;
        
    case MANUAL_MENU_INNER_FADING:
        resp = Manual_Menu_Fading (mem, actions, &manual_need_display_update);

        // change in mode        
        if (resp == resp_ok)
        {
            manual_state = MANUAL_MENU_INIT;
            resp = resp_need_to_save;
        }
        
        break;

    case MANUAL_MENU_INNER_SKIPPING:
        resp = Manual_Menu_Skipping (mem, actions, &manual_need_display_update);

        // change in mode        
        if (resp == resp_ok)
        {
            manual_state = MANUAL_MENU_INIT;
            resp = resp_need_to_save;
        }
        
        break;

    case MANUAL_MENU_INNER_FIXED:
        resp = Manual_Menu_Fixed_Colors (mem, actions, &manual_need_display_update);

        // change in mode        
        if (resp == resp_ok)
        {
            manual_state = MANUAL_MENU_INIT;
            resp = resp_need_to_save;
        }

        break;
        
    }

    if (manual_need_display_update)
        display_update();

    return resp;
    
}


// -- Inner Modes on Manual Menu --
typedef enum {
    FIXED_SHOW_FIRST = 0,
    FIXED_WAIT_INPUTS,
    FIXED_STANDBY,    
    FIXED_WAIT_CHANGE_RED,
    FIXED_CHANGE_RED,
    FIXED_CHANGING_RED,
    FIXED_CHANGE_GREEN,
    FIXED_CHANGING_GREEN,
    FIXED_CHANGE_BLUE,
    FIXED_CHANGING_BLUE,
    FIXED_CHANGE_WHITE,
    FIXED_CHANGING_WHITE,
    FIXED_CHANGE_MODE
    
} inner_state_fixed_e;


inner_state_fixed_e inner_state = 0;
void Manual_Menu_Inner_Reset (void)
{
    inner_state = 0;
}

sw_actions_t manual_menu_last_action = selection_none;
resp_t Manual_Menu_Fixed_Colors (parameters_typedef * mem,
                                 sw_actions_t actions,
                                 unsigned char * need_display_update)
{
    resp_t resp = resp_continue;
    char s_temp[20];
    unsigned char * pch;

    switch (inner_state)
    {
    case FIXED_SHOW_FIRST:
        if (manual_menu_timer)
            break;
        
        SCREEN_Text2_BlankLine1();
        SCREEN_Text2_BlankLine2();

	if (mem->dmx_channel_quantity == 1)
	{
	    sprintf(s_temp, "DIM   %3d ",
		    *((mem->fixed_channels) + 0));
	    SCREEN_Text2_Line1(s_temp);
	}
	else if (mem->dmx_channel_quantity == 3)
	{
	    sprintf(s_temp, "R%3d  G%3d",
		    *((mem->fixed_channels) + 0),
		    *((mem->fixed_channels) + 1));
	    SCREEN_Text2_Line1(s_temp);

            sprintf(s_temp, "B%3d",
                    *((mem->fixed_channels) + 2));
	    SCREEN_Text2_Line2(s_temp);
	}
	else    // 4 channels
	{
	    sprintf(s_temp, "R%3d  G%3d",
		    *((mem->fixed_channels) + 0),
		    *((mem->fixed_channels) + 1));
	    SCREEN_Text2_Line1(s_temp);

            sprintf(s_temp, "B%3d  W%3d",
                    *((mem->fixed_channels) + 2),
                    *((mem->fixed_channels) + 3));
	    SCREEN_Text2_Line2(s_temp);	    
	}

        for (int i = 0; i < 4; i++)
            dmx_local_data[i] = mem->fixed_channels[i];
        
        *need_display_update = 1;
        resp = resp_change;    //first colors update
        inner_state++;
        break;

    case FIXED_WAIT_INPUTS:
        if (actions != selection_none)
        {
            manual_menu_last_action = actions;
            manual_menu_timer = 200;
            inner_state++;
        }
        break;
        
    case FIXED_STANDBY:
        if (manual_menu_timer)
            break;
        
        // change colors, out by timer
        if ((actions == selection_dwn) &&
            (manual_menu_last_action == selection_dwn))
        {
            manual_menu_out_cnt = OPT_CNT_NEXT;
            inner_state++;
        }

        // change mode
        if ((actions == selection_up) &&
            (manual_menu_last_action == selection_up))
        {
            // save actual inner mode
            manual_menu_last_inner_mode = mem->manual_inner_mode;
            Manual_Menu_Change_Inner_Mode_Reset();
            inner_state = FIXED_CHANGE_MODE;
        }

        // show current temp
        if ((actions == selection_enter) ||
            ((actions == selection_dwn) && (manual_menu_last_action == selection_up)) ||
            ((actions == selection_up) && (manual_menu_last_action == selection_dwn)))
        {
            char s_temp [20];
            SCREEN_Text2_BlankLine1();
            SCREEN_Text2_BlankLine2();
            if (Manager_Probe_Temp_Get())
            {
                sprintf(s_temp, "Temp: %dC", Temp_TempToDegreesExtended (Temp_Channel));
                SCREEN_Text2_Line1(s_temp);
            }
            else
                SCREEN_Text2_Line1("Temp: NC");
            
            manual_menu_timer = 1200;
            inner_state = FIXED_SHOW_FIRST;
        }
        break;

    case FIXED_WAIT_CHANGE_RED:
        // wait free
        if (actions != selection_none)
            break;

        Check_S1_Accel_Fast();
        Check_S2_Accel_Fast();        
        inner_state++;
        break;
        
    case FIXED_CHANGE_RED:
        SCREEN_Text2_BlankLine1();
	if (mem->dmx_channel_quantity == 1)
	{
	    if (manual_menu_showing)
	    {
		sprintf(s_temp, "DIM   %3d ",
			*((mem->fixed_channels) + 0));
	    }
	    else
	    {
		strcpy(s_temp, "DIM       ");
	    }
	}
	else
	{
	    if (manual_menu_showing)
	    {
		sprintf(s_temp, "R%3d  G%3d",
			*((mem->fixed_channels) + 0),
			*((mem->fixed_channels) + 1));
	    }
	    else
	    {
		sprintf(s_temp, "R     G%3d",
			*((mem->fixed_channels) + 1));
	    }
	}
        SCREEN_Text2_Line1(s_temp);

        *need_display_update = 1;
        inner_state++;
        break;

    case FIXED_CHANGING_RED:
        
        pch = ((mem->fixed_channels) + 0);

        if ((actions == selection_up) ||
            (actions == selection_dwn))
        {
            if (actions == selection_up)
            {
                if (*pch < 255)
                    *pch += 1;

                // Check_S1_Accel_Fast();                
            }
            else
            {
                if (*pch > 0)
                    *pch -= 1;

                // Check_S2_Accel_Fast();
            }

            dmx_local_data[0] = mem->fixed_channels[0];
            inner_state--;
            manual_menu_timer = CHANGE_OPT_TT;
            manual_menu_out_cnt = OPT_CNT_NEXT;
            manual_menu_showing = 1;
            resp = resp_change;
        }

        if (!manual_menu_timer)
        {
            manual_menu_timer = CHANGE_OPT_TT;

            if (manual_menu_showing)
            {
                if (manual_menu_out_cnt)
                {
                    inner_state--;
                    manual_menu_out_cnt--;
                    manual_menu_showing = 0;
                }
                else
                {
                    if (mem->dmx_channel_quantity != 1)
                    {
                        manual_menu_out_cnt = OPT_CNT_NEXT;
                        inner_state++;
                    }
                    else
                    {
                        Check_S1_Accel_Slow();
                        Check_S2_Accel_Slow();
                        inner_state = FIXED_SHOW_FIRST;
                        resp = resp_need_to_save;
                    }
                }
            }
            else
            {
                manual_menu_showing = 1;
                inner_state--;
            }
        }
        break;
        
    case FIXED_CHANGE_GREEN:
        SCREEN_Text2_BlankLine1();
        if (manual_menu_showing)
        {
            sprintf(s_temp, "R%3d  G%3d",
                    *((mem->fixed_channels) + 0),
                    *((mem->fixed_channels) + 1));
        }
        else
        {
            sprintf(s_temp, "R%3d  G   ",
                    *((mem->fixed_channels) + 0));
        }
        SCREEN_Text2_Line1(s_temp);

        *need_display_update = 1;
        inner_state++;
        break;

    case FIXED_CHANGING_GREEN:
        pch = ((mem->fixed_channels) + 1);

        if ((actions == selection_up) ||
            (actions == selection_dwn))
        {
            if (actions == selection_up)
            {
                if (*pch < 255)
                    *pch += 1;

                // Check_S1_Accel_Fast();                
            }
            else
            {
                if (*pch > 0)
                    *pch -= 1;

                // Check_S2_Accel_Fast();
            }

            dmx_local_data[1] = mem->fixed_channels[1];
            inner_state--;
            manual_menu_timer = CHANGE_OPT_TT;
            manual_menu_out_cnt = OPT_CNT_NEXT;            
            manual_menu_showing = 1;
            resp = resp_change;
        }

        if (!manual_menu_timer)
        {
            manual_menu_timer = CHANGE_OPT_TT;
            if (manual_menu_showing)
            {
                if (manual_menu_out_cnt)
                {
                    inner_state--;
                    manual_menu_out_cnt--;
                    manual_menu_showing = 0;
                }
                else
                {
                    manual_menu_out_cnt = OPT_CNT_NEXT;
                    inner_state++;
                }
            }
            else
            {
                manual_menu_showing = 1;
                inner_state--;
            }
        }
        break;

    case FIXED_CHANGE_BLUE:

        SCREEN_Text2_BlankLine2();
        if (manual_menu_showing)
        {
            if (mem->dmx_channel_quantity == 4)
            {
                sprintf(s_temp, "B%3d  W%3d",
                        *((mem->fixed_channels) + 2),
                        *((mem->fixed_channels) + 3));
            }
            else
            {
                sprintf(s_temp, "B%3d",
                        *((mem->fixed_channels) + 2));
            }
        }
        else
        {
            if (mem->dmx_channel_quantity == 4)
            {
                sprintf(s_temp, "B     W%3d",
                        *((mem->fixed_channels) + 3));
            }
            else
            {
                sprintf(s_temp, "B");
            }
        }
        SCREEN_Text2_Line2(s_temp);            
        
        *need_display_update = 1;
        inner_state++;
        break;

    case FIXED_CHANGING_BLUE:
        pch = ((mem->fixed_channels) + 2);

        if ((actions == selection_up) ||
            (actions == selection_dwn))
        {
            if (actions == selection_up)
            {
                if (*pch < 255)
                    *pch += 1;

                // Check_S1_Accel_Fast();                
            }
            else
            {
                if (*pch > 0)
                    *pch -= 1;

                // Check_S2_Accel_Fast();
            }

            dmx_local_data[2] = mem->fixed_channels[2];
            inner_state--;
            manual_menu_timer = CHANGE_OPT_TT;
            manual_menu_out_cnt = OPT_CNT_NEXT;            
            manual_menu_showing = 1;
            resp = resp_change;
        }

        if (!manual_menu_timer)
        {
            manual_menu_timer = CHANGE_OPT_TT;

            if (manual_menu_showing)
            {
                if (manual_menu_out_cnt)
                {
                    inner_state--;
                    manual_menu_out_cnt--;
                    manual_menu_showing = 0;
                }
                else
                {
                    if (mem->dmx_channel_quantity == 4)
                    {
                        manual_menu_out_cnt = OPT_CNT_NEXT;
                        inner_state++;
                    }
                    else
                    {
                        Check_S1_Accel_Slow();                    
                        Check_S2_Accel_Slow();
                        inner_state = FIXED_SHOW_FIRST;
                        resp = resp_need_to_save;
                    }
                }
            }
            else
            {
                manual_menu_showing = 1;
                inner_state--;
            }
        }
        break;

    case FIXED_CHANGE_WHITE:
        
        SCREEN_Text2_BlankLine2();
        if (manual_menu_showing)
        {
            sprintf(s_temp, "B%3d  W%3d",
                    *((mem->fixed_channels) + 2),
                    *((mem->fixed_channels) + 3));
        }
        else
        {
            sprintf(s_temp, "B%3d  W   ",
                    *((mem->fixed_channels) + 2));
        }
        SCREEN_Text2_Line2(s_temp);            

        *need_display_update = 1;
        inner_state++;
        break;

    case FIXED_CHANGING_WHITE:
        pch = ((mem->fixed_channels) + 3);

        if ((actions == selection_up) ||
            (actions == selection_dwn))
        {
            if (actions == selection_up)
            {
                if (*pch < 255)
                    *pch += 1;

                // Check_S1_Accel_Fast();                
            }
            else
            {
                if (*pch > 0)
                    *pch -= 1;

                // Check_S2_Accel_Fast();
            }

            dmx_local_data[3] = mem->fixed_channels[3];
            inner_state--;
            manual_menu_timer = CHANGE_OPT_TT;
            manual_menu_out_cnt = OPT_CNT_NEXT;            
            manual_menu_showing = 1;
            resp = resp_change;
        }

        if (!manual_menu_timer)
        {
            manual_menu_timer = CHANGE_OPT_TT;
            if (manual_menu_showing)
            {
                if (manual_menu_out_cnt)
                {
                    inner_state--;
                    manual_menu_out_cnt--;
                    manual_menu_showing = 0;
                }
                else
                {
                    Check_S1_Accel_Slow();                    
                    Check_S2_Accel_Slow();
                    inner_state = FIXED_SHOW_FIRST;
                    resp = resp_need_to_save;                
                }
            }
            else
            {
                manual_menu_showing = 1;
                inner_state--;
            }
        }
        break;

    case FIXED_CHANGE_MODE:
        resp = Manual_Menu_Change_Inner_Mode (mem, actions, need_display_update);

        if (resp == resp_ok)
        {
            inner_state = FIXED_SHOW_FIRST;
            if (manual_menu_last_inner_mode != mem->manual_inner_mode)
                resp = resp_ok;
            else
                resp = resp_continue;
            
        }
        break;

    default:
        inner_state = FIXED_SHOW_FIRST;
        resp = resp_continue;
        break;
        
    }
    
    return resp;
}


typedef enum {
    FADING_SHOW_FIRST = 0,
    FADING_WAIT_INPUTS,
    FADING_STANDBY,
    FADING_CHANGE_SPEED,
    FADING_CHANGE_WAIT_FREE,    
    FADING_CHANGING_SPEED,
    FADING_CHANGE_MODE
    
} inner_state_fading_e;

resp_t Manual_Menu_Fading (parameters_typedef * mem,
                           sw_actions_t actions,
                           unsigned char * need_display_update)
{
    resp_t resp = resp_continue;
    char s_temp[20];
    unsigned char * pch;

    switch (inner_state)
    {
    case FADING_SHOW_FIRST:
        if (manual_menu_timer)
            break;
        
        SCREEN_Text2_BlankLine1();
        SCREEN_Text2_BlankLine2();

        SCREEN_Text2_Line1("Fading    ");
        
        sprintf(s_temp, "Speed: %d", mem->manual_inner_speed);
        SCREEN_Text2_Line2(s_temp);            

        Check_S1_Accel_Slow();
        Check_S2_Accel_Slow();        
        
        *need_display_update = 1;
        resp = resp_change;    //first colors update
        inner_state++;
        break;

    case FADING_WAIT_INPUTS:
        if (actions != selection_none)
        {
            manual_menu_last_action = actions;
            // SCREEN_Text2_BlankLine1();
            // SCREEN_Text2_BlankLine2();            
            // *need_display_update = 1;
            manual_menu_timer = 200;
            inner_state++;
        }
        break;
        
    case FADING_STANDBY:
        if (manual_menu_timer)
            break;

        // change speed, out by timer
        if ((actions == selection_dwn) &&
            (manual_menu_last_action == selection_dwn))
        {
            manual_menu_out_cnt = OPT_CNT_NEXT;
            inner_state++;
        }

        // change mode
        if ((actions == selection_up) &&
            (manual_menu_last_action == selection_up))
        {
            // save actual inner mode
            manual_menu_last_inner_mode = mem->manual_inner_mode;
            Manual_Menu_Change_Inner_Mode_Reset();
            inner_state = FADING_CHANGE_MODE;
        }

        // show current temp
        if ((actions == selection_enter) ||
            ((actions == selection_dwn) && (manual_menu_last_action == selection_up)) ||
            ((actions == selection_up) && (manual_menu_last_action == selection_dwn)))
        {
            char s_temp [20];
            SCREEN_Text2_BlankLine1();
            SCREEN_Text2_BlankLine2();
            if (Manager_Probe_Temp_Get())
            {
                sprintf(s_temp, "Temp: %dC", Temp_TempToDegreesExtended (Temp_Channel));
                SCREEN_Text2_Line1(s_temp);
            }
            else
                SCREEN_Text2_Line1("Temp: NC");
            
            manual_menu_timer = 1200;
            inner_state = FADING_SHOW_FIRST;
        }
        break;

    case FADING_CHANGE_SPEED:
        SCREEN_Text2_BlankLine2();
        if (manual_menu_showing)
        {
            sprintf(s_temp, "Speed: %d", mem->manual_inner_speed);
            SCREEN_Text2_Line2(s_temp);            
        }
        else
            SCREEN_Text2_Line2("Speed:");

        *need_display_update = 1;
        inner_state++;
        break;

    case FADING_CHANGE_WAIT_FREE:
        if (actions == selection_none)
            inner_state++;
        
        break;
                
    case FADING_CHANGING_SPEED:
        pch = &(mem->manual_inner_speed);
        
        if ((actions == selection_up) ||
            (actions == selection_dwn))
        {
            if (actions == selection_up)
            {
                if (*pch < 9)
                    *pch += 1;

            }
            else
            {
                if (*pch > 0)
                    *pch -= 1;

            }

            inner_state = FADING_CHANGE_SPEED;
            manual_menu_timer = CHANGE_OPT_TT;
            manual_menu_out_cnt = OPT_CNT_NEXT;
            manual_menu_showing = 1;            
        }

        if (!manual_menu_timer)
        {
            manual_menu_timer = CHANGE_OPT_TT;
            if (manual_menu_showing)
            {
                if (manual_menu_out_cnt)
                {
                    inner_state = FADING_CHANGE_SPEED;
                    manual_menu_out_cnt--;
                    manual_menu_showing = 0;
                }
                else
                {
                    // Check_S1_Accel_Slow();                    
                    // Check_S2_Accel_Slow();
                    inner_state = FADING_SHOW_FIRST;
                    resp = resp_need_to_save;                
                }
            }
            else
            {
                manual_menu_showing = 1;
                inner_state = FADING_CHANGE_SPEED;
            }
        }
        break;

    case FADING_CHANGE_MODE:
        resp = Manual_Menu_Change_Inner_Mode (mem, actions, need_display_update);

        if (resp == resp_ok)
        {
            inner_state = FADING_SHOW_FIRST;
            if (manual_menu_last_inner_mode != mem->manual_inner_mode)
                resp = resp_ok;
            else
                resp = resp_continue;
            
        }
        break;
        
    default:
        inner_state = FADING_SHOW_FIRST;
        resp = resp_continue;
        break;
    }

    // always do the effect, if we are not doing anything else
    if (resp == resp_continue)
    {
        if (!manual_effect_timer)
        {
            // resp = Colors_Fading_Shuffle_Pallete (mem->fixed_channels);
            resp = Colors_Fading_Shuffle_Pallete (dmx_local_data);            
            manual_effect_timer = 10 - mem->manual_inner_speed;
            resp = resp_change;
        }
    }
    
    return resp;
}


typedef enum {
    SKIPPING_SHOW_FIRST = 0,
    SKIPPING_WAIT_INPUTS,
    SKIPPING_STANDBY,
    SKIPPING_CHANGE_SPEED,
    SKIPPING_CHANGE_WAIT_FREE,    
    SKIPPING_CHANGING_SPEED,
    SKIPPING_CHANGE_MODE,
    
} inner_state_skipping_e;

resp_t Manual_Menu_Skipping (parameters_typedef * mem,
                             sw_actions_t actions,
                             unsigned char * need_display_update)
{
    resp_t resp = resp_continue;
    char s_temp[20];
    unsigned char * pch;
    
    switch (inner_state)
    {
    case SKIPPING_SHOW_FIRST:
        if (manual_menu_timer)
            break;
        
        SCREEN_Text2_BlankLine1();
        SCREEN_Text2_BlankLine2();

        SCREEN_Text2_Line1("Skipping  ");
        
        sprintf(s_temp, "Speed: %d", mem->manual_inner_speed);
        SCREEN_Text2_Line2(s_temp);            

        Check_S1_Accel_Slow();
        Check_S2_Accel_Slow();        
        
        *need_display_update = 1;
        resp = resp_change;    //first colors update
        inner_state++;
        break;

    case SKIPPING_WAIT_INPUTS:
        if (actions != selection_none)
        {
            manual_menu_last_action = actions;
            manual_menu_timer = 200;
            inner_state++;
        }
        break;
        
    case SKIPPING_STANDBY:
        if (manual_menu_timer)
            break;

        // change speed, out by timer
        if ((actions == selection_dwn) &&
            (manual_menu_last_action == selection_dwn))
        {
            manual_menu_out_cnt = OPT_CNT_NEXT;
            inner_state++;
        }

        // change mode
        if ((actions == selection_up) &&
            (manual_menu_last_action == selection_up))
        {
            // save actual inner mode
            manual_menu_last_inner_mode = mem->manual_inner_mode;
            Manual_Menu_Change_Inner_Mode_Reset();
            inner_state = SKIPPING_CHANGE_MODE;
        }

        // show current temp
        if ((actions == selection_enter) ||
            ((actions == selection_dwn) && (manual_menu_last_action == selection_up)) ||
            ((actions == selection_up) && (manual_menu_last_action == selection_dwn)))
        {
            char s_temp [20];
            SCREEN_Text2_BlankLine1();
            SCREEN_Text2_BlankLine2();
            if (Manager_Probe_Temp_Get())
            {
                sprintf(s_temp, "Temp: %dC", Temp_TempToDegreesExtended (Temp_Channel));
                SCREEN_Text2_Line1(s_temp);
            }
            else
                SCREEN_Text2_Line1("Temp: NC");
            
            manual_menu_timer = 1200;
            inner_state = SKIPPING_SHOW_FIRST;
        }
        break;

    case SKIPPING_CHANGE_SPEED:
        SCREEN_Text2_BlankLine2();
        if (manual_menu_showing)
        {
            sprintf(s_temp, "Speed: %d", mem->manual_inner_speed);
            SCREEN_Text2_Line2(s_temp);            
        }
        else
            SCREEN_Text2_Line2("Speed:");

        *need_display_update = 1;
        inner_state++;
        break;

    case SKIPPING_CHANGE_WAIT_FREE:
        if (actions == selection_none)
            inner_state++;
        
        break;
                
    case SKIPPING_CHANGING_SPEED:
        pch = &(mem->manual_inner_speed);
        
        if ((actions == selection_up) ||
            (actions == selection_dwn))
        {
            if (actions == selection_up)
            {
                if (*pch < 9)
                    *pch += 1;

            }
            else
            {
                if (*pch > 0)
                    *pch -= 1;

            }

            inner_state = SKIPPING_CHANGE_SPEED;
            manual_menu_timer = CHANGE_OPT_TT;
            manual_menu_out_cnt = OPT_CNT_NEXT;
            manual_menu_showing = 1;            
        }

        if (!manual_menu_timer)
        {
            manual_menu_timer = CHANGE_OPT_TT;
            if (manual_menu_showing)
            {
                if (manual_menu_out_cnt)
                {
                    inner_state = SKIPPING_CHANGE_SPEED;
                    manual_menu_out_cnt--;
                    manual_menu_showing = 0;
                }
                else
                {
                    // Check_S1_Accel_Slow();                    
                    // Check_S2_Accel_Slow();
                    inner_state = SKIPPING_SHOW_FIRST;
                    resp = resp_need_to_save;                
                }
            }
            else
            {
                manual_menu_showing = 1;
                inner_state = SKIPPING_CHANGE_SPEED;
            }
        }
        break;

    case SKIPPING_CHANGE_MODE:
        resp = Manual_Menu_Change_Inner_Mode (mem, actions, need_display_update);

        if (resp == resp_ok)
        {
            inner_state = SKIPPING_SHOW_FIRST;
            if (manual_menu_last_inner_mode != mem->manual_inner_mode)
                resp = resp_ok;
            else
                resp = resp_continue;
            
        }
        break;        

    default:
        inner_state = SKIPPING_SHOW_FIRST;
        resp = resp_continue;
        break;
    }

    // always showing if no other changes needed
    if (resp == resp_continue)
    {
        if (!manual_effect_timer)
        {
            // resp = Colors_Strobe_Pallete (mem->fixed_channels);
            resp = Colors_Strobe_Pallete (dmx_local_data);            
            manual_effect_timer = 1000 - mem->manual_inner_speed * 100;
            resp = resp_change;
        }                
    }
    
    return resp;    
}


typedef enum {
    CHANGE_INNER_INIT = 0,
    CHANGE_INNER_CHANGE,
    CHANGE_INNER_CHANGE_WAIT_FREE,
    CHANGE_INNER_CHANGING
    
} change_inner_e;


change_inner_e change_inner_state = CHANGE_INNER_INIT;
void Manual_Menu_Change_Inner_Mode_Reset (void)
{
    change_inner_state = CHANGE_INNER_INIT;
}


resp_t Manual_Menu_Change_Inner_Mode (parameters_typedef * mem,
                                      sw_actions_t actions,
                                      unsigned char * need_display_update)
{
    resp_t resp = resp_continue;
    unsigned char * pch;
    
    switch (change_inner_state)
    {
    case CHANGE_INNER_INIT:
        manual_menu_out_cnt = OPT_CNT_NEXT;
        manual_menu_timer = CHANGE_MOD_TT;
        manual_menu_showing = 1;
        change_inner_state++;
        break;

    case CHANGE_INNER_CHANGE:
        SCREEN_Text2_BlankLine1();
        SCREEN_Text2_BlankLine2();
        
        if (manual_menu_showing)
        {
            switch (mem->manual_inner_mode)
            {
            case 0:
                SCREEN_Text2_Line1("Fixed");
                SCREEN_Text2_Line2("    Colors");
                break;
            case 1:
                SCREEN_Text2_Line1("Skipping");
                SCREEN_Text2_Line2("    Colors");                
                break;                
            case 2:
                SCREEN_Text2_Line1("Fading");
                SCREEN_Text2_Line2("    Colors");                
                break;
            }
        }

        *need_display_update = 1;
        change_inner_state++;
        break;

    case CHANGE_INNER_CHANGE_WAIT_FREE:
        if (actions == selection_none)
            change_inner_state++;
        
        break;
        
    case CHANGE_INNER_CHANGING:
        pch = &(mem->manual_inner_mode);

        if ((actions == selection_up) ||
            (actions == selection_dwn))
        {
            if (*pch < 2)
                *pch += 1;
            else
                *pch = 0;

            manual_menu_out_cnt = OPT_CNT_NEXT;
            manual_menu_timer = CHANGE_MOD_TT;
            manual_menu_showing = 1;
            change_inner_state = CHANGE_INNER_CHANGE;
        }

        if (!manual_menu_timer)
        {
            manual_menu_timer = CHANGE_OPT_TT;

            if (manual_menu_showing)
            {
                if (manual_menu_out_cnt)
                {
                    change_inner_state = CHANGE_INNER_CHANGE;
                    manual_menu_out_cnt--;
                    manual_menu_showing = 0;
                }
                else
                {
                    change_inner_state = CHANGE_INNER_INIT;
                    resp = resp_ok;
                }
            }
            else
            {
                manual_menu_showing = 1;
                change_inner_state = CHANGE_INNER_CHANGE;
            }
        }
        break;

    default:
        change_inner_state = CHANGE_INNER_INIT;
        break;
    }

    return resp;    
}
//--- end of file ---//
