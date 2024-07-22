//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### COLORS_FUNCTIONS.C ####################
//---------------------------------------------

#include "colors_functions.h"
// #include "parameters.h"


// Module Private Types Constants and Macros -----------------------------------
#define CH1_EFFECT    0x01
#define CH2_EFFECT    0x02
#define CH3_EFFECT    0x04
#define CH4_EFFECT    0x08


// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------

#define FADING_RISING    0
#define FADING_FALLING    1
unsigned char fading_state = FADING_RISING;
unsigned char fading_step = 0;
resp_t Colors_Fading (unsigned char * ch_val, unsigned char fade_ch)
{
    resp_t resp = resp_continue;
    unsigned char how_many_channels = 0;    
    unsigned char calc;

    switch (fading_state)
    {
    case FADING_RISING:
        if (fading_step < 255)
            fading_step++;
        else
            fading_state = FADING_FALLING;

        break;

    case FADING_FALLING:
        if (fading_step)
            fading_step--;
        else
        {
            fading_state = FADING_RISING;
            resp = resp_finish;
        }
        break;

    default:
        fading_state = FADING_RISING;
        break;
    }


    /////////////////////////
    // Power Color Control //
    /////////////////////////
    for (unsigned char i = 0; i < 8; i++)
    {
        calc = 1;
        calc <<= i;
        if (calc & fade_ch)
            how_many_channels++;
    }

    if (!how_many_channels)
        return resp_finish;

    calc = fading_step / how_many_channels;

    if (fade_ch & CH1_EFFECT)
        *(ch_val + 0) = calc;

    if (fade_ch & CH2_EFFECT)
        *(ch_val + 1) = calc;

    if (fade_ch & CH3_EFFECT)
        *(ch_val + 2) = calc;

    if (fade_ch & CH4_EFFECT)
        *(ch_val + 3) = calc;

    return resp;
}


resp_t Colors_Fading_Shuffle (unsigned char * ch_val,
                              unsigned char fade_ch_up,
                              unsigned char fade_ch_dwn)
{
    resp_t resp = resp_continue;
    unsigned char calc_up;
    unsigned char calc_dwn;
    unsigned char how_many_channels_up = 0;    
    unsigned char how_many_channels_dwn = 0;    

    if (fading_step < 255)
        fading_step++;
    else
    {
        fading_step = 0;
        return resp_finish;
    }

    /////////////////////////
    // Power Color Control //
    /////////////////////////
    for (unsigned char i = 0; i < 8; i++)
    {
        calc_up = 1;
        calc_up <<= i;
        if (calc_up & fade_ch_up)
            how_many_channels_up++;

        if (calc_up & fade_ch_dwn)
            how_many_channels_dwn++;
    }

    if ((!how_many_channels_up) || (!how_many_channels_dwn))
        return resp_finish;

    
    calc_up = fading_step / how_many_channels_up;
    calc_dwn = (255 - fading_step) / how_many_channels_dwn;
    
    unsigned char channel1 = 0;
    unsigned char channel2 = 0;
    unsigned char channel3 = 0;
    unsigned char channel4 = 0;

    
    ///////////////////////
    // Channels going up //
    ///////////////////////
    if (fade_ch_up & CH1_EFFECT)
        channel1 = calc_up;

    if (fade_ch_up & CH2_EFFECT)
        channel2 = calc_up;

    if (fade_ch_up & CH3_EFFECT)
        channel3 = calc_up;

    if (fade_ch_up & CH4_EFFECT)
        channel4 = calc_up;

    /////////////////////////
    // Channels going down //
    /////////////////////////
    if (fade_ch_dwn & CH1_EFFECT)
    {
        if (channel1 < calc_dwn)
            channel1 = calc_dwn;
    }

    if (fade_ch_dwn & CH2_EFFECT)
    {
        if (channel2 < calc_dwn)
            channel2 = calc_dwn;
    }

    if (fade_ch_dwn & CH3_EFFECT)
    {
        if (channel3 < calc_dwn)
            channel3 = calc_dwn;
    }

    if (fade_ch_dwn & CH4_EFFECT)
    {
        if (channel4 < calc_dwn)
            channel4 = calc_dwn;
    }

    *(ch_val + 0) = channel1;
    *(ch_val + 1) = channel2;
    *(ch_val + 2) = channel3;
    *(ch_val + 3) = channel4;

    return resp;
}

// variables re-use
#define strobe_state    fading_state
#define STROBE_IN_ON    0
#define STROBE_IN_OFF    1
resp_t Colors_Strobe (unsigned char * ch_val, unsigned char strobe_ch)
{
    resp_t resp = resp_continue;
    unsigned char how_many_channels = 0;
    unsigned char calc;

    switch (strobe_state)
    {
    case STROBE_IN_ON:
        for (unsigned char i = 0; i < 8; i++)
        {
            calc = 1;
            calc <<= i;
            if (calc & strobe_ch)
                how_many_channels++;
        }

        if (!how_many_channels)
            break;

        calc = 255 / how_many_channels;
        strobe_state = STROBE_IN_OFF;
        break;

    case STROBE_IN_OFF:
        calc = 0;
        strobe_state = STROBE_IN_ON;
        resp = resp_finish;
        break;

    default:
        strobe_state = STROBE_IN_ON;
        break;
    }

    if (strobe_ch & CH1_EFFECT)
        *(ch_val + 0) = calc;

    if (strobe_ch & CH2_EFFECT)
        *(ch_val + 1) = calc;

    if (strobe_ch & CH3_EFFECT)
        *(ch_val + 2) = calc;

    if (strobe_ch & CH4_EFFECT)
        *(ch_val + 3) = calc;

    return resp;
}


typedef enum {
    RED_COLOR = 0,
    GREEN_COLOR,
    BLUE_COLOR,
    WHITE_COLOR,
    PURPLE_COLOR,
    YELLOW_COLOR,
    CYAN_COLOR


} fading_pallete_colors_t;

#define RED_FLAG    0x01
#define GREEN_FLAG    0x02
#define BLUE_FLAG    0x04
#define PURPLE_FLAG    (RED_FLAG | BLUE_FLAG)
#define YELLOW_FLAG    (RED_FLAG | GREEN_FLAG)
#define CYAN_FLAG    (GREEN_FLAG | BLUE_FLAG)
#define WHITE_FLAG    (RED_FLAG | GREEN_FLAG | BLUE_FLAG)

fading_pallete_colors_t which_color = 0;
resp_t Colors_Fading_Pallete (unsigned char * ch_val)
{
    resp_t resp = resp_continue;

    switch (which_color)
    {
    case RED_COLOR:
        resp = Colors_Fading(ch_val, RED_FLAG);
        if (resp == resp_finish)
        {
            which_color++;
            resp = resp_continue;
        }
        break;

    case GREEN_COLOR:
        resp = Colors_Fading(ch_val, GREEN_FLAG);
        if (resp == resp_finish)
        {
            which_color++;
            resp = resp_continue;
        }
        break;

    case BLUE_COLOR:
        resp = Colors_Fading(ch_val, BLUE_FLAG);
        if (resp == resp_finish)
        {
            which_color++;
            resp = resp_continue;
        }
        break;

    case WHITE_COLOR:
        resp = Colors_Fading(ch_val, WHITE_FLAG);
        if (resp == resp_finish)
        {
            which_color++;
            resp = resp_continue;
        }
        break;

    case PURPLE_COLOR:
        resp = Colors_Fading(ch_val, PURPLE_FLAG);
        if (resp == resp_finish)
        {
            which_color++;
            resp = resp_continue;
        }
        break;

    case YELLOW_COLOR:
        resp = Colors_Fading(ch_val, YELLOW_FLAG);
        if (resp == resp_finish)
        {
            which_color++;
            resp = resp_continue;
        }
        break;

    case CYAN_COLOR:
        resp = Colors_Fading(ch_val, CYAN_FLAG);
        if (resp == resp_finish)
            which_color = RED_COLOR;

        break;

    default:
        which_color = RED_COLOR;
        break;
    }

    return resp;
}


resp_t Colors_Fading_Shuffle_Pallete (unsigned char * ch_val)
{
    resp_t resp = resp_continue;

    switch (which_color)
    {
    case RED_COLOR:
        resp = Colors_Fading_Shuffle(ch_val, RED_FLAG, CYAN_FLAG);
        if (resp == resp_finish)
        {
            which_color++;
            resp = resp_continue;
        }
        break;

    case GREEN_COLOR:
        resp = Colors_Fading_Shuffle(ch_val, GREEN_FLAG, RED_FLAG);
        if (resp == resp_finish)
        {
            which_color++;
            resp = resp_continue;
        }
        break;

    case BLUE_COLOR:
        resp = Colors_Fading_Shuffle(ch_val, BLUE_FLAG, GREEN_FLAG);
        if (resp == resp_finish)
        {
            which_color++;
            resp = resp_continue;
        }
        break;

    case WHITE_COLOR:
        resp = Colors_Fading_Shuffle(ch_val, WHITE_FLAG, BLUE_FLAG);
        if (resp == resp_finish)
        {
            which_color++;
            resp = resp_continue;
        }
        break;

    case PURPLE_COLOR:
        resp = Colors_Fading_Shuffle(ch_val, PURPLE_FLAG, WHITE_FLAG);
        if (resp == resp_finish)
        {
            which_color++;
            resp = resp_continue;
        }
        break;

    case YELLOW_COLOR:
        resp = Colors_Fading_Shuffle(ch_val, YELLOW_FLAG, PURPLE_FLAG);
        if (resp == resp_finish)
        {
            which_color++;
            resp = resp_continue;
        }
        break;

    case CYAN_COLOR:
        resp = Colors_Fading_Shuffle(ch_val, CYAN_FLAG, YELLOW_FLAG);
        if (resp == resp_finish)
            which_color = RED_COLOR;

        break;

    default:
        which_color = RED_COLOR;
        break;
    }

    return resp;
}


resp_t Colors_Strobe_Pallete (unsigned char * ch_val)
{
    resp_t resp = resp_continue;

    switch (which_color)
    {
    case RED_COLOR:
        resp = Colors_Strobe(ch_val, RED_FLAG);
        if (resp == resp_finish)
        {
            which_color++;
            resp = resp_continue;
        }
        break;

    case GREEN_COLOR:
        resp = Colors_Strobe(ch_val, GREEN_FLAG);
        if (resp == resp_finish)
        {
            which_color++;
            resp = resp_continue;
        }
        break;

    case BLUE_COLOR:
        resp = Colors_Strobe(ch_val, BLUE_FLAG);
        if (resp == resp_finish)
        {
            which_color++;
            resp = resp_continue;
        }
        break;

    case WHITE_COLOR:
        resp = Colors_Strobe(ch_val, WHITE_FLAG);
        if (resp == resp_finish)
        {
            which_color++;
            resp = resp_continue;
        }
        break;

    case PURPLE_COLOR:
        resp = Colors_Strobe(ch_val, PURPLE_FLAG);
        if (resp == resp_finish)
        {
            which_color++;
            resp = resp_continue;
        }
        break;

    case YELLOW_COLOR:
        resp = Colors_Strobe(ch_val, YELLOW_FLAG);
        if (resp == resp_finish)
        {
            which_color++;
            resp = resp_continue;
        }
        break;

    case CYAN_COLOR:
        resp = Colors_Strobe(ch_val, CYAN_FLAG);
        if (resp == resp_finish)
            which_color = RED_COLOR;

        break;

    default:
        which_color = RED_COLOR;
        break;
    }

    return resp;
}

//--- end of file ---//
