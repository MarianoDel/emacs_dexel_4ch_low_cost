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
#define SWITCHES_TIMER_RELOAD    5
#define SWITCHES_THRESHOLD_FULL	1000    //5 secs
#define SWITCHES_THRESHOLD_HALF	50    //250 ms
#define SWITCHES_THRESHOLD_MIN	10    //50 ms
#define SWITCHES_THRESHOLD_MIN_FAST    2    //10 ms


// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------
//for timers or timeouts
volatile unsigned char switches_timer = 0;


// Module Private Functions ----------------------------------------------------


// Module Functions ------------------------------------------------------------
void HARD_Timeouts (void)
{
    if (switches_timer)
        switches_timer--;
}


// Switches Routines -----------------------------------------------------------
unsigned short s1_cntr = 0;
unsigned short s2_cntr = 0;
resp_sw_t Check_S1 (void)
{
    resp_sw_t sw = SW_NO;

    if (s1_cntr > SWITCHES_THRESHOLD_MIN)
    {
        // s1_cntr -= SWITCHES_THRESHOLD_MIN;
        sw = SW_MIN;
    }

    return sw;    
}


unsigned char s2_accel = 0;
resp_sw_t Check_S2 (void)
{
    resp_sw_t sw = SW_NO;

    if (!s2_accel)
    {
        if (s2_cntr > SWITCHES_THRESHOLD_FULL)
            sw = SW_FULL;
        else if (s2_cntr > SWITCHES_THRESHOLD_HALF)
            sw = SW_HALF;
        else if (s2_cntr > SWITCHES_THRESHOLD_MIN)
            sw = SW_MIN;
    }
    else
    {
        if (s2_cntr > SWITCHES_THRESHOLD_MIN)
        {
            s2_cntr -= SWITCHES_THRESHOLD_MIN;
            sw = SW_MIN;
        }
    }
    
    return sw;    
}


void Check_S2_Accel_Fast (void)
{
    s2_accel = 1;
}


void Check_S2_Accel_Slow (void)
{
    s2_accel = 0;
}

void HARD_UpdateSwitches (void)
{
    if (!switches_timer)
    {
        if (S1)
            s1_cntr++;
        else if (s1_cntr > 50)
            s1_cntr -= 50;
        else if (s1_cntr > 10)
            s1_cntr -= 5;
        else if (s1_cntr)
            s1_cntr--;
        // else
        //     s1_cont_cntr = 0;

        if (S2)
            s2_cntr++;
        else if (s2_cntr > 50)
            s2_cntr -= 50;
        else if (s2_cntr > 10)
            s2_cntr -= 5;
        else if (s2_cntr)
            s2_cntr--;
        // else
        //     s2_cont_cntr = 0;
        
        switches_timer = SWITCHES_TIMER_RELOAD;
    }       
}


sw_actions_t CheckActions (void)
{
    sw_actions_t sw = selection_none;
    
    if (Check_S1 () > SW_NO)
        sw = selection_up;

    // if (Check_SW_DWN () > SW_NO)
    //     sw = selection_dwn;

    resp_sw_t s_sel = SW_NO;
    s_sel = Check_S2 ();
    
    if (s_sel > SW_HALF)
        sw = selection_back;
    else if (s_sel > SW_NO)
        sw = selection_enter;
    
    return sw;
    
}


// End of Switches Routines ----------------------------------------------------

#if defined HARDWARE_VERSION_1_0
char hardware_version [] = {"Hrd 1.0"};
#elif defined HARDWARE_VERSION_2_0
char hardware_version [] = {"Hrd 2.0"};
#else
#error "No hard version selected on hard.c"
#endif
char * HARD_GetHardwareVersion (void)
{
    return hardware_version;
}

#if defined FIRMWARE_VERSION_1_0
char software_version [] = {"Soft 1.0"};
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
