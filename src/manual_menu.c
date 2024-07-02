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
#include "menu_options_oled.h"

#include <string.h>
#include <stdio.h>


// Module Private Types Constants and Macros -----------------------------------
typedef enum {
    MANUAL_MENU_INIT = 0,
    MANUAL_MENU_SHOW_FIRST,
    MANUAL_MENU_STANDBY,    
    MANUAL_MENU_WAIT_CHANGE_RED,
    MANUAL_MENU_CHANGE_RED,    
    MANUAL_MENU_CHANGING_RED,
    MANUAL_MENU_CHANGE_GREEN,
    MANUAL_MENU_CHANGING_GREEN,
    MANUAL_MENU_CHANGE_BLUE,
    MANUAL_MENU_CHANGING_BLUE,
    MANUAL_MENU_CHANGE_WHITE,    
    MANUAL_MENU_CHANGING_WHITE
    
} manual_menu_state_e;


// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------
volatile unsigned short manual_menu_timer = 0;
unsigned char manual_menu_showing = 0;
unsigned char manual_menu_out_cnt = 0;
manual_menu_state_e manual_state = MANUAL_MENU_INIT;


// Module Private Functions ----------------------------------------------------


// Module Funtions -------------------------------------------------------------
void Manual_Menu_Timeouts (void)
{
    if (manual_menu_timer)
        manual_menu_timer--;
}

void Manual_Menu_Reset (void)
{
    manual_state = MANUAL_MENU_INIT;
}


resp_t Manual_Menu (parameters_typedef * mem, sw_actions_t actions)
{
    unsigned char manual_need_display_update = 0;
    resp_t resp = resp_continue;
    char s_temp[20];

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

    case MANUAL_MENU_SHOW_FIRST:
        if (manual_menu_timer)
            break;
        
        // if (mem->dmx_channel_quantity >= 4)
        // {
        //     sprintf(s_temp, "CH1: %3d     CH4: %3d",
        //             *((mem->fixed_channels) + 0),
        //             *((mem->fixed_channels) + 3));
        // }

        SCREEN_Text2_BlankLine1();
        SCREEN_Text2_BlankLine2();

        sprintf(s_temp, "R%3d  G%3d",
                *((mem->fixed_channels) + 0),
                *((mem->fixed_channels) + 1));
        SCREEN_Text2_Line1(s_temp);

        sprintf(s_temp, "B%3d  W%3d",
                *((mem->fixed_channels) + 2),
                *((mem->fixed_channels) + 3));
        SCREEN_Text2_Line2(s_temp);

        manual_need_display_update = 1;
        resp = resp_change;    //first colors update
        manual_state++;
        break;
        
    case MANUAL_MENU_STANDBY:
        if ((actions == selection_up) ||
            (actions == selection_enter))
        {
            Options_Up_Dwn_Select_Reset();
            manual_menu_out_cnt = 20;
            manual_state++;
        }
        break;

    case MANUAL_MENU_WAIT_CHANGE_RED:
        // wait free
        if ((actions == selection_up) ||
            (actions == selection_enter))
            break;
        
        manual_state++;
        break;
        
    case MANUAL_MENU_CHANGE_RED:
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

        manual_need_display_update = 1;
        manual_state++;
        break;

    case MANUAL_MENU_CHANGING_RED:
        
        resp = Options_Up_Dwn_Next (actions);

        if (resp != resp_continue)
        {
            manual_need_display_update = 1;
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
            manual_state--;
            resp = resp_change;
        }

        if (resp == resp_dwn)
        {
            unsigned char * pch = ((mem->fixed_channels) + 0);            
            // if actions selection_all_up, change fast
            if (*pch > 0)
                *pch -= 1;

            manual_state--;            
            resp = resp_change;
        }

        if (!manual_menu_timer)
        {
            manual_menu_timer = 500;
            if (manual_menu_showing)
                manual_menu_showing = 0;
            else
                manual_menu_showing = 1;

            manual_state--;
            manual_menu_out_cnt--;

            if (!manual_menu_out_cnt)
            {
                manual_state = MANUAL_MENU_SHOW_FIRST;                
            }
        }

        if (resp == resp_ok)
        {
            manual_state++;
            resp = resp_continue;
        }
        break;
        
    case MANUAL_MENU_CHANGE_GREEN:

        SCREEN_Text2_BlankLine1();
        if (manual_menu_showing)
        {
            sprintf(s_temp, "Green: %3d",
                    *((mem->fixed_channels) + 1));
            SCREEN_Text2_Line1(s_temp);
        }
        else
            SCREEN_Text2_Line1("Green:");

        manual_need_display_update = 1;
        manual_state++;
        break;

    case MANUAL_MENU_CHANGING_GREEN:

        resp = Options_Up_Dwn_Next (actions);

        if (resp != resp_continue)
        {
            manual_need_display_update = 1;
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

            manual_state--;
            resp = resp_change;
        }

        if (resp == resp_dwn)
        {
            unsigned char * pch = ((mem->fixed_channels) + 1);
            // if actions selection_all_up, change fast
            if (*pch > 0)
                *pch -= 1;
            
            manual_state--;
            resp = resp_change;
        }

        if (!manual_menu_timer)
        {
            manual_menu_timer = 500;
            if (manual_menu_showing)
                manual_menu_showing = 0;
            else
                manual_menu_showing = 1;

            manual_state--;
            manual_menu_out_cnt--;

            if (!manual_menu_out_cnt)
            {
                manual_state = MANUAL_MENU_SHOW_FIRST;                
            }
        }

        if (resp == resp_ok)
        {
            manual_state++;
            resp = resp_continue;
        }
        break;

    case MANUAL_MENU_CHANGE_BLUE:

        SCREEN_Text2_BlankLine1();
        if (manual_menu_showing)
        {
            sprintf(s_temp, "Blue:  %3d",
                    *((mem->fixed_channels) + 2));
            SCREEN_Text2_Line1(s_temp);
        }
        else
            SCREEN_Text2_Line1("Blue:");

        manual_need_display_update = 1;
        manual_state++;
        break;

    case MANUAL_MENU_CHANGING_BLUE:

        resp = Options_Up_Dwn_Next (actions);

        if (resp != resp_continue)
        {
            manual_need_display_update = 1;
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

            manual_state--;
            resp = resp_change;
        }

        if (resp == resp_dwn)
        {
            unsigned char * pch = ((mem->fixed_channels) + 2);
            // if actions selection_all_up, change fast
            if (*pch > 0)
                *pch -= 1;
            
            manual_state--;
            resp = resp_change;
        }

        if (!manual_menu_timer)
        {
            manual_menu_timer = 500;
            if (manual_menu_showing)
                manual_menu_showing = 0;
            else
                manual_menu_showing = 1;

            manual_state--;
            manual_menu_out_cnt--;

            if (!manual_menu_out_cnt)
            {
                manual_state = MANUAL_MENU_SHOW_FIRST;                
            }
        }

        if (resp == resp_ok)
        {
            manual_state++;
            resp = resp_continue;
        }
        break;

    case MANUAL_MENU_CHANGE_WHITE:

        SCREEN_Text2_BlankLine1();
        if (manual_menu_showing)
        {
            sprintf(s_temp, "White: %3d",
                    *((mem->fixed_channels) + 3));
            SCREEN_Text2_Line1(s_temp);
        }
        else
            SCREEN_Text2_Line1("White:");

        manual_need_display_update = 1;
        manual_state++;
        break;

    case MANUAL_MENU_CHANGING_WHITE:

        resp = Options_Up_Dwn_Next (actions);

        if (resp != resp_continue)
        {
            manual_need_display_update = 1;
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

            manual_state--;
            resp = resp_change;
        }

        if (resp == resp_dwn)
        {
            unsigned char * pch = ((mem->fixed_channels) + 3);
            // if actions selection_all_up, change fast
            if (*pch > 0)
                *pch -= 1;
            
            manual_state--;
            resp = resp_change;
        }

        if (!manual_menu_timer)
        {
            manual_menu_timer = 500;
            if (manual_menu_showing)
                manual_menu_showing = 0;
            else
                manual_menu_showing = 1;

            manual_state--;
            manual_menu_out_cnt--;

            if (!manual_menu_out_cnt)
            {
                manual_state = MANUAL_MENU_SHOW_FIRST;                
            }
        }

        if (resp == resp_ok)
        {
            manual_state = MANUAL_MENU_SHOW_FIRST;
        }
        break;
    }

    if (manual_need_display_update)
        display_update();

    return resp;
    
}


//--- end of file ---//
