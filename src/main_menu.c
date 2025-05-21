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
    MAIN_MENU_CHANGE_CHANNELS_QTTY,
    MAIN_MENU_CHANGING_CHANNELS_QTTY,
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


unsigned char main_menu_config_change = 0;
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
        if ((main_menu_timer) || (actions != selection_none))
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
            Check_S2_Accel_Fast();
            unsigned char * pch = &(mem->max_current_channels[0]);
            if (*pch < 255)
                *pch += 1;

            main_menu_state--;
            main_menu_config_change = 1;
        }

        if (resp == resp_dwn)
        {
            Check_S2_Accel_Fast();            
            unsigned char * pch = &(mem->max_current_channels[0]);
            if (*pch > 1)
                *pch -= 1;

            main_menu_state--;
            main_menu_config_change = 1;            
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

                Check_S2_Accel_Slow();
                main_menu_state = MAIN_MENU_INIT;

                if (main_menu_config_change)
                    resp = resp_need_to_save;
                    
            }
        }

        if (resp == resp_ok)
        {
            Check_S2_Accel_Slow();            
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
            Check_S2_Accel_Fast();
            unsigned char * pch = &(mem->temp_prot_deg);
            if (*pch < 100)
                *pch += 1;

            main_menu_state--;
            main_menu_config_change = 1;
        }

        if (resp == resp_dwn)
        {
            Check_S2_Accel_Fast();            
            unsigned char * pch = &(mem->temp_prot_deg);
            if (*pch > 50)
                *pch -= 1;

            main_menu_state--;            
            main_menu_config_change = 1;
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
                Check_S2_Accel_Slow();

                if (main_menu_config_change)
                    resp = resp_need_to_save;
            }
        }

        if (resp == resp_ok)
        {
            Check_S2_Accel_Slow();            
            resp = resp_continue;
#ifdef ONE_CHANNEL_CONF_INIT
            main_menu_state = MAIN_MENU_SHOW_VERSION;
#else
            main_menu_state++;	    
#endif
        }
        break;

    case MAIN_MENU_CHANGE_CHANNELS_QTTY:

        SCREEN_Text2_BlankLine1();
        if (main_menu_showing)
        {
            sprintf(s_temp, "Ch Qtty: %d", mem->dmx_channel_quantity);
            SCREEN_Text2_Line1(s_temp);
        }
        else
            SCREEN_Text2_Line1("Ch Qtty:");

        main_need_display_update = 1;
        main_menu_state++;
        break;

    case MAIN_MENU_CHANGING_CHANNELS_QTTY:
        
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
	    // 1, 3 or 4 channels
            Check_S2_Accel_Fast();
            unsigned char * pch = &(mem->dmx_channel_quantity);
	    if (*pch == 1)
		*pch = 3;
	    else if (*pch < 4)
                *pch += 1;

            main_menu_state--;
            main_menu_config_change = 1;
        }

        if (resp == resp_dwn)
        {
	    // 1, 3 or 4 channels	    
            Check_S2_Accel_Fast();            
            unsigned char * pch = &(mem->dmx_channel_quantity);
            if (*pch == 3)
		*pch = 1;
	    else if (*pch > 3)		
                *pch -= 1;

            main_menu_state--;            
            main_menu_config_change = 1;
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
                Check_S2_Accel_Slow();

                if (main_menu_config_change)
                    resp = resp_need_to_save;
            }
        }

        if (resp == resp_ok)
        {
            Check_S2_Accel_Slow();            
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
        main_menu_state = MAIN_MENU_INIT;
        // Check_S2_Accel_Slow();
        
        if (main_menu_config_change)
        {
            main_menu_config_change = 0;
            resp = resp_need_to_save;
        }
        else
            resp = resp_ok;
        
        break;
    }

    if (main_need_display_update)
        display_update();


    return resp;
    
}

    
//--- end of file ---//
