//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### PARAMETERS.H ##########################
//---------------------------------------------

#ifndef _PARAMETERS_H_
#define _PARAMETERS_H_

//-- Configuration for Struct --------------------
typedef enum {
    // program types (modes)
    DMX_MODE = 0,
    MANUAL_MODE,
    AUTODETECT_MODE
    
} programs_type_e;


typedef enum {

    CCT1_MODE = 0,
    CCT2_MODE,
    ONECH_MODE
    
} channels_operation_mode_e;


//-- Memory Struct to Save --------------------
// Beware this struct must be 4 bytes align and be multiple of 8
typedef struct parameters {

    //-- Program Type ----
    unsigned char program_type;

    //-- DMX mode configs. ----                  
    unsigned char dmx_channel_quantity;
    unsigned char dummy1 [2];
    unsigned short dmx_first_channel;    // 2 bytes if its aligned to 4

    //-- For Manual mode settings ----   //6
    unsigned char fixed_channels [4];

    //-- For Max Current Channels settings ----   //10
    unsigned char max_current_channels [4];
    unsigned char manual_inner_mode;
    unsigned char manual_inner_speed;
    
    //-- For Temperature Protection ----    //16
    unsigned short temp_prot;    // 2 bytes if its aligned to 4
    unsigned char temp_prot_deg;
    unsigned char dummy3 [5];
    // unsigned char dummy2;          
    // unsigned char dummy3;       

    //-- End of Struct check for 8 bytes alignment ---- //24

} parameters_typedef;

//-- End of Memory Struct to Save --------------------

#endif    /* _PARAMETERS_H_ */

//--- end of file ---//

