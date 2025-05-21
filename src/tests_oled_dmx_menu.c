//----------------------------------------------------
// Tests for gtk lib
// Implementation of tests_oled_application.h functions
//----------------------------------------------------

#include <gtk/gtk.h>
#include <stdint.h>
#include "tests_oled_application.h"

// Application Includes needed for this test
#include "dmx_menu.h"
#include "screen.h"
#include "switches_answers.h"
#include "parameters.h"


// Module Types Constants and Macros -------------------------------------------


// Externals -- Access to the tested Module ------------------------------------


// Globals -- Externals for the tested Module ----------------------------------
sw_actions_t switch_actions = selection_none;
volatile unsigned short adc_ch [1];


// - for DMX receiver
volatile unsigned char dmx_buff_data [5];
volatile unsigned char Packet_Detected_Flag = 0;
volatile unsigned short DMX_channel_selected = 1;
volatile unsigned char DMX_channel_quantity = 4;
volatile unsigned char dmx_receive_flag = 0;


// Globals ---------------------------------------------------------------------
static GMutex mutex;
int setup_done = 0;
unsigned int timer_standby = 0;

int dmx_cnt = 0;
int change_dmx_values = 0;
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

        Dmx_Menu_Reset ();
        mem.dmx_first_channel = 1;
        // mem.dmx_channel_quantity = 3;
        mem.dmx_channel_quantity = 1;	
        printf("testing with %d channels\n", mem.dmx_channel_quantity);        
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
        resp = Dmx_Menu (&mem, switch_actions);

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
        }
        
        display_update_int_state_machine ();

        if (change_dmx_values)
        {
            change_dmx_values = 0;
            Packet_Detected_Flag = 1;

            if (dmx_cnt < 255)
                dmx_cnt++;
            else
                dmx_cnt = 0;
            
            for (int i = 1; i < 5; i++)
                dmx_buff_data[i] = dmx_cnt;
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

    Dmx_Menu_Timeouts();
    
    return TRUE;
}


gboolean Test_Timeouts_Loop_1000ms (gpointer user_data)
{
    change_dmx_values = 1;
    
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
    switch_actions = selection_up;
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
