//----------------------------------------------------
// Tests for gtk lib
// Implementation of tests_oled_application.h functions
//----------------------------------------------------

#include <gtk/gtk.h>
#include <stdint.h>
#include "tests_oled_application.h"

// Application Includes needed for this test
#include "menu_options_oled.h"
#include "screen.h"
#include "switches_answers.h"
#include "parameters.h"


// Module Types Constants and Macros -------------------------------------------


// Externals -- Access to the tested Module ------------------------------------


// Globals -- Externals for the tested Module ----------------------------------
sw_actions_t switch_actions = selection_none;


// Globals ---------------------------------------------------------------------
static GMutex mutex;
int setup_done = 0;
unsigned int timer_standby = 0;



// Testing Function loop -------------------------------------------------------
gboolean Test_Main_Loop (gpointer user_data)
{
    resp_t resp = resp_continue;

    if (setup_done == 0)
    {
        setup_done = 1;

        SCREEN_Init();

        SCREEN_Clear ();        
        SCREEN_Text2_Line1 ("Infinity  ");    
        SCREEN_Text2_Line2 ("  Clinics ");
        
        timer_standby = 1300;
    }

    if (setup_done == 1)
    {
        if (timer_standby)
            display_update_int_state_machine ();
        else
            setup_done = 2;
    }

    if (setup_done == 2)
    {
        resp = Options_Up_Dwn_Select (switch_actions);

        if (resp == resp_up)
        {
            printf("resp_up\n");
        }

        if (resp == resp_dwn)
        {
            printf("resp_dwn\n");
        }

        if (resp == resp_change)
        {
            printf("resp_change\n");
        }
        
    }
    
    //wraper to clean sw
    g_mutex_lock (&mutex);

    if (switch_actions != selection_none)
        switch_actions = selection_none;
    
    g_mutex_unlock (&mutex);
    // usleep(500);
        
    return TRUE;
}


gboolean Test_Timeouts_Loop_1ms (gpointer user_data)
{
    if (timer_standby)
        timer_standby--;

    return TRUE;
}


gboolean Test_Timeouts_Loop_1000ms (gpointer user_data)
{
    return TRUE;
}


// Module Implementation of buttons functions
// only two switches, answers always up
void cw_button_function (void)
{
    g_mutex_lock (&mutex);
    switch_actions = selection_up;
    g_mutex_unlock (&mutex);
}


void ccw_button_function (void)
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

// // Module Implementation of SW buttons functions
// // only two switches, answers always selection_up
// void dwn_button_function (void)
// {
//     g_mutex_lock (&mutex);
//     // switch_actions = selection_dwn;
//     switch_actions = selection_up;    
//     g_mutex_unlock (&mutex);
// }

// void up_button_function (void)
// {
//     g_mutex_lock (&mutex);
//     switch_actions = selection_up;
//     g_mutex_unlock (&mutex);
// }

// void set_button_function (void)
// {
//     g_mutex_lock (&mutex);
//     switch_actions = selection_enter;
//     g_mutex_unlock (&mutex);
// }


// Nedded by menues module provided by hard module
// void UpdateSwitches (void)
// {
// }

// resp_sw_t Check_SW_SEL (void)
// {
//     resp_sw_t sw = SW_NO;
    
//     g_mutex_lock (&mutex);

//     if (switch_actions == selection_enter)
//         sw = SW_MIN;
    
//     g_mutex_unlock (&mutex);
    
//     return sw;    
// }

// unsigned char Check_SW_DWN (void)
// {
//     unsigned char a = 0;
    
//     g_mutex_lock (&mutex);

//     if (switch_actions == selection_dwn)
//         a = 1;
    
//     g_mutex_unlock (&mutex);
    
//     return a;
// }


// unsigned char Check_SW_UP (void)
// {
//     unsigned char a = 0;
    
//     g_mutex_lock (&mutex);

//     if (switch_actions == selection_up)
//         a = 1;
    
//     g_mutex_unlock (&mutex);
    
//     return a;
// }


//--- end of file ---//
