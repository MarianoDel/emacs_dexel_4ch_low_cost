//----------------------------------------------------
// Tests for gtk lib
// Implementation of tests_oled_application.h functions
//----------------------------------------------------

#include <gtk/gtk.h>
#include <stdint.h>
#include "tests_oled_application.h"

// Application Includes needed for this test
#include "main_menu.h"
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

parameters_typedef mem;

// Testing Function loop -------------------------------------------------------
gboolean Test_Main_Loop (gpointer user_data)
{
    resp_t resp = resp_continue;

    if (setup_done == 0)
    {
        setup_done = 1;

        SCREEN_Init();

        SCREEN_Clear ();        
        SCREEN_Text2_Line1 ("Dexel     ");    
        SCREEN_Text2_Line2 ("  Lighting");

        Main_Menu_Reset ();
        mem.max_current_channels[0] = 128;
        mem.max_current_channels[1] = 121;
        mem.max_current_channels[2] = 120;
        mem.max_current_channels[3] = 64;

	mem.dmx_channel_quantity = 1;
        mem.temp_prot_deg = 30;
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
        resp = Main_Menu (&mem, switch_actions);

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

        if (resp == resp_ok)
        {
            printf("resp_ok ended!\n");
            setup_done = 3;
        }
        
        display_update_int_state_machine ();

    }

    if (setup_done ==3)
    {
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

    Main_Menu_Timeouts();
    
    return TRUE;
}


gboolean Test_Timeouts_Loop_1000ms (gpointer user_data)
{
    
    return TRUE;
}


// Module Implementation of buttons functions
// only two switches, answers always up
void button1_function (void)
{
    g_mutex_lock (&mutex);
    switch_actions = selection_up;
    g_mutex_unlock (&mutex);
}


void button2_function (void)
{
    g_mutex_lock (&mutex);
    switch_actions = selection_dwn;
    g_mutex_unlock (&mutex);
}


void button3_function (void)
{
    g_mutex_lock (&mutex);
    switch_actions = selection_enter;
    g_mutex_unlock (&mutex);
}


// Module Mocked Functions -----------------------------------------------------
char hardware_version [] = {"Hard 1.0"};
char software_version [] = {"Soft 1.0"};
char * HARD_GetHardwareVersion (void)
{
    return hardware_version;
}

char * HARD_GetSoftwareVersion (void)
{
    return software_version;
}


void Check_S1_Accel_Fast (void)
{
}


void Check_S1_Accel_Slow (void)
{
}


void Check_S2_Accel_Fast (void)
{
}


void Check_S2_Accel_Slow (void)
{
}


//--- end of file ---//
