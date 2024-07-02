//-------------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### FILTERS_AND_OFFSETS.H #####################
//-------------------------------------------------

// Prevent recursive inclusion -------------------------------------------------
#ifndef _FILTERS_AND_OFFSETS_H_
#define _FILTERS_AND_OFFSETS_H_


// Exported Types Constants and Macros -----------------------------------------


// Module Exported Functions ---------------------------------------------------
void FiltersAndOffsets_Calc (unsigned char * ch_dmx_val);
void FiltersAndOffsets_Calc_SM (volatile unsigned char * ch_dmx_val);
void FiltersAndOffsets_Calc_NoTimed (volatile unsigned char * ch_dmx_val, unsigned char which_channel);
void FiltersAndOffsets_Filters_Reset (void);


#endif    /* _FILTERS_AND_OFFSETS_H_ */

//--- end of file ---//
