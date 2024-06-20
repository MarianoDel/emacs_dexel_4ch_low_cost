//----------------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### HARD.C #######################################
//----------------------------------------------------

// Includes --------------------------------------------------------------------
#include "hard.h"
#include "stm32g0xx.h"
#include "switches_answers.h"
#include "parameters.h"


// Module Private Types Constants and Macros -----------------------------------
#ifdef HARDWARE_VERSION_2_0
#define SWITCHES_TIMER_RELOAD    5
#define SWITCHES_THRESHOLD_FULL	1000    //5 segundos
#define SWITCHES_THRESHOLD_HALF	50    //250 ms
#define SWITCHES_THRESHOLD_MIN	10    //50 ms
#define SWITCHES_THRESHOLD_MIN_FAST    2    //10 ms
#endif

#ifdef HARDWARE_VERSION_2_1
#define SWITCHES_TIMER_RELOAD    5
#define SWITCHES_THRESHOLD_FULL	2000    //10 segundos
#define SWITCHES_THRESHOLD_HALF	50    //250 ms
#define SWITCHES_THRESHOLD_MIN	10    //50 ms
#define SWITCHES_THRESHOLD_MIN_FAST    2    //10 ms

#define ENCODER_COUNTER_ROOF    10
#define ENCODER_COUNTER_THRESHOLD    3
#endif

// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------
//for timers or timeouts
volatile unsigned char switches_timer = 0;


// Module Private Functions ----------------------------------------------------
void UpdateEncoderFilters (void);

// Module Functions ------------------------------------------------------------
void HARD_Timeouts (void)
{
    if (switches_timer)
        switches_timer--;

#ifdef HARDWARE_VERSION_2_1
    UpdateEncoderFilters ();
#endif
}


// Encoder Routines ------------------------------------------------------------
#ifdef HARDWARE_VERSION_2_1
unsigned short sw_set_cntr = 0;

resp_sw_t CheckSET (void)
{
    resp_sw_t sw = SW_NO;
    
    if (sw_set_cntr > SWITCHES_THRESHOLD_FULL)
        sw = SW_FULL;
    else if (sw_set_cntr > SWITCHES_THRESHOLD_HALF)
        sw = SW_HALF;
    else if (sw_set_cntr > SWITCHES_THRESHOLD_MIN)
        sw = SW_MIN;

    return sw;    
}


void UpdateSwitches (void)
{
    if (!switches_timer)
    {
        if (EN_SW)
            sw_set_cntr++;
        else if (sw_set_cntr > 50)
            sw_set_cntr -= 50;
        else if (sw_set_cntr > 10)
            sw_set_cntr -= 5;
        else if (sw_set_cntr)
            sw_set_cntr--;
        
        switches_timer = SWITCHES_TIMER_RELOAD;
    }       
}


volatile unsigned char enc_clk_cntr = 0;
volatile unsigned char enc_dt_cntr = 0;
void UpdateEncoderFilters (void)
{
    if (EN_CLK)
    {
        if (enc_clk_cntr < ENCODER_COUNTER_ROOF)
            enc_clk_cntr++;
    }
    else
    {
        if (enc_clk_cntr)
            enc_clk_cntr--;        
    }

    if (EN_DT)
    {
        if (enc_dt_cntr < ENCODER_COUNTER_ROOF)
            enc_dt_cntr++;
    }
    else
    {
        if (enc_dt_cntr)
            enc_dt_cntr--;        
    }
}


unsigned char last_clk = 0;
unsigned char encoder_ccw = 0;
unsigned char encoder_cw = 0;
extern parameters_typedef mem_conf;
void UpdateEncoder (void)
{
    unsigned char current_clk = 0;

    //check if we have rising edge on clk
    if (enc_clk_cntr > ENCODER_COUNTER_THRESHOLD)
        current_clk = 1;
    else
        current_clk = 0;
    
    if ((last_clk == 0) && (current_clk == 1))    //rising edge
    {
        //have a new clock edge
        if (enc_dt_cntr > ENCODER_COUNTER_THRESHOLD)
        {
            if(mem_conf.encoder_direction == 0)    // USE_ENCODER_DIRECT
            {
                //CW
                if (encoder_cw < 1)
                    encoder_cw++;
            }
            else
            {
                //CCW
                if (encoder_ccw < 1)
                    encoder_ccw++;
            }
        }
        else
        {
            if(mem_conf.encoder_direction == 0)    // USE_ENCODER_DIRECT
            {
                //CCW
                if (encoder_ccw < 1)
                    encoder_ccw++;
            }
            else
            {
                //CW
                if (encoder_cw < 1)
                    encoder_cw++;
            }
        }
    }

    if (last_clk != current_clk)
        last_clk = current_clk;
}


unsigned char CheckCCW (void)
{
    unsigned char a = 0;
    
    if (encoder_ccw)
    {
        encoder_ccw--;
        a = 1;
    }
    
    return a;
}


unsigned char CheckCW (void)
{
    unsigned char a = 0;
    
    if (encoder_cw)
    {
        encoder_cw--;
        a = 1;
    }
    
    return a;
}


unsigned char last_was_enter = 0;
sw_actions_t CheckActions (void)
{
    sw_actions_t sw = selection_none;
    resp_sw_t s_sel = SW_NO;

    s_sel = CheckSET ();
    
    if (CheckCW () > SW_NO)
    {
        if (s_sel > SW_NO)
        {
            sw = selection_up_fast;
            last_was_enter = 2;
        }
        else
            sw = selection_up;
    }

    if (CheckCCW () > SW_NO)
    {
        if (s_sel > SW_NO)
        {
            sw = selection_dwn_fast;
            last_was_enter = 2;
        }
        else
            sw = selection_dwn;
    }
    
    if (s_sel > SW_HALF)
        sw = selection_back;
    else if (s_sel > SW_NO)
    {
        if (!last_was_enter)
            last_was_enter = 1;
    }
    else    // s_sel == SW_NO
    {
        if (last_was_enter == 2)    // fast update
        {
        }
        else if (last_was_enter == 1)    // no fast update
        {
            sw = selection_enter;
        }
        
        last_was_enter = 0;            
    }
    
    return sw;    
}
#endif    // HARDWARE_VERSION_2_1
// End of Encoder Routines -----------------------------------------------------


// Switches Routines -----------------------------------------------------------
#ifdef HARDWARE_VERSION_2_0
unsigned short sw_up_cntr = 0;
unsigned short sw_dwn_cntr = 0;
unsigned short sw_sel_cntr = 0;
unsigned char sw_up_cont_cntr = 0;
unsigned char sw_dwn_cont_cntr = 0;
resp_sw_t Check_SW_UP (void)
{
    resp_sw_t sw = SW_NO;

    if (sw_up_cont_cntr >= 5)
    {
        sw_up_cont_cntr = 5;

        if (sw_up_cntr > SWITCHES_THRESHOLD_MIN_FAST)
        {
            sw_up_cntr -= SWITCHES_THRESHOLD_MIN_FAST;
            sw = SW_MIN;
        }
    }
    else if (sw_up_cntr > SWITCHES_THRESHOLD_MIN)
    {
        sw_up_cntr -= SWITCHES_THRESHOLD_MIN;
        sw = SW_MIN;
        sw_up_cont_cntr++;
    }

    return sw;    
}


resp_sw_t Check_SW_DWN (void)
{
    resp_sw_t sw = SW_NO;

    if (sw_dwn_cont_cntr >= 5)
    {
        sw_dwn_cont_cntr = 5;

        if (sw_dwn_cntr > SWITCHES_THRESHOLD_MIN_FAST)
        {
            sw_dwn_cntr -= SWITCHES_THRESHOLD_MIN_FAST;
            sw = SW_MIN;
        }
    }
    else if (sw_dwn_cntr > SWITCHES_THRESHOLD_MIN)
    {
        sw_dwn_cntr -= SWITCHES_THRESHOLD_MIN;
        sw = SW_MIN;
        sw_dwn_cont_cntr++;
    }

    return sw;    
}


resp_sw_t Check_SW_SEL (void)
{
    resp_sw_t sw = SW_NO;
    
    if (sw_sel_cntr > SWITCHES_THRESHOLD_FULL)
        sw = SW_FULL;
    else if (sw_sel_cntr > SWITCHES_THRESHOLD_HALF)
        sw = SW_HALF;
    else if (sw_sel_cntr > SWITCHES_THRESHOLD_MIN)
    {
        // sw_sel_cntr -= SWITCHES_THRESHOLD_MIN;
        sw = SW_MIN;
    }

    return sw;    
}


void UpdateSwitches (void)
{
    if (!switches_timer)
    {
        if (SW_UP)
            sw_up_cntr++;
        else if (sw_up_cntr > 50)
            sw_up_cntr -= 50;
        else if (sw_up_cntr > 10)
            sw_up_cntr -= 5;
        else if (sw_up_cntr)
            sw_up_cntr--;
        else
            sw_up_cont_cntr = 0;

        if (SW_DWN)
            sw_dwn_cntr++;
        else if (sw_dwn_cntr > 50)
            sw_dwn_cntr -= 50;
        else if (sw_dwn_cntr > 10)
            sw_dwn_cntr -= 5;
        else if (sw_dwn_cntr)
            sw_dwn_cntr--;
        else
            sw_dwn_cont_cntr = 0;

        if (SW_SEL)
            sw_sel_cntr++;
        else if (sw_sel_cntr > 50)
            sw_sel_cntr -= 50;
        else if (sw_sel_cntr > 10)
            sw_sel_cntr -= 5;
        else if (sw_sel_cntr)
            sw_sel_cntr--;
        
        switches_timer = SWITCHES_TIMER_RELOAD;
    }       
}


sw_actions_t CheckActions (void)
{
    sw_actions_t sw = selection_none;
    
    if (Check_SW_UP () > SW_NO)
        sw = selection_up;

    if (Check_SW_DWN () > SW_NO)
        sw = selection_dwn;

    resp_sw_t s_sel = SW_NO;
    s_sel = Check_SW_SEL ();
    
    if (s_sel > SW_HALF)
        sw = selection_back;
    else if (s_sel > SW_NO)
        sw = selection_enter;
    
    return sw;
    
}
#endif    // HARDWARE_VERSION_2_0
// End of Switches Routines ----------------------------------------------------

#if defined HARDWARE_VERSION_2_1
char hardware_version [] = {"Hrd 2.1"};
#elif defined HARDWARE_VERSION_2_0
char hardware_version [] = {"Hrd 2.0"};
#else
#error "No hard version selected on hard.c"
#endif
char * HARD_GetHardwareVersion (void)
{
    return hardware_version;
}

#if defined FIRMWARE_VERSION_2_1
char software_version [] = {"Soft 2.1"};
#elif defined FIRMWARE_VERSION_2_0
char software_version [] = {"Soft 2.0"};
#else
#error "No soft version selected on hard.c"
#endif

char * HARD_GetSoftwareVersion (void)
{
    return software_version;
}


//--- end of file ---//
