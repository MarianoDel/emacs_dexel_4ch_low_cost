//----------------------------------------------------
// Tests for gtk lib
// Implementation of tests_lcd_application.h functions
//----------------------------------------------------

#include <gtk/gtk.h>
#include <stdint.h>
#include "tests_lcd_application.h"

// Application Includes needed for this test
#include "lcd_utils.h"
#include "menues.h"
#include "parameters.h"


// Module Types Constants and Macros -------------------------------------------


// Externals -- Access to the tested Module ------------------------------------
extern volatile unsigned short show_select_timer;


// Globals -- Externals for the tested Module ----------------------------------
sw_actions_t switch_actions = selection_none;
parameters_typedef configurations;

// for the hard.h inclusion
unsigned short adc_ch [2];


// Globals ---------------------------------------------------------------------
static GMutex mutex;
int setup_done = 0;



// Testing Function loop -------------------------------------------------------
gboolean Test_Main_Loop (gpointer user_data)
{    
    resp_t resp = resp_continue;

    if (!setup_done)
    {
        configurations.current_int = 1;
        configurations.current_dec = 0;
        setup_done = 1;
    }

    if (setup_done)
    {
        resp = MENU_Main (&configurations, switch_actions);
    
        if (resp == resp_need_to_save)
        {
            printf("memory needs a save!\n");
            printf("current set to: %01d.%01dA\n", configurations.current_int, configurations.current_dec);
        }

        //wraper to clean sw
        g_mutex_lock (&mutex);

        if (switch_actions != selection_none)
            switch_actions = selection_none;
    
        g_mutex_unlock (&mutex);
    }
        
    return TRUE;
}


gboolean Test_Timeouts (gpointer user_data)
{
    //timeout lcd_utils internal
    if (show_select_timer)
        show_select_timer--;

    return TRUE;
}


// Module Implemantation of LCD buttons functions
void dwn_button_function (void)
{
    g_mutex_lock (&mutex);
    switch_actions = selection_dwn;
    g_mutex_unlock (&mutex);
}

void up_button_function (void)
{
    g_mutex_lock (&mutex);
    switch_actions = selection_up;
    g_mutex_unlock (&mutex);
}

void set_button_function (void)
{
    g_mutex_lock (&mutex);
    switch_actions = selection_enter;
    g_mutex_unlock (&mutex);
}


// Nedded by menues module provided by hard module
void UpdateSwitches (void)
{
}

resp_sw_t Check_SW_SEL (void)
{
    resp_sw_t sw = SW_NO;
    
    g_mutex_lock (&mutex);

    if (switch_actions == selection_enter)
        sw = SW_MIN;
    
    g_mutex_unlock (&mutex);
    
    return sw;    
}

unsigned char Check_SW_DWN (void)
{
    unsigned char a = 0;
    
    g_mutex_lock (&mutex);

    if (switch_actions == selection_dwn)
        a = 1;
    
    g_mutex_unlock (&mutex);
    
    return a;
}


unsigned char Check_SW_UP (void)
{
    unsigned char a = 0;
    
    g_mutex_lock (&mutex);

    if (switch_actions == selection_up)
        a = 1;
    
    g_mutex_unlock (&mutex);
    
    return a;
}

// -- Mocked Functions ---------------------------------------------------------
char * Comms_Power_Check_Version (void)
{
    static char s_power_version [] = {"Hrd 2.1 Soft 1.3"};
    return s_power_version;
}

char * HARD_GetHardwareVersion (void)
{
    static char hard [] = {"Hardware: 1.2   "};
    return hard;
}

char * HARD_GetSoftwareVersion (void)
{
    static char soft [] = {"Software: 1.2   "};
    return soft;
}

//--- end of file ---//
