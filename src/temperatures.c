//----------------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### TEMPERATURES.C ###############################
//----------------------------------------------------

// Includes --------------------------------------------------------------------
#include "temperatures.h"


// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------
unsigned char temp_probe_present = 0;
unsigned short temp_probe_meas = 0;


// Module Private Types & Macros -----------------------------------------------


// Module Private Functions ----------------------------------------------------


// Module Functions ------------------------------------------------------------
unsigned char Temp_TempToDegreesExtended (unsigned short temp)
{
    int calc = 0;
    short dx = TEMP_IN_85 - TEMP_IN_30;
    short dy = 85 - 30;

    calc = temp * dy;
    calc = calc / dx;

    if (calc >= TEMP_DEG_OFFSET)    // no less than 0 degrees
        calc = calc - TEMP_DEG_OFFSET;
    else
        calc = 0;

    return (unsigned char) calc;
}


unsigned char Temp_Probe_Present_Get (void)
{
    return temp_probe_present;
}
        

void Temp_Probe_Present_Set (void)
{
    temp_probe_present = 1;
}


void Temp_Probe_Present_Reset (void)
{
    temp_probe_present = 0;
}


void Temp_Probe_Meas_Filtered_Save (unsigned short temp_filtered)
{
    temp_probe_meas = temp_filtered;
}


unsigned short Temp_Probe_Meas_Filtered_Get (void)
{
    return temp_probe_meas;
}

//--- end of file ---//
