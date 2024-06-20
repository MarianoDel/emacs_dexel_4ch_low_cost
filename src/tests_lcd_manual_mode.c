//----------------------------------------------------
// Tests for gtk lib
// Implementation of tests_lcd_application.h functions
//----------------------------------------------------

#include <gtk/gtk.h>
#include <stdint.h>
#include "tests_lcd_application.h"

// Application Includes needed for this test
#include "lcd_utils.h"
#include "manual_mode.h"
#include "parameters.h"


// Module Types Constants and Macros -------------------------------------------


// Externals -- Access to the tested Module ------------------------------------
extern volatile unsigned short show_select_timer;


// Globals -- Externals for the tested Module ----------------------------------
volatile unsigned short mode_effect_timer = 0;
unsigned char mode_state;

volatile unsigned char dmx_buff_data[3] = { 0 };
volatile unsigned char Packet_Detected_Flag = 0;
volatile unsigned short DMX_channel_selected = 1;
parameters_typedef mem_conf;
unsigned char pwm_channels [2] = { 0 };

sw_actions_t switch_actions = selection_none;


// Globals ---------------------------------------------------------------------
static GMutex mutex;
int setup_done = 0;



// Testing Function loop -------------------------------------------------------
gboolean Test_Main_Loop (gpointer user_data)
{
    resp_t resp = resp_continue;

    // mem_conf.channels_operation_mode = CCT1_MODE;
    // mem_conf.channels_operation_mode = CCT2_MODE;
    mem_conf.channels_operation_mode = ONECH_MODE;    
    resp = ManualMode (pwm_channels, switch_actions);

    if (resp == resp_need_to_save)
    {
        printf("memory needs a save!\n");
    }

    //wraper to clean sw
    g_mutex_lock (&mutex);

    if (switch_actions != selection_none)
        switch_actions = selection_none;
    
    g_mutex_unlock (&mutex);
    // usleep(500);
        
    return TRUE;
}


gboolean Test_Timeouts (gpointer user_data)
{
    //timeout lcd_utils internal
    if (show_select_timer)
        show_select_timer--;

    //timeout for dmx_mode or manual_mode
    if (mode_effect_timer)
        mode_effect_timer--;
    
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


//--- end of file ---//
