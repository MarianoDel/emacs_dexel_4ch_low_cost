//----------------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### FILTERS_AND_OFFSETS.C ########################
//----------------------------------------------------

// Includes --------------------------------------------------------------------
#include "filters_and_offsets.h"
#include "dsp.h"
#include "parameters.h"
#include "pwm.h"
#include "tim.h"


// #define FILTER_MA32
#define FILTER_MA16
// #define FILTER_MA8
// Externals -------------------------------------------------------------------
extern parameters_typedef mem_conf;

extern unsigned short ch1_pwm;
extern unsigned short ch2_pwm;
extern unsigned short ch3_pwm;
extern unsigned short ch4_pwm;

extern volatile unsigned char dmx_filters_timer;



// Globals ---------------------------------------------------------------------
#ifdef FILTER_MA16
ma16_u16_data_obj_t st_sp1;
ma16_u16_data_obj_t st_sp2;
ma16_u16_data_obj_t st_sp3;
ma16_u16_data_obj_t st_sp4;
#endif
#ifdef FILTER_MA32
ma32_u16_data_obj_t st_sp1;
ma32_u16_data_obj_t st_sp2;
ma32_u16_data_obj_t st_sp3;
ma32_u16_data_obj_t st_sp4;
#endif
#ifdef FILTER_MA8
ma8_u16_data_obj_t st_sp1;
ma8_u16_data_obj_t st_sp2;
ma8_u16_data_obj_t st_sp3;
ma8_u16_data_obj_t st_sp4;
#endif

volatile unsigned char ch_bkp_val [4] = { 0 };
volatile unsigned short limit_output [4] = { 0 };

volatile unsigned char filters_enable_outputs = 0;
// Module Private Types & Macros -----------------------------------------------


// Module Private Functions ----------------------------------------------------


// Module Functions ------------------------------------------------------------
void FiltersAndOffsets_Channels_to_Backup (unsigned char * channels)
{
    // __disable_irq();
    *(ch_bkp_val + 0) = *(channels + 0);
    *(ch_bkp_val + 1) = *(channels + 1);
    *(ch_bkp_val + 2) = *(channels + 2);
    *(ch_bkp_val + 3) = *(channels + 3);
    // __enable_irq();
}


void FiltersAndOffsets_Enable_Outputs (void)
{
    filters_enable_outputs = 1;
}


void FiltersAndOffsets_Disable_Outputs (void)
{
    filters_enable_outputs = 0;    
}


typedef enum {
    FILTERS_BKP_CHANNELS,
    FILTERS_LIMIT_EACH_CHANNEL,
    FILTERS_LIMIT_ALL_CHANNELS,
    FILTERS_OUTPUTS_CH1_CH2,
    FILTERS_OUTPUTS_CH3_CH4
    
} filters_and_offsets_e;

filters_and_offsets_e filters_sm = FILTERS_BKP_CHANNELS;
void FiltersAndOffsets_Calc_SM (void)
{
    unsigned short calc = 0;
    unsigned short ch_pwm = 0;

    if (!filters_enable_outputs)
        return;
    
    switch (filters_sm)
    {
    case FILTERS_BKP_CHANNELS:
        limit_output[0] = *(ch_bkp_val + 0);
        limit_output[1] = *(ch_bkp_val + 1);
        limit_output[2] = *(ch_bkp_val + 2);
        limit_output[3] = *(ch_bkp_val + 3);
        filters_sm++;
        break;

    case FILTERS_LIMIT_EACH_CHANNEL:
        calc = limit_output[0] * mem_conf.max_current_channels[0];
        limit_output[0] = (unsigned short) calc;

        calc = limit_output[1] * mem_conf.max_current_channels[1];
        limit_output[1] = (unsigned short) calc;

        calc = limit_output[2] * mem_conf.max_current_channels[2];
        limit_output[2] = (unsigned short) calc;

        calc = limit_output[3] * mem_conf.max_current_channels[3];
        limit_output[3] = (unsigned short) calc;
        
        filters_sm++;
        // filters_sm = FILTERS_OUTPUTS_CH1_CH2;        
        break;

    case FILTERS_LIMIT_ALL_CHANNELS:
        // PWM_Set_PwrCtrl(limit_output,
        //                 mem_conf.dmx_channel_quantity,
        //                 mem_conf.max_power);
        filters_sm++;
        break;

    case FILTERS_OUTPUTS_CH1_CH2:
#ifdef FILTER_MA8
        // channel 1
        ch_pwm = MA8_U16Circular (
            &st_sp1,
            PWM_Map_From_Dmx_Short(*(limit_output + 0))
            );
        PWM_Update_CH1(ch_pwm);

        // channel 2
        ch_pwm = MA8_U16Circular (
            &st_sp2,
            PWM_Map_From_Dmx_Short(*(limit_output + 1))
            );
        PWM_Update_CH2(ch_pwm);
#endif
#ifdef FILTER_MA16
        // channel 1
        ch_pwm = MA16_U16Circular (
            &st_sp1,
            PWM_Map_From_Dmx_Short(*(limit_output + 0))
            );
        PWM_Update_CH1(ch_pwm);

        // channel 2
        ch_pwm = MA16_U16Circular (
            &st_sp2,
            PWM_Map_From_Dmx_Short(*(limit_output + 1))
            );
        PWM_Update_CH2(ch_pwm);
#endif
#ifdef FILTER_MA32
        // channel 1
        ch_pwm = MA32_U16Circular (
            &st_sp1,
            PWM_Map_From_Dmx_Short(*(limit_output + 0))
            );
        PWM_Update_CH1(ch_pwm);

        // channel 2
        ch_pwm = MA32_U16Circular (
            &st_sp2,
            PWM_Map_From_Dmx_Short(*(limit_output + 1))
            );
        PWM_Update_CH2(ch_pwm);
#endif
        filters_sm++;
        break;

    case FILTERS_OUTPUTS_CH3_CH4:
#ifdef FILTER_MA8
        // channel 3
        ch_pwm = MA8_U16Circular (
            &st_sp3,
            PWM_Map_From_Dmx_Short(*(limit_output + 2))
            );
        PWM_Update_CH3(ch_pwm);

        // channel 4
        ch_pwm = MA8_U16Circular (
            &st_sp4,
            PWM_Map_From_Dmx_Short(*(limit_output + 3))
            );
        PWM_Update_CH4(ch_pwm);
#endif
#ifdef FILTER_MA16
        // channel 3
        ch_pwm = MA16_U16Circular (
            &st_sp3,
            PWM_Map_From_Dmx_Short(*(limit_output + 2))
            );
        PWM_Update_CH3(ch_pwm);

        // channel 4
        ch_pwm = MA16_U16Circular (
            &st_sp4,
            PWM_Map_From_Dmx_Short(*(limit_output + 3))
            );
        PWM_Update_CH4(ch_pwm);
#endif
#ifdef FILTER_MA32
        // channel 3
        ch_pwm = MA32_U16Circular (
            &st_sp3,
            PWM_Map_From_Dmx_Short(*(limit_output + 2))
            );
        PWM_Update_CH3(ch_pwm);

        // channel 4
        ch_pwm = MA32_U16Circular (
            &st_sp4,
            PWM_Map_From_Dmx_Short(*(limit_output + 3))
            );
        PWM_Update_CH4(ch_pwm);
#endif
        filters_sm = FILTERS_BKP_CHANNELS;
        break;
        
    default:
        filters_sm = FILTERS_BKP_CHANNELS;
        break;
    }
}


void FiltersAndOffsets_Filters_Reset (void)
{
#ifdef FILTER_MA8
    MA8_U16Circular_Reset(&st_sp1);
    MA8_U16Circular_Reset(&st_sp2);
    MA8_U16Circular_Reset(&st_sp3);
    MA8_U16Circular_Reset(&st_sp4);
#endif
#ifdef FILTER_MA16
    MA16_U16Circular_Reset(&st_sp1);
    MA16_U16Circular_Reset(&st_sp2);
    MA16_U16Circular_Reset(&st_sp3);
    MA16_U16Circular_Reset(&st_sp4);
#endif
#ifdef FILTER_MA32
    MA32_U16Circular_Reset(&st_sp1);
    MA32_U16Circular_Reset(&st_sp2);
    MA32_U16Circular_Reset(&st_sp3);
    MA32_U16Circular_Reset(&st_sp4);
#endif
}


void FiltersAndOffsets_Channels_Reset (void)
{
    for (int i = 0; i < 4; i++)
    {
        ch_bkp_val [i] = 0;
        limit_output [i] = 0;
        // pwm_chnls [i] = 0;
        // dac_chnls [i] = 0;
    }

    PWM_Update_CH1(0);
    PWM_Update_CH2(0);
    PWM_Update_CH3(0);
    PWM_Update_CH4(0);    
}


//--- end of file ---//
