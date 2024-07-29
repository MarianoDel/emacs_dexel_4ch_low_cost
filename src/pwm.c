//----------------------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### PWM.C ##############################################
//----------------------------------------------------------

// Includes --------------------------------------------------------------------
#include "pwm.h"
#include "hard.h"
#include "tim.h"


// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------


// Module Private Types & Macros -----------------------------------------------


// Module Private Functions ----------------------------------------------------


// Module Functions ------------------------------------------------------------
void PWMChannelsReset (void)
{
    PWM_Update_CH1(0);
    PWM_Update_CH2(0);
    PWM_Update_CH3(0);
    PWM_Update_CH4(0);
}


// 255 * 255 or 255 * 128
unsigned short PWM_Map_From_Dmx_Short (unsigned short adj_val)
{
    unsigned int pwm = 0;

    if (adj_val)
    {
        pwm = adj_val >> 4;
        // if (pwm < 29)    // original
        //     pwm = 29;
        
        if (pwm < 38)
            pwm = 38;
    }

    if (pwm > 4095)
        pwm = 4095;

    return (unsigned short) pwm;

}


unsigned short PWM_Map_From_Dmx (unsigned char dmx_val)
{
    unsigned int pwm = 0;

#if (DUTY_100_PERCENT == 1000)    
    if (dmx_val)
    {
        pwm = dmx_val * 391;
        pwm = pwm / 100;
        pwm += 4;
    }
#elif (DUTY_100_PERCENT == 2000)
    if (dmx_val)
    {
        pwm = dmx_val * 78;
        pwm = pwm / 10;
        pwm += 11;
    }
#elif (DUTY_100_PERCENT == 3000)
    if (dmx_val)
    {
        pwm = dmx_val * 117;
        pwm = pwm / 10;
        pwm += 18;
    }
#elif (DUTY_100_PERCENT == 4000)
    if (dmx_val)
    {
        pwm = dmx_val * 157;
        pwm = pwm / 10;
        pwm += 13;
    }
#else
#error "no duty selected in pwm.c from hard.h"
#endif

    return (unsigned short) pwm;

}


void PWM_Set_PwrCtrl (unsigned char * ch_dmx_val, unsigned char chnls_qtty, unsigned short max_power)
{
    unsigned short total_dmx = 0;

    //cuantos en total
    for (unsigned char i = 0; i < chnls_qtty; i++)
        total_dmx += *(ch_dmx_val + i);

    if (total_dmx > max_power)
    {
        unsigned int new = 0;
        for (unsigned char i = 0; i < chnls_qtty; i++)
        {
            // si el canal tiene algo
            if (*(ch_dmx_val + i))
            {
                new = *(ch_dmx_val + i) * max_power;
                new = new / total_dmx;

                // no dejo que se apaguen los canales
                if (new)
                    *(ch_dmx_val + i) = (unsigned char) new;
                else
                    *(ch_dmx_val + i) = 1;
                
            }
        }
    }
}

//--- end of file ---//
