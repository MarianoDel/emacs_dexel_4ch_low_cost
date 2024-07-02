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
void FiltersAndOffsets_Channels_to_Backup (unsigned char * channels);
void FiltersAndOffsets_Enable_Outputs (void);
void FiltersAndOffsets_Disable_Outputs (void);

void FiltersAndOffsets_Calc_SM (volatile unsigned char * ch_dmx_val);
void FiltersAndOffsets_Filters_Reset (void);
void FiltersAndOffsets_Channels_Reset (void);

#endif    /* _FILTERS_AND_OFFSETS_H_ */

//--- end of file ---//
