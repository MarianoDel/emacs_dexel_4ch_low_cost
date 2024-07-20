//----------------------------------------------------
// #### DEXEL 2CH PROJECT - Custom Board ####
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


//--- end of file ---//
