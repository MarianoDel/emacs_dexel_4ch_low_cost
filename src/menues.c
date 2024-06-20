//-----------------------------------------------
// #### DEXEL 2CH PROJECT - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### MENUES.C ################################
//-----------------------------------------------

// Includes --------------------------------------------------------------------
#include "menues.h"
#include "lcd_utils.h"
#include "temperatures.h"
#include "adc.h"
#include "hard.h"
#include "comms_power.h"

#include <stdio.h>

// Module Private Types & Macros -----------------------------------------------
typedef enum {
    MENU_INIT = 0,
    MENU_SHOW_OPERATION_MODE,
    MENU_SHOW_MAX_CURRENT,
    MENU_SHOW_TEMP,
    MENU_SHOW_CURRENT_TEMP,
    MENU_SHOW_VERSION,
    MENU_SHOW_END_CONF,
    
    MENU_CONF_OPERATION_MODE,
    MENU_CONF_MAX_CURRENT,
    MENU_CONF_TEMP,
    MENU_CONF_CURRENT_TEMP,
    MENU_CONF_CURRENT_TEMP_1,
    MENU_CONF_VERSION_CONTROL,
    MENU_CONF_VERSION_POWER,
    MENU_END_CONF

} menu_state_t;


typedef enum {
    MENU_ENC_INIT = 0,
    MENU_ENC_TIEMPO_ENCENDIDO

} menu_enc_state_t;


// Externals -------------------------------------------------------------------
extern volatile unsigned short adc_ch [];

// Globals ---------------------------------------------------------------------
menu_state_t menu_state = MENU_INIT;
menu_enc_state_t menu_enc_state = MENU_ENC_INIT;


// Module Private Functions ----------------------------------------------------
sw_actions_t MENU_Check_Actions (void);
unsigned char ConvertCurrentFromMemory (mem_bkp_t * config);
void ConvertCurrentToMemory (mem_bkp_t * config, unsigned char current_int, unsigned char current_dec);


// Module Functions ------------------------------------------------------------
void MENU_Main_Reset (void)
{
    menu_state = MENU_INIT;
}

//funcion de seleccion del menu principal
//devuelve nueva selección o estado anterior
unsigned short current_temp = 0;
resp_t MENU_Main (mem_bkp_t * configurations, sw_actions_t sw_action)
{
    resp_t resp = resp_continue;
    unsigned char onoff = 0;
    unsigned short fchannel = 0;
    char s_temp[17] = { 0 };    //16 chars per line + '\0'
    unsigned char new_current_int = 0;
    unsigned char new_current_dec = 0;

    switch (menu_state)
    {
    case MENU_INIT:
        resp = LCD_ShowBlink ("    Entering    ",
                              "    Main Menu   ",
                              1,
                              BLINK_DIRECT);

        if (resp == resp_finish)
        {
            resp = resp_continue;
            if (sw_action == selection_none)
            {
                LCD_ShowSelectv2Reset();
                menu_state++;
            }
        }
        break;

        // Options Menues        
    case MENU_SHOW_OPERATION_MODE:
        
        resp = LCD_ShowSelectv2((const char *) "Operation mode  ",
                                sw_action);

        if (resp == resp_change)
            menu_state = MENU_SHOW_END_CONF;

        if (resp == resp_change_all_up)
            menu_state = MENU_SHOW_MAX_CURRENT;
        
        if (resp == resp_selected)
        {
            LCD_EncoderChangeReset();
            menu_state = MENU_CONF_OPERATION_MODE;
        }

        break;

    case MENU_SHOW_MAX_CURRENT:
        resp = LCD_ShowSelectv2((const char *) "Set max current ",
                                sw_action);

        if (resp == resp_change)
            menu_state = MENU_SHOW_OPERATION_MODE;

        if (resp == resp_change_all_up)
            menu_state = MENU_SHOW_TEMP;
        
        if (resp == resp_selected)
        {
            LCD_EncoderChangeReset();
            menu_state = MENU_CONF_MAX_CURRENT;
        }

        break;


    case MENU_SHOW_TEMP:
        
        resp = LCD_ShowSelectv2((const char *) "Temperature prot",
                                sw_action);

        if (resp == resp_change)
            menu_state = MENU_SHOW_MAX_CURRENT;

        if (resp == resp_change_all_up)
            menu_state = MENU_SHOW_CURRENT_TEMP;
        
        if (resp == resp_selected)
        {
            LCD_EncoderChangeReset();
            menu_state = MENU_CONF_TEMP;
        }

        break;

    case MENU_SHOW_CURRENT_TEMP:
        
        resp = LCD_ShowSelectv2((const char *) "Current Temp.   ",
                                sw_action);

        if (resp == resp_change)
            menu_state = MENU_SHOW_TEMP;

        if (resp == resp_change_all_up)
            menu_state = MENU_SHOW_VERSION;
        
        if (resp == resp_selected)
        {
            current_temp = Temp_Channel;
            menu_state = MENU_CONF_CURRENT_TEMP;
        }
        break;

    case MENU_SHOW_VERSION:
        
        resp = LCD_ShowSelectv2((const char *) "Current Version ",                                
                                sw_action);

        if (resp == resp_change)
            menu_state = MENU_SHOW_CURRENT_TEMP;

        if (resp == resp_change_all_up)
            menu_state = MENU_SHOW_END_CONF;
        
        if (resp == resp_selected)
        {
            menu_state = MENU_CONF_VERSION_CONTROL;
        }
        break;
        
    case MENU_SHOW_END_CONF:

        resp = LCD_ShowSelectv2((const char *) "Save & Exit     ",
                                sw_action);

        if (resp == resp_change)
            menu_state = MENU_SHOW_VERSION;

        if (resp == resp_change_all_up)
            menu_state = MENU_SHOW_OPERATION_MODE;

        if (resp == resp_selected)
        {
            LCD_EncoderOptionsOnOffReset();
            menu_state = MENU_END_CONF;
        }

        break;
        
        // Configuration Menues
    case MENU_CONF_OPERATION_MODE:

        resp = LCD_EncoderShowSelect3 ("CCT1 mode       ",
                                       "CCT2 mode       ",
                                       "1Ch mode        ",
                                       sw_action,
                                       &onoff);
                                     
        if (resp == resp_selected)
        {
            switch (onoff)
            {
            case 1:
                configurations->channels_operation_mode = CCT1_MODE;
                configurations->dmx_channel_quantity = 2;
                break;
            case 2:
                configurations->channels_operation_mode = CCT2_MODE;
                configurations->dmx_channel_quantity = 2;                
                break;
            case 3:
                configurations->channels_operation_mode = ONECH_MODE;
                configurations->dmx_channel_quantity = 1;
                break;
            default:
                configurations->channels_operation_mode = CCT1_MODE;
                configurations->dmx_channel_quantity = 2;                
                break;
            }

            menu_state = MENU_SHOW_OPERATION_MODE;
            resp = resp_continue;
        }
        
        break;

    case MENU_CONF_MAX_CURRENT:
        // old config
        // fchannel = ConvertCurrentFromMemory(configurations);        
        // resp = LCD_EncoderChange("Max current:    ",
        //                          &fchannel,
        //                          1,
        //                          8,
        //                          sw_action);

        // new config
        new_current_int = configurations->current_int;
        new_current_dec = configurations->current_dec;
        resp = LCD_EncoderChange_Decimals ("Max current:    ",
                                           &new_current_int, &new_current_dec,
                                           1, 0,
                                           8, 0,
                                           sw_action);
        
        if (resp == resp_finish)
        {
            ConvertCurrentToMemory(configurations, new_current_int, new_current_dec);
            menu_state = MENU_SHOW_MAX_CURRENT;
            resp = resp_continue;
        }

        break;

    case MENU_CONF_TEMP:
#ifdef TEMP_SENSOR_LM335
        fchannel = Temp_TempToDegrees(configurations->temp_prot);

        resp = LCD_EncoderChange("Max temp:       ",
                                 &fchannel,
                                 TEMP_DEG_MIN,
                                 TEMP_DEG_MAX,
                                 sw_action);
                                     
        if (resp == resp_finish)
        {
            configurations->temp_prot = Temp_DegreesToTemp(fchannel);

            menu_state = MENU_SHOW_TEMP;
            resp = resp_continue;
        }
#endif
#ifdef TEMP_SENSOR_NTC1K
        fchannel = configurations->temp_prot_deg;

        resp = LCD_EncoderChange("Max temp:       ",
                                 &fchannel,
                                 TEMP_DEG_MIN,
                                 TEMP_DEG_MAX,
                                 sw_action);
                                     
        if (resp == resp_finish)
        {
            configurations->temp_prot_deg = (unsigned char) fchannel;
            configurations->temp_prot = Temp_DegreesToTemp(fchannel);

            menu_state = MENU_SHOW_TEMP;
            resp = resp_continue;
        }
#endif
        break;

    case MENU_CONF_CURRENT_TEMP:
        sprintf(s_temp, "temp: %dC       ", Temp_TempToDegreesExtended(current_temp));

        resp = LCD_ShowBlink (s_temp,
                              "                ",
                              1,
                              BLINK_NO);

        if (resp == resp_finish)
        {
            current_temp = Temp_Channel;
            resp = resp_continue;
        }
        
        if (sw_action == selection_enter)
        {
            menu_state = MENU_CONF_CURRENT_TEMP_1;
            resp = resp_continue;
        }
        break;

    case MENU_CONF_CURRENT_TEMP_1:
        if (sw_action == selection_none)
        {
            menu_state = MENU_SHOW_CURRENT_TEMP;
            resp = resp_continue;
        }
        break;
        
    case MENU_CONF_VERSION_CONTROL:
        sprintf(s_temp, "%s %s", HARD_GetHardwareVersion(), HARD_GetSoftwareVersion());
        resp = LCD_ShowBlink (" Control Board  ",
                              s_temp,
                              2,
                              BLINK_NO);
        // resp = LCD_ShowBlink (HARD_GetHardwareVersion(),
        //                       HARD_GetSoftwareVersion(),
        //                       2,
        //                       BLINK_NO);

        if (resp == resp_finish)
        {
            LCD_Writel2 (" version unknown");

            menu_state = MENU_CONF_VERSION_POWER;                
            resp = resp_continue;
        }
        break;

    case MENU_CONF_VERSION_POWER:
        resp = LCD_ShowBlink ("  Power Board   ",
                              Comms_Power_Check_Version(),
                              2,
                              BLINK_NO);

        if (resp == resp_finish)
        {
            menu_state = MENU_SHOW_VERSION;
            resp = resp_continue;
        }
        break;
        
    case MENU_END_CONF:
        resp = LCD_ShowBlink ("   Saving new   ",
                              "    params...   ",
                              1,
                              BLINK_DIRECT);

        if (resp == resp_finish)
        {
            resp = resp_continue;
            if (sw_action == selection_none)
            {
                resp = resp_need_to_save;
                menu_state = MENU_INIT;
            }
        }
        break;
        
    default:
        menu_state = MENU_INIT;
        break;
    }

    return resp;
}


unsigned char ConvertCurrentFromMemory (mem_bkp_t * config)
{
    unsigned char curr = 0;

    if (config->current_eight_amps == 0)
    {
        if (config->max_current_channels[0] > 192)
            curr = 4;
        else if (config->max_current_channels[0] > 128)
            curr = 3;
        else if (config->max_current_channels[0] > 64)
            curr = 2;
        else
            curr = 1;
    }
    else
    {
        if (config->max_current_channels[0] > 224)
            curr = 8;
        else if (config->max_current_channels[0] > 192)
            curr = 7;
        else if (config->max_current_channels[0] > 160)
            curr = 6;
        else
            curr = 5;
    }
        
    return curr;
}

    
void ConvertCurrentToMemory (mem_bkp_t * config,
                             unsigned char current_int,
                             unsigned char current_dec)
{
    // new memory backup
    config->current_int = current_int;
    config->current_dec = current_dec;    

    // old memory backup
    // switch (current)
    // {
    // case 8:
        
    //     config->max_current_channels[0] = 255;
    //     config->max_current_channels[1] = 255;
    //     break;

    // case 7:
    //     config->current_eight_amps = 1;
    //     config->max_current_channels[0] = 224;
    //     config->max_current_channels[1] = 224;        
    //     break;

    // case 6:
    //     config->current_eight_amps = 1;
    //     config->max_current_channels[0] = 192;
    //     config->max_current_channels[1] = 192;
    //     break;

    // case 5:
    //     config->current_eight_amps = 1;
    //     config->max_current_channels[0] = 160;
    //     config->max_current_channels[1] = 160;        
    //     break;

    // case 4:
    //     config->current_eight_amps = 0;
    //     config->max_current_channels[0] = 255;
    //     config->max_current_channels[1] = 255;        
    //     break;

    // case 3:
    //     config->current_eight_amps = 0;
    //     config->max_current_channels[0] = 192;
    //     config->max_current_channels[1] = 192;        
    //     break;

    // case 2:
    //     config->current_eight_amps = 0;
    //     config->max_current_channels[0] = 128;
    //     config->max_current_channels[1] = 128;        
    //     break;
 
    // case 1:
    // default:
    //     config->current_eight_amps = 0;
    //     config->max_current_channels[0] = 64;
    //     config->max_current_channels[1] = 64;        
    //     break;

    // }
}
    
//--- end of file ---//
