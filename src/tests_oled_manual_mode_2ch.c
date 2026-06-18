//----------------------------------------------------
// Tests for gtk lib
// Implementation of tests_oled_application.h functions
//----------------------------------------------------

#include <gtk/gtk.h>
#include <stdint.h>
#include "tests_oled_application.h"

// Application Includes needed for this test
// #include "main_menu.h"
#include "manual_mode_2ch.h"
#include "screen.h"
#include "switches_answers.h"
#include "parameters.h"


// Module Types Constants and Macros -------------------------------------------


// Externals -- Access to the tested Module ------------------------------------
extern void display_update_int_state_machine (void);
extern void DataShow_2Ch (unsigned char to_show,
			  unsigned char bright,
			  unsigned char temp,
			  unsigned short degree);


// Globals -- Externals for the tested Module ----------------------------------
sw_actions_t switch_actions = selection_none;
unsigned char mode_state = 0;
unsigned short mode_effect_timer = 0;
unsigned int timer_standby = 0;

parameters_typedef mem;
unsigned char mode_show_options = 0;
unsigned char mode_cntr_out = 0;

unsigned char dmx_local_data[4] = { 0 };


// Globals ---------------------------------------------------------------------
static GMutex mutex;
int setup_done = 0;
int toggled_on = 0;


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

        // Main_Menu_Reset ();
	mem.program_type = CCT2_MODE;
        mem.max_current_channels[0] = 128;
        mem.max_current_channels[1] = 121;
        mem.max_current_channels[2] = 120;
        mem.max_current_channels[3] = 64;

	mem.dmx_channel_quantity = 2;
        mem.temp_prot_deg = 30;
        timer_standby = 1300;
	printf("to setup 1\n");
    }

    if (setup_done == 1)
    {
        if (timer_standby)
            display_update_int_state_machine ();
        else
	{
            setup_done = 2;
	    ManualMode_2Ch_Reset();
	    printf("reseting manual mode, go loop\n");	    
	}
    }

    if (setup_done == 2)
    {
        resp = ManualMode_2Ch (&mem, switch_actions);

        if (resp == resp_need_to_save)
        {
            printf("a save is needed!!!\n");
        }

        if (resp == resp_change)
        {
            printf("resp_change\n");

	    printf("mem.fixed_channels getted\n");
	    for (int i = 0; i < 4; i++)
		printf("mem.fixed_channels[%d]: %d\n", i, mem.fixed_channels[i]);
	    
	    unsigned short calc = 0;
	    unsigned char bright = 0;
	    unsigned char temp0 = 0;
	    unsigned char temp1 = 0;

	    // backup and bright temp calcs
	    // ch0 the bright ch1 the temp
	    bright = mem.fixed_channels[0];
	    temp0 = 255 - mem.fixed_channels[1];
	    temp1 = 255 - temp0;

	    calc = temp0 * bright;
	    calc >>= 8;

	    if ((bright) && (temp0))
		dmx_local_data[0] = (unsigned char) calc + 1;
	    else
		dmx_local_data[0] = 0;
	    
	    dmx_local_data[1] = dmx_local_data[0];
	    
	    calc = temp1 * bright;
	    calc >>= 8;

	    if ((bright) && (temp1))
		dmx_local_data[2] = (unsigned char) calc + 1;
	    else
		dmx_local_data[2] = 0;

	    dmx_local_data[3] = dmx_local_data[2];

	    printf("dmx_local_data processed\n");	    
	    for (int i = 0; i < 4; i++)
		printf("dmx_local_data[%d]: %d\n", i, dmx_local_data[i]);
        }

        if (resp == resp_ok)
        {
            printf("resp_ok ended!\n");
            setup_done = 3;
        }
        
        display_update_int_state_machine ();
    }

    //wraper to clean sw
    g_mutex_lock (&mutex);

    if (!toggled_on)
    {
	if (switch_actions != selection_none)
	    switch_actions = selection_none;
    }
    
    g_mutex_unlock (&mutex);
    // usleep(500);
        
    return TRUE;
}

gboolean Test_Timeouts_Loop_1ms (gpointer user_data)
{
    if (timer_standby)
        timer_standby--;

    ManualMode_2Ch_UpdateTimers();
    
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


void toggled1_function (void)
{
    g_mutex_lock (&mutex);

    if (toggled_on)
    {
	toggled_on = 0;
	printf("toggled 1 off\n");
    }
    else
    {
	toggled_on = 1;
	printf("toggled 1 on\n");
    }
    switch_actions = selection_up;
    g_mutex_unlock (&mutex);    
}


void toggled2_function (void)
{
    g_mutex_lock (&mutex);

    if (toggled_on)
    {
	toggled_on = 0;
	printf("toggled 2 off\n");
    }
    else
    {
	toggled_on = 1;
	printf("toggled 2 on\n");
    }
    switch_actions = selection_dwn;
    g_mutex_unlock (&mutex);    
}


void toggled3_function (void)
{
    printf("toggled 3\n"); 
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


unsigned char Temp_TempToDegreesExtended (unsigned short degree)
{
    return 85;
}


unsigned short Temp_Probe_Meas_Filtered_Get (void)
{
    return 1000;
}


unsigned char Temp_Probe_Present_Get (void)
{
    return 1;
}

//--- end of file ---//
