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


// Externals -------------------------------------------------------------------
extern volatile unsigned short adc_ch [];


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
        
        if (mem->manual_inner_mode == 2)
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
            manual_state = MANUAL_MENU_INIT;
        
        break;

    case MANUAL_MENU_INNER_SKIPPING:
        resp = Manual_Menu_Skipping (mem, actions, &manual_need_display_update);

        // change in mode        
        if (resp == resp_ok)
            manual_state = MANUAL_MENU_INIT;
        
        break;

    case MANUAL_MENU_INNER_FIXED:
        resp = Manual_Menu_Fixed_Colors (mem, actions, &manual_need_display_update);

        // change in mode        
        if (resp == resp_ok)
            manual_state = MANUAL_MENU_INIT;

        break;
        
    }

    if (manual_need_display_update)
        display_update();

    return resp;
    
}


// -- Inner Modes on Manual Menu --
typedef enum {
    FIXED_SHOW_FIRST = 0,
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
    FIXED_CHANGE_MODE,
    FIXED_CHANGING_MODE
    
} inner_state_fixed_e;


inner_state_fixed_e inner_state = 0;
void Manual_Menu_Inner_Reset (void)
{
    inner_state = 0;
}


resp_t Manual_Menu_Fixed_Colors (parameters_typedef * mem,
                                 sw_actions_t actions,
                                 unsigned char * need_display_update)
{
    resp_t resp = resp_continue;
    char s_temp[20];

    switch (inner_state)
    {
    case FIXED_SHOW_FIRST:
        if (manual_menu_timer)
            break;
        
        SCREEN_Text2_BlankLine1();
        SCREEN_Text2_BlankLine2();

        sprintf(s_temp, "R%3d  G%3d",
                *((mem->fixed_channels) + 0),
                *((mem->fixed_channels) + 1));
        SCREEN_Text2_Line1(s_temp);

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
        SCREEN_Text2_Line2(s_temp);            

        *need_display_update = 1;
        resp = resp_change;    //first colors update
        inner_state++;
        break;
        
    case FIXED_STANDBY:
        // for temp
        if (actions == selection_enter)
        {
            Options_Up_Dwn_Select_Reset();
            manual_menu_out_cnt = 20;
            inner_state++;
        }

        if (actions == selection_up)
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
        // end for temp
        break;

    case FIXED_WAIT_CHANGE_RED:
        // wait free
        if ((actions == selection_up) ||
            (actions == selection_enter))
            break;

        Check_S2_Accel_Fast();
        inner_state++;
        break;
        
    case FIXED_CHANGE_RED:
        // wait free
        // if ((actions == selection_up) ||
        //     (actions == selection_enter))
        //     break;
        
        SCREEN_Text2_BlankLine1();
        if (manual_menu_showing)
        {
            sprintf(s_temp, "Red:   %3d",
                    *((mem->fixed_channels) + 0));
            SCREEN_Text2_Line1(s_temp);            
        }
        else
            SCREEN_Text2_Line1("Red:");

        *need_display_update = 1;
        inner_state++;
        break;

    case FIXED_CHANGING_RED:
        
        resp = Options_Up_Dwn_Next (actions);

        if (resp != resp_continue)
        {
            *need_display_update = 1;
            manual_menu_out_cnt = 20;
            manual_menu_timer = 500;
            manual_menu_showing = 1;
        }
        
        if (resp == resp_up)
        {
            unsigned char * pch = ((mem->fixed_channels) + 0);
            // if actions selection_all_up, change fast
            if (*pch < 255)
                *pch += 1;

            // // force high velocity update
            // Options_Up_Dwn_Next (selection_none);
            inner_state--;
            resp = resp_need_to_save;
        }

        if (resp == resp_dwn)
        {
            unsigned char * pch = ((mem->fixed_channels) + 0);            
            // if actions selection_all_up, change fast
            if (*pch > 0)
                *pch -= 1;

            inner_state--;            
            resp = resp_need_to_save;
        }

        if (!manual_menu_timer)
        {
            manual_menu_timer = 500;
            if (manual_menu_showing)
                manual_menu_showing = 0;
            else
                manual_menu_showing = 1;

            inner_state--;
            manual_menu_out_cnt--;

            if (!manual_menu_out_cnt)
            {
                Check_S2_Accel_Slow();
                inner_state = FIXED_SHOW_FIRST;                
            }
        }

        if (resp == resp_ok)
        {
            inner_state++;
            resp = resp_continue;
        }
        break;
        
    case FIXED_CHANGE_GREEN:

        SCREEN_Text2_BlankLine1();
        if (manual_menu_showing)
        {
            sprintf(s_temp, "Green: %3d",
                    *((mem->fixed_channels) + 1));
            SCREEN_Text2_Line1(s_temp);
        }
        else
            SCREEN_Text2_Line1("Green:");

        *need_display_update = 1;
        inner_state++;
        break;

    case FIXED_CHANGING_GREEN:

        resp = Options_Up_Dwn_Next (actions);

        if (resp != resp_continue)
        {
            *need_display_update = 1;
            manual_menu_out_cnt = 20;
            manual_menu_timer = 500;
            manual_menu_showing = 1;
        }

        if (resp == resp_up)
        {
            unsigned char * pch = ((mem->fixed_channels) + 1);
            // if actions selection_all_up, change fast
            if (*pch < 255)
                *pch += 1;

            inner_state--;
            resp = resp_need_to_save;
        }

        if (resp == resp_dwn)
        {
            unsigned char * pch = ((mem->fixed_channels) + 1);
            // if actions selection_all_up, change fast
            if (*pch > 0)
                *pch -= 1;
            
            inner_state--;
            resp = resp_need_to_save;
        }

        if (!manual_menu_timer)
        {
            manual_menu_timer = 500;
            if (manual_menu_showing)
                manual_menu_showing = 0;
            else
                manual_menu_showing = 1;

            inner_state--;
            manual_menu_out_cnt--;

            if (!manual_menu_out_cnt)
            {
                Check_S2_Accel_Slow();
                inner_state = FIXED_SHOW_FIRST;                
            }
        }

        if (resp == resp_ok)
        {
            inner_state++;
            resp = resp_continue;
        }
        break;

    case FIXED_CHANGE_BLUE:

        SCREEN_Text2_BlankLine1();
        if (manual_menu_showing)
        {
            sprintf(s_temp, "Blue:  %3d",
                    *((mem->fixed_channels) + 2));
            SCREEN_Text2_Line1(s_temp);
        }
        else
            SCREEN_Text2_Line1("Blue:");

        *need_display_update = 1;
        inner_state++;
        break;

    case FIXED_CHANGING_BLUE:

        resp = Options_Up_Dwn_Next (actions);

        if (resp != resp_continue)
        {
            *need_display_update = 1;
            manual_menu_out_cnt = 20;
            manual_menu_timer = 500;
            manual_menu_showing = 1;
        }
        
        if (resp == resp_up)
        {
            unsigned char * pch = ((mem->fixed_channels) + 2);
            // if actions selection_all_up, change fast
            if (*pch < 255)
                *pch += 1;

            inner_state--;
            resp = resp_need_to_save;
        }

        if (resp == resp_dwn)
        {
            unsigned char * pch = ((mem->fixed_channels) + 2);
            // if actions selection_all_up, change fast
            if (*pch > 0)
                *pch -= 1;
            
            inner_state--;
            resp = resp_need_to_save;
        }

        if (!manual_menu_timer)
        {
            manual_menu_timer = 500;
            if (manual_menu_showing)
                manual_menu_showing = 0;
            else
                manual_menu_showing = 1;

            inner_state--;
            manual_menu_out_cnt--;

            if (!manual_menu_out_cnt)
            {
                Check_S2_Accel_Slow();
                inner_state = FIXED_SHOW_FIRST;                
            }
        }

        if (resp == resp_ok)
        {
            if (mem->dmx_channel_quantity == 4)
                inner_state++;
            else
            {
                Check_S2_Accel_Slow();
                inner_state = FIXED_CHANGE_MODE;

                // save actual inner mode
                manual_menu_last_inner_mode = mem->manual_inner_mode;
            }
            resp = resp_continue;            
        }
        break;

    case FIXED_CHANGE_WHITE:

        SCREEN_Text2_BlankLine1();
        if (manual_menu_showing)
        {
            sprintf(s_temp, "White: %3d",
                    *((mem->fixed_channels) + 3));
            SCREEN_Text2_Line1(s_temp);
        }
        else
            SCREEN_Text2_Line1("White:");

        *need_display_update = 1;
        inner_state++;
        break;

    case FIXED_CHANGING_WHITE:

        resp = Options_Up_Dwn_Next (actions);

        if (resp != resp_continue)
        {
            *need_display_update = 1;
            manual_menu_out_cnt = 20;
            manual_menu_timer = 500;
            manual_menu_showing = 1;
        }
        
        if (resp == resp_up)
        {
            unsigned char * pch = ((mem->fixed_channels) + 3);
            // if actions selection_all_up, change fast
            if (*pch < 255)
                *pch += 1;

            inner_state--;
            resp = resp_need_to_save;
        }

        if (resp == resp_dwn)
        {
            unsigned char * pch = ((mem->fixed_channels) + 3);
            // if actions selection_all_up, change fast
            if (*pch > 0)
                *pch -= 1;
            
            inner_state--;
            resp = resp_need_to_save;
        }

        if (!manual_menu_timer)
        {
            manual_menu_timer = 500;
            if (manual_menu_showing)
                manual_menu_showing = 0;
            else
                manual_menu_showing = 1;

            inner_state--;
            manual_menu_out_cnt--;

            if (!manual_menu_out_cnt)
            {
                Check_S2_Accel_Slow();
                inner_state = FIXED_SHOW_FIRST;
            }
        }

        if (resp == resp_ok)
        {
            Check_S2_Accel_Slow();
            inner_state++;
            resp = resp_continue;

            // save actual inner mode
            manual_menu_last_inner_mode = mem->manual_inner_mode;
        }
        break;

    case FIXED_CHANGE_MODE:

        SCREEN_Text2_BlankLine1();
        if (manual_menu_showing)
        {
            switch (mem->manual_inner_mode)
            {
            case 0:
                SCREEN_Text2_Line1("Fixed");
                break;
            case 1:
                SCREEN_Text2_Line1("Skipping");
                break;                
            case 2:
                SCREEN_Text2_Line1("Fading");
                break;
            }
        }
        else
            // SCREEN_Text2_Line1("Manual Mod");
            SCREEN_Text2_Line1("    Colors");        

        *need_display_update = 1;
        inner_state++;
        break;

    case FIXED_CHANGING_MODE:
        
        resp = Options_Up_Dwn_Out (actions);

        if (resp != resp_continue)
        {
            *need_display_update = 1;
            manual_menu_out_cnt = 20;
            manual_menu_timer = 500;
            manual_menu_showing = 1;
        }
        
        if ((resp == resp_up) ||
            (resp == resp_dwn))
        {
            unsigned char * pch = &(mem->manual_inner_mode);
            if (*pch < 2)
                *pch += 1;
            else
                *pch = 0;

            inner_state--;
            resp = resp_need_to_save;
        }

        if (!manual_menu_timer)
        {
            manual_menu_timer = 500;
            if (manual_menu_showing)
                manual_menu_showing = 0;
            else
                manual_menu_showing = 1;

            inner_state--;
            manual_menu_out_cnt--;

            if (!manual_menu_out_cnt)
            {
                Check_S2_Accel_Slow();
                inner_state = FIXED_SHOW_FIRST;
                if (manual_menu_last_inner_mode != mem->manual_inner_mode)
                    resp = resp_ok;

                break;
            }
        }

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
    FADING_STANDBY,
    FADING_WAIT_CHANGE_SPEED,
    FADING_CHANGE_SPEED,
    FADING_CHANGING_SPEED,
    FADING_CHANGE_MODE,
    FADING_CHANGING_MODE
    
} inner_state_fading_e;

resp_t Manual_Menu_Fading (parameters_typedef * mem,
                           sw_actions_t actions,
                           unsigned char * need_display_update)
{
    resp_t resp = resp_continue;
    char s_temp[20];

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

        *need_display_update = 1;
        resp = resp_change;    //first colors update
        inner_state++;
        break;
        
    case FADING_STANDBY:
        // for temp
        if (actions == selection_enter)
        {
            Options_Up_Dwn_Select_Reset();
            manual_menu_out_cnt = 20;
            inner_state++;
        }

        if (actions == selection_up)
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
        // end for temp
        break;

    case FADING_WAIT_CHANGE_SPEED:
        // wait free
        if ((actions == selection_up) ||
            (actions == selection_enter))
            break;

        // Check_S2_Accel_Fast();
        inner_state++;
        break;
        
    case FADING_CHANGE_SPEED:
        SCREEN_Text2_BlankLine1();
        if (manual_menu_showing)
        {
            sprintf(s_temp, "Speed: %d", mem->manual_inner_speed);
            SCREEN_Text2_Line1(s_temp);            
        }
        else
            SCREEN_Text2_Line1("Speed:");

        *need_display_update = 1;
        inner_state++;
        break;

    case FADING_CHANGING_SPEED:
        
        resp = Options_Up_Dwn_Select (actions);

        if (resp != resp_continue)
        {
            *need_display_update = 1;
            manual_menu_out_cnt = 20;
            manual_menu_timer = 500;
            manual_menu_showing = 1;
        }
        
        if (resp == resp_up)
        {
            unsigned char * pch = &(mem->manual_inner_speed);
            // if actions selection_all_up, change fast
            if (*pch < 9)
                *pch += 1;

            // // force high velocity update
            // Options_Up_Dwn_Next (selection_none);
            inner_state--;
            resp = resp_need_to_save;
        }

        if (resp == resp_dwn)
        {
            unsigned char * pch = &(mem->manual_inner_speed);
            // if actions selection_all_up, change fast
            if (*pch > 0)
                *pch -= 1;

            inner_state--;            
            resp = resp_need_to_save;
        }

        if (!manual_menu_timer)
        {
            manual_menu_timer = 500;
            if (manual_menu_showing)
                manual_menu_showing = 0;
            else
                manual_menu_showing = 1;

            inner_state--;
            manual_menu_out_cnt--;

            if (!manual_menu_out_cnt)
            {
                // Check_S2_Accel_Slow();
                inner_state = FADING_SHOW_FIRST;
            }
        }

        if (resp == resp_ok)
        {
            inner_state++;
            resp = resp_continue;

            // save actual inner mode
            manual_menu_last_inner_mode = mem->manual_inner_mode;
        }
        break;

    case FADING_CHANGE_MODE:

        SCREEN_Text2_BlankLine1();
        if (manual_menu_showing)
        {
            switch (mem->manual_inner_mode)
            {
            case 0:
                SCREEN_Text2_Line1("Fixed");
                break;
            case 1:
                SCREEN_Text2_Line1("Skipping");
                break;                
            case 2:
                SCREEN_Text2_Line1("Fading");
                break;
            }
        }
        else
            // SCREEN_Text2_Line1("Manual Mod");
            SCREEN_Text2_Line1("    Colors");        

        *need_display_update = 1;
        inner_state++;
        break;

    case FADING_CHANGING_MODE:
        
        resp = Options_Up_Dwn_Out (actions);

        if (resp != resp_continue)
        {
            *need_display_update = 1;
            manual_menu_out_cnt = 20;
            manual_menu_timer = 500;
            manual_menu_showing = 1;
        }
        
        if ((resp == resp_up) ||
            (resp == resp_dwn))
        {
            unsigned char * pch = &(mem->manual_inner_mode);
            if (*pch < 2)
                *pch += 1;
            else
                *pch = 0;

            inner_state--;
            resp = resp_need_to_save;
        }

        if (!manual_menu_timer)
        {
            manual_menu_timer = 500;
            if (manual_menu_showing)
                manual_menu_showing = 0;
            else
                manual_menu_showing = 1;

            inner_state--;
            manual_menu_out_cnt--;

            if (!manual_menu_out_cnt)
            {
                // Check_S2_Accel_Slow();
                inner_state = FADING_SHOW_FIRST;
                if (manual_menu_last_inner_mode != mem->manual_inner_mode)
                    resp = resp_ok;

                break;
            }
        }

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
            resp = Colors_Fading_Shuffle_Pallete (mem->fixed_channels);
            manual_effect_timer = 10 - mem->manual_inner_speed;
            resp = resp_change;
        }
    }
    
    return resp;
}


typedef enum {
    SKIPPING_SHOW_FIRST = 0,
    SKIPPING_STANDBY,
    SKIPPING_WAIT_CHANGE_SPEED,
    SKIPPING_CHANGE_SPEED,
    SKIPPING_CHANGING_SPEED,
    SKIPPING_CHANGE_MODE,
    SKIPPING_CHANGING_MODE
    
} inner_state_skipping_e;

resp_t Manual_Menu_Skipping (parameters_typedef * mem,
                             sw_actions_t actions,
                             unsigned char * need_display_update)
{
    resp_t resp = resp_continue;
    char s_temp[20];

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

        *need_display_update = 1;
        resp = resp_change;    //first colors update
        inner_state++;
        break;
        
    case SKIPPING_STANDBY:
        // for temp
        if (actions == selection_enter)
        {
            Options_Up_Dwn_Select_Reset();
            manual_menu_out_cnt = 20;
            inner_state++;
        }

        if (actions == selection_up)
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
        // end for temp
        break;

    case SKIPPING_WAIT_CHANGE_SPEED:
        // wait free
        if ((actions == selection_up) ||
            (actions == selection_enter))
            break;

        // Check_S2_Accel_Fast();
        inner_state++;
        break;
        
    case SKIPPING_CHANGE_SPEED:
        SCREEN_Text2_BlankLine1();
        if (manual_menu_showing)
        {
            sprintf(s_temp, "Speed: %d", mem->manual_inner_speed);
            SCREEN_Text2_Line1(s_temp);            
        }
        else
            SCREEN_Text2_Line1("Speed:");

        *need_display_update = 1;
        inner_state++;
        break;

    case SKIPPING_CHANGING_SPEED:
        
        resp = Options_Up_Dwn_Select (actions);

        if (resp != resp_continue)
        {
            *need_display_update = 1;
            manual_menu_out_cnt = 20;
            manual_menu_timer = 500;
            manual_menu_showing = 1;
        }
        
        if (resp == resp_up)
        {
            unsigned char * pch = &(mem->manual_inner_speed);
            // if actions selection_all_up, change fast
            if (*pch < 9)
                *pch += 1;

            // // force high velocity update
            // Options_Up_Dwn_Next (selection_none);
            inner_state--;
            resp = resp_need_to_save;
        }

        if (resp == resp_dwn)
        {
            unsigned char * pch = &(mem->manual_inner_speed);
            // if actions selection_all_up, change fast
            if (*pch > 0)
                *pch -= 1;

            inner_state--;            
            resp = resp_need_to_save;
        }

        if (!manual_menu_timer)
        {
            manual_menu_timer = 500;
            if (manual_menu_showing)
                manual_menu_showing = 0;
            else
                manual_menu_showing = 1;

            inner_state--;
            manual_menu_out_cnt--;

            if (!manual_menu_out_cnt)
            {
                // Check_S2_Accel_Slow();
                inner_state = SKIPPING_SHOW_FIRST;
            }
        }

        if (resp == resp_ok)
        {
            inner_state++;
            resp = resp_continue;

            // save actual inner mode
            manual_menu_last_inner_mode = mem->manual_inner_mode;            
        }
        break;

    case SKIPPING_CHANGE_MODE:

        SCREEN_Text2_BlankLine1();
        if (manual_menu_showing)
        {
            switch (mem->manual_inner_mode)
            {
            case 0:
                SCREEN_Text2_Line1("Fixed");
                break;
            case 1:
                SCREEN_Text2_Line1("Skipping");
                break;                
            case 2:
                SCREEN_Text2_Line1("Fading");
                break;
            }
        }
        else
            // SCREEN_Text2_Line1("Manual Mod");
            SCREEN_Text2_Line1("    Colors");        

        *need_display_update = 1;
        inner_state++;
        break;

    case SKIPPING_CHANGING_MODE:
        
        resp = Options_Up_Dwn_Out (actions);

        if (resp != resp_continue)
        {
            *need_display_update = 1;
            manual_menu_out_cnt = 20;
            manual_menu_timer = 500;
            manual_menu_showing = 1;
        }
        
        if ((resp == resp_up) ||
            (resp == resp_dwn))
        {
            unsigned char * pch = &(mem->manual_inner_mode);
            if (*pch < 2)
                *pch += 1;
            else
                *pch = 0;

            inner_state--;
            resp = resp_need_to_save;
        }

        if (!manual_menu_timer)
        {
            manual_menu_timer = 500;
            if (manual_menu_showing)
                manual_menu_showing = 0;
            else
                manual_menu_showing = 1;

            inner_state--;
            manual_menu_out_cnt--;

            if (!manual_menu_out_cnt)
            {
                // Check_S2_Accel_Slow();
                inner_state = SKIPPING_SHOW_FIRST;
                if (manual_menu_last_inner_mode != mem->manual_inner_mode)
                    resp = resp_ok;

                break;
            }
        }

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
            resp = Colors_Strobe_Pallete (mem->fixed_channels);
            manual_effect_timer = 1000 - mem->manual_inner_speed * 100;
            resp = resp_change;
        }                
    }
    
    return resp;    
}

//--- end of file ---//
