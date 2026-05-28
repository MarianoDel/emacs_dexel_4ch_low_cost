//-------------------------------------------------
// #### DEXEL 2CH PROJECT - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### MANUAL_MODE.C #############################
//-------------------------------------------------

// Includes --------------------------------------------------------------------
#include "manual_mode.h"
#include "lcd_utils.h"
#include "dmx_utils.h"
#include "parameters.h"
#include "usart.h"

#include "temperatures.h"

#include <stdio.h>
#include <string.h>


// Private Types Constants and Macros ------------------------------------------
typedef enum {
    MANUAL_MODE_INIT = 0,
    MANUAL_MODE_RUNNING

} manual_mode_e;


#define TT_MENU_TIMEOUT    30000


// variables re-use
#define manual_mode_state    mode_state
#define mm_menu_timer    mode_effect_timer

#define SHOW_ALL    0
#define SHOW_ONLY_TEMP    1
#define SHOW_ONLY_BRIGHT    2

#define TT_SHOW_OPTIONS    500
#define CNTR_TO_OUT    16

// Externals -------------------------------------------------------------------
// -- externals re-used
extern unsigned char mode_state;
extern volatile unsigned short mode_effect_timer;

#define mm_menu_timer    mode_effect_timer

extern parameters_typedef mem_conf;

// Globals ---------------------------------------------------------------------
volatile unsigned short mm_serial_timer = 0;
volatile unsigned short mm_serial_need_to_save_timer = 0;
unsigned char mm_serial_need_to_save_flag = 0;


// Module Private Functions ----------------------------------------------------
void ManualMode_MenuReset (void);
resp_t ManualMode_Menu (unsigned char *, sw_actions_t);
void DataShow (unsigned char state, unsigned char bright, unsigned char temp, unsigned char ch_mode);
unsigned char ManualMode_CheckSerial (void);


// Module Functions ------------------------------------------------------------
void ManualMode_UpdateTimers (void)
{
    if (mm_menu_timer)
        mm_menu_timer--;

    if (mm_serial_timer)
        mm_serial_timer--;

    if (mm_serial_need_to_save_timer)
        mm_serial_need_to_save_timer--;
    
}


void ManualModeReset (void)
{
    manual_mode_state = MANUAL_MODE_INIT;
}



resp_t ManualMode (unsigned char * ch_val, sw_actions_t action)
{
    resp_t resp = resp_continue;
    
    switch (manual_mode_state)
    {
    case MANUAL_MODE_INIT:
        ManualMode_MenuReset();
        manual_mode_state++;
        break;

    case MANUAL_MODE_RUNNING:
        resp = ManualMode_Menu(ch_val, action);

        break;

    default:
        manual_mode_state = MANUAL_MODE_INIT;
        break;
    }

    
    if (resp == resp_finish)
    {
        //end of changing ask for a memory save
        resp = resp_need_to_save;
        
    }
    else
    {
        unsigned char input_type = ManualMode_CheckSerial();
        switch (input_type)
        {
        case 0:    // do nothing here
            break;

        case 1:    // low color selected
            if (!mm_serial_timer)
            {
                *(ch_val + 0) = 255;
                *(ch_val + 1) = 0;            
            
                DataShow (SHOW_ALL,
                          *(ch_val + 0),
                          *(ch_val + 1),
                          mem_conf.channels_operation_mode);

                mm_serial_timer = 300;
                mm_serial_need_to_save_flag = 1;
                mm_serial_need_to_save_timer = 5000;
                resp = resp_change;
            }
            break;

        case 2:    // middle color selected
            if (!mm_serial_timer)
            {
                *(ch_val + 0) = 255;
                if (mem_conf.channels_operation_mode == CCT1_MODE)
                    *(ch_val + 1) = MIDDLE_COLOR_VALUE_TEMP1;

                if (mem_conf.channels_operation_mode == CCT2_MODE)
                    *(ch_val + 1) = MIDDLE_COLOR_VALUE_TEMP2;
                
                DataShow (SHOW_ALL,
                          *(ch_val + 0),
                          *(ch_val + 1),
                          mem_conf.channels_operation_mode);

                mm_serial_timer = 300;
                mm_serial_need_to_save_flag = 1;
                mm_serial_need_to_save_timer = 5000;                
                resp = resp_change;
            }
            break;

        case 3:    // high color selected
            if (!mm_serial_timer)
            {
                *(ch_val + 0) = 255;
                *(ch_val + 1) = 255;
            
                DataShow (SHOW_ALL,
                          *(ch_val + 0),
                          *(ch_val + 1),
                          mem_conf.channels_operation_mode);

                mm_serial_timer = 300;
                mm_serial_need_to_save_flag = 1;
                mm_serial_need_to_save_timer = 5000;                
                resp = resp_change;
            }
            break;
        }
    }

    if (mm_serial_need_to_save_flag)
    {
        if (!mm_serial_need_to_save_timer)
        {
            //end of serial colors change ask for a memory save
            mm_serial_need_to_save_flag = 0;
            resp = resp_need_to_save;
        }
    }
    
    return resp;
            
}


typedef enum {
    MM_MENU_INIT = 0,
    MM_MENU_CHECK_SWITCHES,
    MM_MENU_SELECT_BRIGHTNESS_START,
    MM_MENU_SELECT_BRIGHTNESS,
    MM_MENU_SELECT_TEMP_START,
    MM_MENU_SELECT_TEMP,
    MM_MENU_CLEAN_OUT
    
} manual_mode_menu_state_e;


manual_mode_menu_state_e mm_menu_state;
void ManualMode_MenuReset (void)
{
    mm_menu_state = MM_MENU_INIT;
}


unsigned char mm_menu_cntr_out = 0;
unsigned char show_option = 0;
resp_t ManualMode_Menu (unsigned char * ch, sw_actions_t sw)
{
    resp_t resp = resp_continue;

    switch (mm_menu_state)
    {
    case MM_MENU_INIT:
        // LCD_Writel1("  Manual Mode   ");

        DataShow (SHOW_ALL,
                  *(ch + 0),
                  *(ch + 1),
                  mem_conf.channels_operation_mode);
        
        mm_menu_state++;
        break;

    case MM_MENU_CHECK_SWITCHES:
        if (sw == selection_enter)
        {
            DataShow (SHOW_ONLY_TEMP,
                      *(ch + 0),
                      *(ch + 1),
                      mem_conf.channels_operation_mode);
            
            mm_menu_state++;
        }

	// update for current temp on 5 secs
	else if (!mm_menu_timer)
	{
	    mm_menu_timer = 10000;
	    DataShow (SHOW_ALL,
		      *(ch + 0),
		      *(ch + 1),
		      mem_conf.channels_operation_mode);
	}
        break;

    case MM_MENU_SELECT_BRIGHTNESS_START:
        if (sw == selection_none)
        {
            show_option = 0;
            mm_menu_timer = TT_SHOW_OPTIONS;
            mm_menu_cntr_out = CNTR_TO_OUT;
            mm_menu_state++;
        }
        break;

    case MM_MENU_SELECT_BRIGHTNESS:
        if ((sw == selection_up) ||
            (sw == selection_up_fast))
        {
            if (sw == selection_up_fast)
            {
                if (*(ch + 0) < (255 - 10))
                    *(ch + 0) += 10;
                else
                    *(ch + 0) = 255;
            }
            else
            {
                if (*(ch + 0) < 255)
                    *(ch + 0) += 1;
            }

            show_option = 1;
            DataShow (SHOW_ALL,
                      *(ch + 0),
                      *(ch + 1),
                      mem_conf.channels_operation_mode);
            
            mm_menu_timer = TT_SHOW_OPTIONS;

            resp = resp_change;
        }

        if ((sw == selection_dwn) ||
            (sw == selection_dwn_fast))
        {
            if (sw == selection_dwn_fast)
            {
                if (*(ch + 0) > 10)
                    *(ch + 0) -= 10;
                else
                    *(ch + 0) = 0;
            }
            else
            {
                if (*(ch + 0) > 0)
                    *(ch + 0) -= 1;
            }

            show_option = 1;
            DataShow (SHOW_ALL,
                      *(ch + 0),
                      *(ch + 1),
                      mem_conf.channels_operation_mode);

            mm_menu_timer = TT_SHOW_OPTIONS;

            resp = resp_change;            
        }

        if (sw == selection_enter)
        {
            DataShow (SHOW_ONLY_BRIGHT,
                      *(ch + 0),
                      *(ch + 1),
                      mem_conf.channels_operation_mode);

            mm_menu_state = MM_MENU_SELECT_TEMP_START;
        }

        if (!mm_menu_timer)
        {
            if (show_option)
            {
                show_option = 0;
                DataShow (SHOW_ONLY_TEMP,
                          *(ch + 0),
                          *(ch + 1),
                          mem_conf.channels_operation_mode);
            }
            else
            {
                show_option = 1;
                DataShow (SHOW_ALL,
                          *(ch + 0),
                          *(ch + 1),
                          mem_conf.channels_operation_mode);
            }

            if (mm_menu_cntr_out)
                mm_menu_cntr_out--;
            
            mm_menu_timer = TT_SHOW_OPTIONS;
        }

        if (!mm_menu_cntr_out)
            mm_menu_state = MM_MENU_CLEAN_OUT;

        break;

    case MM_MENU_SELECT_TEMP_START:
        if (sw == selection_none)
        {
            show_option = 0;
            mm_menu_timer = TT_SHOW_OPTIONS;
            mm_menu_cntr_out = CNTR_TO_OUT;
            mm_menu_state++;
        }
        break;
        
    case MM_MENU_SELECT_TEMP:
        if ((sw == selection_up) ||
            (sw == selection_up_fast))
        {
            if (sw == selection_up_fast)
            {
                if (*(ch + 1) < (255 - 10))
                    *(ch + 1) += 10;
                else
                    *(ch + 1) = 255;
            }
            else
            {
                if (*(ch + 1) < 255)
                    *(ch + 1) += 1;
            }

            show_option = 1;
            DataShow (SHOW_ALL,
                      *(ch + 0),
                      *(ch + 1),
                      mem_conf.channels_operation_mode);
            
            mm_menu_timer = TT_SHOW_OPTIONS;

            resp = resp_change;            
        }

        if ((sw == selection_dwn) ||
            (sw == selection_dwn_fast))
        {
            if (sw == selection_dwn_fast)
            {
                if (*(ch + 1) > 10)
                    *(ch + 1) -= 10;
                else
                    *(ch + 1) = 0;
            }
            else
            {
                if (*(ch + 1) > 0)
                    *(ch + 1) -= 1;
            }

            show_option = 1;
            DataShow (SHOW_ALL,
                      *(ch + 0),
                      *(ch + 1),
                      mem_conf.channels_operation_mode);
            
            mm_menu_timer = TT_SHOW_OPTIONS;

            resp = resp_change;            
        }

        if (sw == selection_enter)
        {
            mm_menu_state = MM_MENU_CLEAN_OUT;
        }

        if (!mm_menu_timer)
        {
            if (show_option)
            {
                show_option = 0;
                DataShow (SHOW_ONLY_BRIGHT,
                          *(ch + 0),
                          *(ch + 1),
                          mem_conf.channels_operation_mode);
            }
            else
            {
                show_option = 1;
                DataShow (SHOW_ALL,
                          *(ch + 0),
                          *(ch + 1),
                          mem_conf.channels_operation_mode);
            }

            if (mm_menu_cntr_out)
                mm_menu_cntr_out--;
            
            mm_menu_timer = TT_SHOW_OPTIONS;
        }

        if (!mm_menu_cntr_out)
            mm_menu_state = MM_MENU_CLEAN_OUT;

        break;
        
    case MM_MENU_CLEAN_OUT:
        // show_option = 0;
        mm_menu_state = MM_MENU_INIT;

        resp = resp_finish;
        break;

    default:
        mm_menu_state = MM_MENU_INIT;
        break;
    }

    return resp;
    
}


void DataShow (unsigned char state, unsigned char bright, unsigned char temp, unsigned char ch_mode)
{
    // char s_temp[17] = { 0 };    //16 chars per line + '\0'
    char s_temp[20] = { 0 };    //16 chars per line + '\0'    

    switch (state)
    {
    case SHOW_ALL:
        if (ch_mode == CCT1_MODE)
        {
            unsigned short color = 0;
            ColorTemp1 (temp, &color);
            sprintf(s_temp, "Cct: %dK   Man", color);
            LCD_Writel1(s_temp);

            unsigned char dmx_int = 0;
            unsigned char dmx_dec = 0;
            Percentage (bright, &dmx_int, &dmx_dec);
	    if (Temp_Sensor_Present_Get())
	    {
		sprintf(s_temp, "Dim:%3d.%d%%   %dC",
			dmx_int,
			dmx_dec,
			Temp_TempToDegreesExtended_Last_Filtered ());
	    }
	    else
	    {
		sprintf(s_temp, "Dim:%3d.%d%%      ",
			dmx_int,
			dmx_dec);
	    }
            LCD_Writel2(s_temp);
        }

        if (ch_mode == CCT2_MODE)
        {
            unsigned short color = 0;
            ColorTemp2 (temp, &color);
            sprintf(s_temp, "Cct: %dK   Man", color);
            LCD_Writel1(s_temp);

            unsigned char dmx_int = 0;
            unsigned char dmx_dec = 0;
            Percentage (bright, &dmx_int, &dmx_dec);
	    if (Temp_Sensor_Present_Get())
	    {
		sprintf(s_temp, "Dim:%3d.%d%%   %dC",
			dmx_int,
			dmx_dec,
			Temp_TempToDegreesExtended_Last_Filtered ());
	    }
	    else
	    {
		sprintf(s_temp, "Dim:%3d.%d%%      ",
			dmx_int,
			dmx_dec);
	    }
            LCD_Writel2(s_temp);
        }
        
        if (ch_mode == ONECH_MODE)
        {
            unsigned char dmx_int = 0;
            unsigned char dmx_dec = 0;
            Percentage (bright, &dmx_int, &dmx_dec);
            sprintf(s_temp, "Dim:%3d.%d%%   Man",
                    dmx_int,
                    dmx_dec);
            LCD_Writel1(s_temp);

	    if (Temp_Sensor_Present_Get())
	    {
		sprintf(s_temp, "             %dC",
			Temp_TempToDegreesExtended_Last_Filtered ());
	    }
	    else
	    {
		sprintf(s_temp, "                ");
	    }
            LCD_Writel2(s_temp);
        }
        break;

    case SHOW_ONLY_TEMP:
        if (ch_mode == CCT1_MODE)
        {
            unsigned short color = 0;
            ColorTemp1 (temp, &color);
            sprintf(s_temp, "Cct: %dK   Man", color);
            LCD_Writel1(s_temp);

	    if (Temp_Sensor_Present_Get())
	    {
		sprintf(s_temp, "Dim:         %dC",
			Temp_TempToDegreesExtended_Last_Filtered ());
	    }
	    else
	    {
		sprintf(s_temp, "Dim:            ");
	    }
            LCD_Writel2(s_temp);
        }

        if (ch_mode == CCT2_MODE)
        {
            unsigned short color = 0;
            ColorTemp2 (temp, &color);
            sprintf(s_temp, "Cct: %dK   Man", color);
            LCD_Writel1(s_temp);

	    if (Temp_Sensor_Present_Get())
	    {
		sprintf(s_temp, "Dim:         %dC",
			Temp_TempToDegreesExtended_Last_Filtered ());
	    }
	    else
	    {
		sprintf(s_temp, "Dim:            ");
	    }
            LCD_Writel2(s_temp);
        }
        
        if (ch_mode == ONECH_MODE)
        {
            sprintf(s_temp, "Dim:         Man");
            LCD_Writel1(s_temp);

	    if (Temp_Sensor_Present_Get())
	    {
		sprintf(s_temp, "             %dC",
			Temp_TempToDegreesExtended_Last_Filtered ());
	    }
	    else
	    {
		sprintf(s_temp, "                ");
	    }
            LCD_Writel2(s_temp);
        }
        break;

    case SHOW_ONLY_BRIGHT:
        if ((ch_mode == CCT1_MODE) ||
            (ch_mode == CCT2_MODE))
        {
            sprintf(s_temp, "Cct:         Man");
            LCD_Writel1(s_temp);    

            unsigned char dmx_int = 0;
            unsigned char dmx_dec = 0;
            Percentage (bright, &dmx_int, &dmx_dec);
	    if (Temp_Sensor_Present_Get())
	    {
		sprintf(s_temp, "Dim:%3d.%d%%   %dC",
			dmx_int,
			dmx_dec,
			Temp_TempToDegreesExtended_Last_Filtered ());
	    }
	    else
	    {
		sprintf(s_temp, "Dim:%3d.%d%%      ",
			dmx_int,
			dmx_dec);
	    }
            LCD_Writel2(s_temp);
        }

        if (ch_mode == ONECH_MODE)
        {
            unsigned char dmx_int = 0;
            unsigned char dmx_dec = 0;
            Percentage (bright, &dmx_int, &dmx_dec);
            sprintf(s_temp, "Dim:%3d.%d%%   Man",
                    dmx_int,
                    dmx_dec);
            LCD_Writel1(s_temp);
            
	    if (Temp_Sensor_Present_Get())
	    {
		sprintf(s_temp, "             %dC",
			Temp_TempToDegreesExtended_Last_Filtered ());
	    }
	    else
	    {
		sprintf(s_temp, "                ");
	    }
            LCD_Writel2(s_temp);
        }
        break;
    }
}


// get some of this strings, five times, on 50ms separation
// strcpy(color_send, "red\n");
// strcpy(color_send, "green\n");
// strcpy(color_send, "blue\n");
// strcpy(color_send, "warm\n");
// strcpy(color_send, "cold\n");

unsigned char ManualMode_CheckSerial (void)
{
    if (Usart1HaveData())
    {
        char msg [20] = { 0 };
        Usart1ReadBuffer((unsigned char *) msg, 20);
        Usart1HaveDataReset();

        if (!strncmp(msg, "cold", sizeof("cold") - 1))
            return 1;

        if (!strncmp(msg, "red", sizeof("red") - 1))
            return 2;

        if (!strncmp(msg, "green", sizeof("green") - 1))
            return 3;        
    }

    return 0;
}
//--- end of file ---//
