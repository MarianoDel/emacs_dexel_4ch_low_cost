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
    MANUAL_MENU_CHANGE_RED,
    MANUAL_MENU_CHANGING_RED,
    MANUAL_MENU_CHANGE_GREEN,
    MANUAL_MENU_CHANGING_GREEN,
    MANUAL_MENU_CHANGE_BLUE,
    MANUAL_MENU_CHANGING_BLUE,
    MANUAL_MENU_CHANGE_WHITE,    
    MANUAL_MENU_CHANGING_WHITE
    
} manual_menu_state_e;


#define TT_SHOW    500
#define TT_NOT_SHOW    500


// Externals -------------------------------------------------------------------



// Globals ---------------------------------------------------------------------
volatile unsigned short manual_menu_timer = 0;
manual_menu_state_e manual_state = MANUAL_MENU_INIT;


// Module Private Functions ----------------------------------------------------
void Manual_Selected_To_Line_Init (unsigned char, unsigned char *, unsigned char *, unsigned char *);
void ManualMenu_Options(unsigned char, unsigned char, char *);
resp_t ManualMenu_CheckColors (parameters_typedef * mem);


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
            // manual_need_display_update = 1;
            manual_state++;
        }
        break;

    case MANUAL_MENU_CHANGE_RED:

        SCREEN_Text2_BlankLine1();
        sprintf(s_temp, "Red:   %3d",
                *((mem->fixed_channels) + 0));
        SCREEN_Text2_Line1(s_temp);

        manual_need_display_update = 1;
        manual_state++;
        break;

    case MANUAL_MENU_CHANGING_RED:
        
        resp = Options_Up_Dwn_Next (actions);

        if (resp != resp_continue)
            manual_need_display_update = 1;
        
        if (resp == resp_up)
        {
            unsigned char * pch = ((mem->fixed_channels) + 0);
            // if actions selection_all_up, change fast
            if (*pch < 255)
                *pch += 1;

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

        if (resp == resp_ok)
        {
            manual_state++;
            resp = resp_continue;
        }
        break;
        
    case MANUAL_MENU_CHANGE_GREEN:

        SCREEN_Text2_BlankLine1();
        sprintf(s_temp, "Green: %3d",
                *((mem->fixed_channels) + 1));
        SCREEN_Text2_Line1(s_temp);

        manual_need_display_update = 1;
        manual_state++;
        break;

    case MANUAL_MENU_CHANGING_GREEN:

        resp = Options_Up_Dwn_Next (actions);

        if (resp != resp_continue)
            manual_need_display_update = 1;
        
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

        if (resp == resp_ok)
        {
            manual_state++;
            resp = resp_continue;
        }
        break;

    case MANUAL_MENU_CHANGE_BLUE:

        SCREEN_Text2_BlankLine1();
        sprintf(s_temp, "Blue:  %3d",
                *((mem->fixed_channels) + 2));
        SCREEN_Text2_Line1(s_temp);

        manual_need_display_update = 1;
        manual_state++;
        break;

    case MANUAL_MENU_CHANGING_BLUE:

        resp = Options_Up_Dwn_Next (actions);

        if (resp != resp_continue)
            manual_need_display_update = 1;
        
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

        if (resp == resp_ok)
        {
            manual_state++;
            resp = resp_continue;
        }
        break;

    case MANUAL_MENU_CHANGE_WHITE:

        SCREEN_Text2_BlankLine1();
        sprintf(s_temp, "White: %3d",
                *((mem->fixed_channels) + 3));
        SCREEN_Text2_Line1(s_temp);

        manual_need_display_update = 1;
        manual_state++;
        break;

    case MANUAL_MENU_CHANGING_WHITE:

        resp = Options_Up_Dwn_Next (actions);

        if (resp != resp_continue)
            manual_need_display_update = 1;
        
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




// unsigned char manual_colors_from_comms = 0;
// void ManualMenu_SetColors (unsigned char color)
// {
//     manual_colors_from_comms |= color;
// }


// resp_t ManualMenu_CheckColors (parameters_typedef * mem)
// {
//     resp_t resp = resp_continue;
    
//     if (manual_colors_from_comms)
//     {
//         //check if channel is active
//         unsigned char ch = manual_colors_from_comms;

//         for (unsigned char i = 0; i < 6; i++)
//         {
//             if (ch & 0x01)
//             {
//                 if (i < mem->dmx_channel_quantity)
//                 {
//                     //change the colors and update interface
//                     mem->manual_channels[0] = 0;
//                     mem->manual_channels[1] = 0;
//                     mem->manual_channels[2] = 0;
//                     mem->manual_channels[3] = 0;
//                     mem->manual_channels[4] = 0;
//                     mem->manual_channels[5] = 0;
                    
//                     mem->manual_channels[i] = 255;
//                     i = 6;
//                     resp = resp_change;
//                 }
//             }
//             else
//                 ch >>= 1;

//         }
//         manual_colors_from_comms = 0;
//     }

//     return resp;
// }


//--- end of file ---//
