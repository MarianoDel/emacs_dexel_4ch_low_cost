//-----------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### MAIN_MENU.C #############################
//-----------------------------------------------

// Includes --------------------------------------------------------------------
#include "main_menu.h"
#include "screen.h"
#include "ssd1306_display.h"
#include "menu_options_oled.h"
// #include "temperatures.h"
#include "hard.h"

#include <stdio.h>

// Module Private Types & Macros -----------------------------------------------
typedef enum {
    MAIN_MENU_INIT,
    MAIN_MENU_WAIT_INIT,    
    MAIN_MENU_CHANGE_CURRENT,
    MAIN_MENU_CHANGING_CURRENT,
    MAIN_MENU_CHANGE_TEMP_PROT,
    MAIN_MENU_CHANGING_TEMP_PROT,
    MAIN_MENU_SHOW_VERSION,
    MAIN_MENU_WAIT_SHOW_VERSION

} main_menu_state_t;


// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------
main_menu_state_t main_menu_state = MAIN_MENU_INIT;
volatile unsigned short main_menu_timer = 0;
unsigned char main_menu_showing = 0;
unsigned char main_menu_out_cnt = 0;


// Module Private Functions ----------------------------------------------------


// Module Functions ------------------------------------------------------------
void Main_Menu_Timeouts (void)
{
    if (main_menu_timer)
        main_menu_timer--;
    
}


void Main_Menu_Reset (void)
{
    main_menu_state = MAIN_MENU_INIT;
}


resp_t Main_Menu (parameters_typedef * mem, sw_actions_t actions)
{
    unsigned char main_need_display_update = 0;
    resp_t resp = resp_continue;
    char s_temp[20];

    switch (main_menu_state)
    {
    case MAIN_MENU_INIT:
        SCREEN_Text2_BlankLine1();
        SCREEN_Text2_BlankLine2();
        SCREEN_Text2_Line1(" Hardware ");
        SCREEN_Text2_Line2("  Config  ");        
        main_menu_timer = 800;
        main_need_display_update = 1;
        main_menu_state++;
        break;

    case MAIN_MENU_WAIT_INIT:
        if (main_menu_timer)
            break;
        
        SCREEN_Text2_BlankLine1();
        SCREEN_Text2_BlankLine2();
        main_menu_state++;
        break;

    case MAIN_MENU_CHANGE_CURRENT:
        SCREEN_Text2_BlankLine1();
        if (main_menu_showing)
        {
            sprintf(s_temp, "Curr:  %3d", mem->max_current_channels[0]);
            SCREEN_Text2_Line1(s_temp);
        }
        else
            SCREEN_Text2_Line1("Curr:");

        main_need_display_update = 1;
        main_menu_state++;
        break;
        
    case MAIN_MENU_CHANGING_CURRENT:
        resp = Options_Up_Dwn_Next (actions);

        if (resp != resp_continue)
        {
            main_need_display_update = 1;
            main_menu_out_cnt = 20;
            main_menu_timer = 500;
            main_menu_showing = 1;
        }
        
        if (resp == resp_up)
        {
            unsigned char * pch = &(mem->max_current_channels[0]);
            // if actions selection_all_up, change fast
            if (*pch < 255)
                *pch += 1;

            main_menu_state--;
            // resp = resp_change;
        }

        if (resp == resp_dwn)
        {
            unsigned char * pch = &(mem->max_current_channels[0]);
            // if actions selection_all_up, change fast
            if (*pch > 1)
                *pch -= 1;

            main_menu_state--;            
        }

        if (!main_menu_timer)
        {
            main_menu_timer = 500;
            if (main_menu_showing)
                main_menu_showing = 0;
            else
                main_menu_showing = 1;

            main_menu_state--;
            main_menu_out_cnt--;

            if (!main_menu_out_cnt)
            {
                for (int i = 1; i < 4; i++)
                    mem->max_current_channels[i] = mem->max_current_channels[0];

                main_menu_state = MAIN_MENU_INIT;                
            }
        }

        if (resp == resp_ok)
        {
            for (int i = 1; i < 4; i++)
                mem->max_current_channels[i] = mem->max_current_channels[0];
            
            resp = resp_continue;
            main_menu_state++;
        }
        break;

    case MAIN_MENU_CHANGE_TEMP_PROT:

        SCREEN_Text2_BlankLine1();
        if (main_menu_showing)
        {
            sprintf(s_temp, "Temp:  %3d", mem->temp_prot_deg);
            SCREEN_Text2_Line1(s_temp);
        }
        else
            SCREEN_Text2_Line1("Temp:");

        main_need_display_update = 1;
        main_menu_state++;
        break;

    case MAIN_MENU_CHANGING_TEMP_PROT:
        
        resp = Options_Up_Dwn_Next (actions);

        if (resp != resp_continue)
        {
            main_need_display_update = 1;
            main_menu_out_cnt = 20;
            main_menu_timer = 500;
            main_menu_showing = 1;
        }
        
        if (resp == resp_up)
        {
            unsigned char * pch = &(mem->temp_prot_deg);
            // if actions selection_all_up, change fast
            if (*pch < 100)
                *pch += 1;

            main_menu_state--;
            // resp = resp_change;
        }

        if (resp == resp_dwn)
        {
            unsigned char * pch = &(mem->temp_prot_deg);
            // if actions selection_all_up, change fast
            if (*pch > 30)
                *pch -= 1;

            main_menu_state--;            
            // resp = resp_change;
        }

        if (!main_menu_timer)
        {
            main_menu_timer = 500;
            if (main_menu_showing)
                main_menu_showing = 0;
            else
                main_menu_showing = 1;

            main_menu_state--;
            main_menu_out_cnt--;

            if (!main_menu_out_cnt)
            {
                main_menu_state = MAIN_MENU_INIT;                
            }
        }

        if (resp == resp_ok)
        {
            resp = resp_continue;
            main_menu_state++;
        }
        break;

    case MAIN_MENU_SHOW_VERSION:
        SCREEN_Text2_BlankLine1();
        SCREEN_Text2_BlankLine2();
        SCREEN_Text2_Line1(HARD_GetHardwareVersion());
        SCREEN_Text2_Line2(HARD_GetSoftwareVersion());        
        main_menu_timer = 800;
        main_need_display_update = 1;
        main_menu_state++;
        break;

    case MAIN_MENU_WAIT_SHOW_VERSION:
        if (main_menu_timer)
            break;
        
        SCREEN_Text2_BlankLine1();
        SCREEN_Text2_BlankLine2();

        resp = resp_ok;
        break;
    }

    if (main_need_display_update)
        display_update();


    return resp;
    
}

    
//--- end of file ---//
