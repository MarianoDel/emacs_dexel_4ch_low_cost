//-------------------------------------------------
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    TEST PLATFORM FOR FIRMWARE
// ##
// #### TESTS_PWM_POST_MAP.C ######################
//-------------------------------------------------

// Includes Modules for tests --------------------------------------------------
#include "pwm.h"
#include "dsp.h"
// #include "filters_and_offsets.h"
#include "parameters.h"


// Includes tests helper modules for the tests ---------------------------------
#include "tests_vector_utils.h"


#include <stdio.h>
#include <math.h>
#include <stdint.h>


// Types Constants and Macros --------------------------------------------------
// Select simulation lenght in mseconds
#define SIMULATION_MS    2000

#define VLENGTH    256
// #define VECTOR_LENGTH    ((int)(SAMPLING_FREQ * SIMULATION_LENGTH))


// Externals -- Globals on module to test --------------------------------------
// extern unsigned char filters_enable_outputs;


// Externals -------------------------------------------------------------------
volatile unsigned char pwm_chnls [6];
volatile unsigned short dac_chnls [6];



// Globals ---------------------------------------------------------------------
// unsigned short dmx_data_ch1 [VECTOR_LENGTH] = { 0 };
// unsigned short dmx_data_ch2 [VECTOR_LENGTH] = { 0 };

// unsigned short ena_output_ch1 [VECTOR_LENGTH] = { 0 };
// unsigned short pwm_output_ch1 [VECTOR_LENGTH] = { 0 };

// unsigned short ena_output_ch2 [VECTOR_LENGTH] = { 0 };
// unsigned short pwm_output_ch2 [VECTOR_LENGTH] = { 0 };

parameters_typedef mem_conf;

// Tests Functions -------------------------------------------------------------
void Test_PWM_Map_From_Dmx_Short_Mapping (void);
void Test_Dsp_ma32 (void);
// void Test_PWM_Post_Mapping (void);
// void Test_PWM_Power_Ctrl (void);
// void Test_Dmx_Post_Step (void);
// void Test_Dmx_Post_Step_Dowm (void);
// void Test_Dmx_Post_Ramp_Ch1 (void);
// void Test_Dmx_Post_Ramp_Inner_Ch1 (void);
// void Test_Dmx_Post_Filter (void);


// Main Function to Test -------------------------------------------------------


// Module Mocked Functions -----------------------------------------------------
void TIM6_Update (void);
void DAC_MUX_Update_by_Int (void);


// Module Functions ------------------------------------------------------------
int main (int argc, char *argv[])
{
    // new tests
    // Test_PWM_Map_From_Dmx_Short_Mapping ();
    Test_Dsp_ma32 (); 
    // Test_Dmx_Post_Ramp_Ch1 ();
    // Test_Dmx_Post_Ramp_Inner_Ch1 ();
    // Test_Dmx_Post_Filter ();

    // old tests
    // Test_PWM_Power_Ctrl ();
    // Test_Dmx_Post_Step_Dowm ();    

    return 0;
}


void Test_PWM_Map_From_Dmx_Short_Mapping (void)
{
    printf("\nTest pwm post-mapping\n from 0 to 4095");

    unsigned short ch_dmx [VLENGTH] = { 0 };
    unsigned short ch_pwm [VLENGTH] = { 0 };
    unsigned short ch_data [VLENGTH] = { 0 };
    // int calc = 0;
    
    for (int i = 0; i < (VLENGTH - 1); i++)
    {
        ch_dmx[i] = i;
        ch_data [i] = ch_dmx[i] * 212;
        ch_pwm [i] = PWM_Map_From_Dmx_Short (ch_data[i]);        
    }

    // ShowVectorUShort("\nVector data input:\n", to_map, 600);
    // ShowVectorUShort("\nVector ena pwm:\n", vena, 600);    
    // ShowVectorUShort("\nVector dac out:\n", vdac, 600);

    ///////////////////////////
    // Backup Data to a file //
    ///////////////////////////
    FILE * file = fopen("data.txt", "w");

    if (file == NULL)
    {
        printf("data file not created!\n");
        return;
    }

    Vector_UShort_To_File (file, "ch_dmx", ch_dmx, VLENGTH);
    // Vector_UShort_To_File (file, "ch_data", ch_data, VLENGTH);    
    Vector_UShort_To_File (file, "ch_pwm", ch_pwm, VLENGTH);    
    // Vector_UShort_To_File (file, "ch1_dac", ch1_dac, 512);
    // Vector_UShort_To_File (file, "ch1_eq_pwm", ch1_eq_pwm, 512);    

    printf("\nRun by hand python3 simul_outputs.py\n");

}


void Test_Dsp_ma32 (void)
{
    printf("\nTest pwm post-mapping\n from 0 to 4095");

    unsigned short ch_input [VLENGTH] = { 0 };
    unsigned short ch_output [VLENGTH] = { 0 };

    ma32_u16_data_obj_t filter32;
    MA32_U16Circular_Reset (&filter32);
    
    for (int i = 0; i < (VLENGTH/2); i++)
    {
        ch_input[i] = 65535;
        ch_output[i] = MA32_U16Circular (&filter32, ch_input[i]);
    }

    for (int i = (VLENGTH/2); i < (VLENGTH - 1); i++)
    {
        ch_input[i] = 0;
        ch_output[i] = MA32_U16Circular (&filter32, ch_input[i]);
    }
    
    ///////////////////////////
    // Backup Data to a file //
    ///////////////////////////
    FILE * file = fopen("data.txt", "w");

    if (file == NULL)
    {
        printf("data file not created!\n");
        return;
    }

    Vector_UShort_To_File (file, "ch_input", ch_input, VLENGTH);
    Vector_UShort_To_File (file, "ch_output", ch_output, VLENGTH);    

    printf("\nRun by hand python3 simul_outputs.py\n");
    
}
// void Test_Dmx_Post_Ramp_Ch1 (void)
// {
//     unsigned char chnls [6] = { 0 };
//     unsigned char dmx_data_ch1 [VECTOR_LENGTH] = { 0 };

//     printf("\nTest dmx Ramp with post-mapping\n");
    
//     filters_enable_outputs = 1;

//     mem_conf.max_current_channels[0] = 255;
//     mem_conf.dmx_channel_quantity = 6;
//     mem_conf.max_power = 255 * 6;
//     mem_conf.program_inner_type = MANUAL_INNER_FIXED_MODE;    //anyone except DMX2_INNER_STROBE_MODE
    
//     // ramp 0 to 255
//     for (int i = 0; i < VECTOR_LENGTH; i++)
//     {
//         if ((i % 5) == 0)
//         {
//             // new value
//         }
//         if (i <= 255)
//             dmx_data_ch1 [i] = i;
//         else
//             dmx_data_ch1 [i] = 255;
//     }

//     FiltersAndOffsets_Filters_Reset ();

//     for (int i = 0; i < VECTOR_LENGTH; i++)
//     {
//         if ((i % 5) == 0)
//             FiltersAndOffsets_Channels_to_Backup ();
    

//         FiltersAndOffsets_Calc_SM (&mem_conf);
//     }

//     ShowVectorUShort("\nVector dmx data ch1:\n", dmx_data_ch1, 100);
//     ShowVectorUShort("\nVector dmx data ch2:\n", dmx_data_ch2, 100);    
//     ShowVectorUShort("\nVector ena outp ch1:\n", ena_output_ch1, 100);
//     ShowVectorUShort("\nVector pwm outp ch1:\n", pwm_output_ch1, 100);    
//     ShowVectorUShort("\nVector ena outp ch2:\n", ena_output_ch2, 100);
//     ShowVectorUShort("\nVector pwm outp ch2:\n", pwm_output_ch2, 100);    

//     ///////////////////////////
//     // Backup Data to a file //
//     ///////////////////////////
//     FILE * file = fopen("data.txt", "w");

//     if (file == NULL)
//     {
//         printf("data file not created!\n");
//         return;
//     }

//     Vector_UShort_To_File (file, "dmx_ch1", dmx_data_ch1, VECTOR_LENGTH);
//     Vector_UShort_To_File (file, "dmx_ch2", dmx_data_ch2, VECTOR_LENGTH);    
//     Vector_UShort_To_File (file, "ena_ch1", ena_output_ch1, VECTOR_LENGTH);
//     Vector_UShort_To_File (file, "pwm_ch1", pwm_output_ch1, VECTOR_LENGTH);    
//     Vector_UShort_To_File (file, "ena_ch2", ena_output_ch2, VECTOR_LENGTH);
//     Vector_UShort_To_File (file, "pwm_ch2", pwm_output_ch2, VECTOR_LENGTH);    

//     printf("\nRun by hand python3 simul_outputs.py\n");

// }


// void Test_Dmx_Post_Ramp_Inner_Ch1 (void)
// {
//     unsigned char chnls [6] = { 0 };
//     unsigned short limit_output [6] = { 0 };    
//     unsigned char pwm_chnls [6] = { 0 };
//     unsigned short dac_chnls [6] = { 0 };
//     unsigned char dmx_data_ch1 [255] = { 0 };
    
//     unsigned short v_dmx_ch1 [255] = { 0 };
//     unsigned short v_pwm_ch1 [255] = { 0 };
//     unsigned short v_dac_ch1 [255] = { 0 };
//     unsigned short v_pwm_eq_ch1 [255] = { 0 };    
    
//     int calc = 0;

//     printf("\nTest dmx Filters_and_Offsets Internals with post-mapping\n");
    
//     filters_enable_outputs = 1;

//     mem_conf.max_current_channels[0] = 255;
//     mem_conf.dmx_channel_quantity = 6;
//     mem_conf.max_power = 255 * 6;
//     mem_conf.program_inner_type = MANUAL_INNER_FIXED_MODE;    //anyone except DMX2_INNER_STROBE_MODE
    
//     // ramp 0 to 255
//     for (int i = 0; i < 256; i++)
//     {
//         dmx_data_ch1 [i] = i;
//     }

//     for (int i = 0; i < 256; i++)
//     {
//         //case FILTERS_LIMIT_EACH_CHANNEL:
//         calc = dmx_data_ch1[i] * mem_conf.max_current_channels[0];
//         calc >>= 7;
//         limit_output[0] = (unsigned short) calc;

//         // case FILTERS_LIMIT_ALL_CHANNELS:    // 0 to 511
//         PWM_Set_PwrCtrl_512 (limit_output,
//                              mem_conf.dmx_channel_quantity,
//                              mem_conf.max_power);

//         // case FILTERS_OUTPUTS_CH1_CH3:
//         PWM_Map_Post_Filter (
//             *(limit_output + 0) << 3,
//             (unsigned char *)&pwm_chnls[0],
//             (unsigned short *)&dac_chnls[0]);

//         v_dmx_ch1[i] = dmx_data_ch1[i];
//         v_pwm_ch1[i] = pwm_chnls[0];
//         v_dac_ch1[i] = dac_chnls[0];

//         calc = pwm_chnls[0] * dac_chnls[0];
//         calc = calc >> 8;
//         v_pwm_eq_ch1[i] = (unsigned short) calc;
//     }

//     ShowVectorUShort("\nVector v_dmx_ch1:\n", v_dmx_ch1, 255);
//     ShowVectorUShort("\nVector v_pwm_ch1:\n", v_pwm_ch1, 255);
//     ShowVectorUShort("\nVector v_dac_ch1:\n", v_dac_ch1, 255);
//     ShowVectorUShort("\nVector v_pwm_eq_ch1:\n", v_pwm_eq_ch1, 255);

//     ///////////////////////////
//     // Backup Data to a file //
//     ///////////////////////////
//     FILE * file = fopen("data.txt", "w");

//     if (file == NULL)
//     {
//         printf("data file not created!\n");
//         return;
//     }

//     Vector_UShort_To_File (file, "dmx_ch1", v_dmx_ch1, 255);
//     Vector_UShort_To_File (file, "pwm_ch1", v_pwm_ch1, 255);    
//     Vector_UShort_To_File (file, "dac_ch1", v_dac_ch1, 255);
//     Vector_UShort_To_File (file, "pwm_eq_ch1", v_pwm_eq_ch1, 255);    

//     printf("\nRun by hand python3 simul_outputs.py\n");

// }


// void Test_Dmx_Post_Filter (void)
// {
//     unsigned short v_dmx_ch1 [255] = { 0 };
//     unsigned short v_pwm_ch1 [255] = { 0 };
//     unsigned short v_dac_ch1 [255] = { 0 };
//     unsigned short v_pwm_eq_ch1 [255] = { 0 };    
    
//     int calc = 0;

//     printf("\nTest PWM_Map_Post_Filter from Dmx data\n");
    
//     filters_enable_outputs = 1;

//     for (int i = 0; i < 256; i++)
//     {
//         calc = i * 16;
//         PWM_Map_Post_Filter (
//             (unsigned short) calc,
//             (unsigned char *)&pwm_chnls[0],
//             (unsigned short *)&dac_chnls[0]);

//         v_dmx_ch1[i] = i;
//         v_pwm_ch1[i] = pwm_chnls[0];
//         v_dac_ch1[i] = dac_chnls[0];

//         calc = pwm_chnls[0] * dac_chnls[0];
//         calc = calc >> 8;
//         v_pwm_eq_ch1[i] = (unsigned short) calc;
//     }

//     ShowVectorUShort("\nVector v_dmx_ch1:\n", v_dmx_ch1, 255);
//     ShowVectorUShort("\nVector v_pwm_ch1:\n", v_pwm_ch1, 255);
//     ShowVectorUShort("\nVector v_dac_ch1:\n", v_dac_ch1, 255);
//     ShowVectorUShort("\nVector v_pwm_eq_ch1:\n", v_pwm_eq_ch1, 255);

//     ///////////////////////////
//     // Backup Data to a file //
//     ///////////////////////////
//     FILE * file = fopen("data.txt", "w");

//     if (file == NULL)
//     {
//         printf("data file not created!\n");
//         return;
//     }

//     Vector_UShort_To_File (file, "dmx_ch1", v_dmx_ch1, 255);
//     Vector_UShort_To_File (file, "pwm_ch1", v_pwm_ch1, 255);    
//     Vector_UShort_To_File (file, "dac_ch1", v_dac_ch1, 255);
//     Vector_UShort_To_File (file, "pwm_eq_ch1", v_pwm_eq_ch1, 255);    

//     printf("\nRun by hand python3 simul_outputs.py\n");

// }


// void Test_PWM_Power_Ctrl (void)
// {
//     printf("\nTest pwm power control\n");

//     unsigned short chnls [6] = { 0 };
    
//     unsigned short vch1_orig [512] = { 0 };
//     unsigned short vch2_orig [512] = { 0 };
//     unsigned short vch3_orig [512] = { 0 };
//     unsigned short vch4_orig [512] = { 0 };
//     unsigned short vch5_orig [512] = { 0 };
//     unsigned short vch6_orig [512] = { 0 };

//     unsigned short vch1_roof [512] = { 0 };
//     unsigned short vch2_roof [512] = { 0 };
//     unsigned short vch3_roof [512] = { 0 };
//     unsigned short vch4_roof [512] = { 0 };
//     unsigned short vch5_roof [512] = { 0 };
//     unsigned short vch6_roof [512] = { 0 };
    
//     for (int i = 0; i < 512; i++)
//     {
//         chnls[0] = i;
//         chnls[1] = i;
//         chnls[2] = i;
//         chnls[3] = i;
//         chnls[4] = i;
//         chnls[5] = i;

//         vch1_orig [i] = chnls[0];
//         vch2_orig [i] = chnls[1];
//         vch3_orig [i] = chnls[2];
//         vch4_orig [i] = chnls[3];
//         vch5_orig [i] = chnls[4];
//         vch6_orig [i] = chnls[5];        
        
//         // PWM_Set_PwrCtrl_512 (chnls, 6, 255);
//         PWM_Set_PwrCtrl_512 (chnls, 6, 1530);
//         // PWM_Set_PwrCtrl_512 (chnls, 6, 765);
//         // PWM_Set_PwrCtrl (chnls, 6, 255);        

//         vch1_roof [i] = chnls[0];
//         vch2_roof [i] = chnls[1];
//         vch3_roof [i] = chnls[2];
//         vch4_roof [i] = chnls[3];
//         vch5_roof [i] = chnls[4];
//         vch6_roof [i] = chnls[5];        
//     }
    
//     ///////////////////////////
//     // Backup Data to a file //
//     ///////////////////////////
//     FILE * file = fopen("data.txt", "w");

//     if (file == NULL)
//     {
//         printf("data file not created!\n");
//         return;
//     }

//     Vector_UShort_To_File (file, "ch1_root", vch1_roof, 512);
//     Vector_UShort_To_File (file, "ch1_orig", vch1_orig, 512);
//     Vector_UShort_To_File (file, "ch2", vch2_roof, 512);
//     Vector_UShort_To_File (file, "ch3", vch3_roof, 512);    
//     Vector_UShort_To_File (file, "ch4", vch4_roof, 512);
//     Vector_UShort_To_File (file, "ch5", vch5_roof, 512);
//     Vector_UShort_To_File (file, "ch6", vch6_roof, 512);
//     printf("\nRun by hand python3 simul_outputs.py\n");
    
// }


// Module Mocked Functions -----------------------------------------------------
void Update_TIM3_CH1 (unsigned short a)
{
}

void Update_TIM3_CH2 (unsigned short a)
{
}

void Update_TIM3_CH3 (unsigned short a)
{
}

void Update_TIM3_CH4 (unsigned short a)
{
}


// void TIM_Deactivate_Channels (unsigned char deact_chnls)
// {
//     printf("deactivate: 0x%02x\n", deact_chnls);
// }


// void TIM_Activate_Channels (unsigned char act_chnls)
// {
//     printf("activate: 0x%02x\n", act_chnls);
// }

// void TIM6_Update (void)
// {
// }


// void DAC_MUX_Update_by_Int (void)
// {
// }

//--- end of file ---//


