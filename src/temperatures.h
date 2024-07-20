//-------------------------------------------------
// #### DEXEL 2CH PROJECT - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### TEMPERATURES.H ############################
//-------------------------------------------------

// Prevent recursive inclusion -------------------------------------------------
#ifndef _TEMPERATURES_H_
#define _TEMPERATURES_H_

#include "temperatures_hard_defs.h"

// Exported Constants ----------------------------------------------------------
#ifdef TEMP_SENSOR_LM335
//---- LM335 measurement Temperatures (8.2mV/C in this circuit) -- 10.2 pts per degree
#define TEMP_IN_30    3120
#define TEMP_IN_35    3172
#define TEMP_IN_48    
#define TEMP_IN_50    
#define TEMP_IN_65    
#define TEMP_IN_70    3530
#define TEMP_IN_85    3673

#define TEMP_DISCONECT    TEMP_IN_85
#define TEMP_RECONNECT    TEMP_IN_48

#define TEMP_IN_MIN    TEMP_IN_30
#define TEMP_IN_MAX    TEMP_IN_85

#define TEMP_DEG_MIN    30
#define TEMP_DEG_MAX    85

#define TEMP_DEG_OFFSET    280

#define LM335_SHORTED    200

#endif    // TEMP_SENSOR_LM335


// Module Exported Functions ---------------------------------------------------
unsigned char Temp_TempToDegreesExtended (unsigned short temp);


#endif    /* _TEMPERATURES_H_ */

//--- end of file ---//
