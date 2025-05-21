//----------------------------------------------------
// Tests for gtk lib
// Implementation of tests_oled_application.h functions
//----------------------------------------------------

#include <gtk/gtk.h>
#include <stdint.h>
#include "tests_oled_application.h"

// Application Includes needed for this test
#include "manual_menu.h"
#include "screen.h"
#include "switches_answers.h"
#include "parameters.h"


// Module Types Constants and Macros -------------------------------------------


// Externals -- Access to the tested Module ------------------------------------


// Globals -- Externals for the tested Module ----------------------------------
sw_actions_t switch_actions = selection_none;
volatile unsigned short adc_ch [1];
unsigned char dmx_local_data [4] = { 0 };


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

        Manual_Menu_Reset ();
        mem.fixed_channels[0] = 255;
        mem.fixed_channels[1] = 255;
        mem.fixed_channels[2] = 0;
        mem.fixed_channels[3] = 0;
        // mem.dmx_channel_quantity = 4;
        // mem.dmx_channel_quantity = 3;
        mem.dmx_channel_quantity = 1;
        mem.manual_inner_mode = 0;
        mem.manual_inner_speed = 3;
        
        printf("-- testing with %d channels on mode: %d speed: %d\n",
               mem.dmx_channel_quantity,
               mem.manual_inner_mode,
               mem.manual_inner_speed);
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
        resp = Manual_Menu (&mem, switch_actions);

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
            printf("resp_change -> update colors\n");
        }

        if (resp == resp_need_to_save)
        {
            printf("resp_need_to_save -> update colors -> save memory!!!\n");
        }
        
        if (resp == resp_ok)
        {
            printf("resp_ok ended!\n");
        }
        
        display_update_int_state_machine ();    

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

    Manual_Menu_Timeouts();
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
unsigned char Manager_Probe_Temp_Get (void)
{
    return 1;
}

unsigned char Temp_TempToDegreesExtended (unsigned short temp)
{
    return 30;
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
