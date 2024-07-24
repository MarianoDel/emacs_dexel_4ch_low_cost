//-----------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### MENU_OPTIONS_OLED.C #####################
//-----------------------------------------------

// Includes --------------------------------------------------------------------
#include "menu_options_oled.h"
#include "screen.h"
#include "ssd1306_display.h"
#include "ssd1306_gfx.h"



// Module Private Types & Macros -----------------------------------------------
typedef enum {
    options_init,
    options_changing_up,
    options_changing_dwn,
    options_select,
    options_done,
    options_wait_free
    
} options_e;


typedef struct {
    unsigned char set_or_reset;
    unsigned char startx;
    unsigned char starty;
    unsigned char box_width;
    unsigned char box_height;
    char * s;
    
} options_st;


// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------
options_e options = options_init;
options_e options_last = options_init;


// Module Private Functions ----------------------------------------------------
void Display_FloatingOptions (options_st * op);
void Option_Selected_To_Box (unsigned char option,
                             unsigned char * line_x,
                             unsigned char * line_y,
                             unsigned char * line_w);

void Option_Selected_Pretty (unsigned char enable, unsigned char selection, char * s);

// Module Functions ------------------------------------------------------------
void Options_Up_Dwn_Select_Reset (void)
{
    options = options_init;
}


resp_t Options_Up_Dwn_Select_Out (sw_actions_t actions, unsigned char out)
{
    resp_t ans = resp_continue;

    switch (options)
    {
    case options_init:
        SCREEN_Text2_BlankLine2();
        if (out == 2)
        {
            SCREEN_Text2_Line2 ("UP DWN NXT");	
            Option_Selected_Pretty (1, 2, "NXT");
            options = options_select;
        }
        else if (out == 1)
        {
            SCREEN_Text2_Line2 ("UP DWN OUT");
            Option_Selected_Pretty (1, 2, "OUT");
            options = options_select;
        }
        else
        {
            SCREEN_Text2_Line2 ("UP DWN OUT");
            Option_Selected_Pretty (1, 0, "UP");
            options++;            
        }
	break;

    case options_changing_up:
	if (actions == selection_enter)
	{
	    ans = resp_up;
	    options_last = options;
	    options = options_wait_free;
	}

	if (actions == selection_up)
	{
	    ans = resp_change;
	    options_last = options_changing_dwn;
	    options = options_wait_free;

            SCREEN_Text2_BlankLine2();

            if (out == 2)
            {
                SCREEN_Text2_Line2 ("UP DWN NXT");	
                Option_Selected_Pretty (1, 1, "DWN");
            }
            else
            {
                SCREEN_Text2_Line2 ("UP DWN OUT");	
                Option_Selected_Pretty (1, 1, "DWN");
            }
	}
	break;

    case options_changing_dwn:
	if (actions == selection_enter)
	{
	    ans = resp_dwn;
	    options_last = options;
	    options = options_wait_free;
	}

	if (actions == selection_up)
	{
	    ans = resp_change;
	    options_last = options_select;
	    options = options_wait_free;

            SCREEN_Text2_BlankLine2();

            if (out == 2)
            {
                SCREEN_Text2_Line2 ("UP DWN NXT");	
                Option_Selected_Pretty (1, 2, "NXT");
            }
            else
            {
                SCREEN_Text2_Line2 ("UP DWN OUT");	
                Option_Selected_Pretty (1, 2, "OUT");
            }
	}
	break;

    case options_select:
	if (actions == selection_enter)
	{
	    options_last = options_done;
	    options = options_wait_free;
	}

	if (actions == selection_up)
	{
	    ans = resp_change;
	    options_last = options_changing_up;
	    options = options_wait_free;

            SCREEN_Text2_BlankLine2();

            if (out == 2)
            {
                SCREEN_Text2_Line2 ("UP DWN NXT");	
                Option_Selected_Pretty (1, 0, "UP");
            }
            else
            {
                SCREEN_Text2_Line2 ("UP DWN OUT");	
                Option_Selected_Pretty (1, 0, "UP");
            }
	}	
	break;

    case options_wait_free:
	if (actions == selection_none)
	{
	    if (options_last == options_done)
	    {
		options = options_init;
		ans = resp_ok;
	    }
	    else
		options = options_last;
	}
	break;

    default:
	options = options_init;
	break;

    }
    
    return ans;
}


// resp_t Options_Up_Dwn_Next (sw_actions_t actions)
// {
//     resp_t ans = resp_continue;

//     switch (options)
//     {
//     case options_init:
//         SCREEN_Text2_BlankLine2();
// 	SCREEN_Text2_Line2 ("UP DWN NXT");	
//         Option_Selected_Pretty (1, 2, "NXT");
        
// 	options = options_select;
// 	break;

//     case options_changing_up:
// 	if (actions == selection_enter)
// 	{
// 	    ans = resp_up;
// 	    options_last = options;
// 	    options = options_wait_free;
// 	}

// 	if (actions == selection_up)
// 	{
// 	    ans = resp_change;
// 	    options_last = options_changing_dwn;
// 	    options = options_wait_free;

//             SCREEN_Text2_BlankLine2();
//             SCREEN_Text2_Line2 ("UP DWN NXT");	
//             Option_Selected_Pretty (1, 1, "DWN");
            
// 	}
// 	break;

//     case options_changing_dwn:
// 	if (actions == selection_enter)
// 	{
// 	    ans = resp_dwn;
// 	    options_last = options;
// 	    options = options_wait_free;
// 	}

// 	if (actions == selection_up)
// 	{
// 	    ans = resp_change;
// 	    options_last = options_select;
// 	    options = options_wait_free;

//             SCREEN_Text2_BlankLine2();
//             SCREEN_Text2_Line2 ("UP DWN NXT");	
//             Option_Selected_Pretty (1, 2, "NXT");
// 	}
// 	break;

//     case options_select:
// 	if (actions == selection_enter)
// 	{
// 	    options_last = options_done;
// 	    options = options_wait_free;
// 	}

// 	if (actions == selection_up)
// 	{
// 	    ans = resp_change;
// 	    options_last = options_changing_up;
// 	    options = options_wait_free;

//             SCREEN_Text2_BlankLine2();
//             SCREEN_Text2_Line2 ("UP DWN NXT");	
//             Option_Selected_Pretty (1, 0, "UP");            
// 	}	
// 	break;

//     case options_wait_free:
// 	if (actions == selection_none)
// 	{
// 	    if (options_last == options_done)
// 	    {
// 		options = options_init;
// 		ans = resp_ok;
// 	    }
// 	    else
// 		options = options_last;
// 	}
// 	break;

//     default:
// 	options = options_init;
// 	break;

//     }
    
//     return ans;
// }


void Display_FloatingOptions (options_st * op)
{
    //change text type
    if (op->set_or_reset)
    {
        gfx_setTextColor(0);
        gfx_setTextBg(1);
    }

    //clean the box
    gfx_fillRect(op->startx, op->starty, op->box_width, op->box_height, 0);
    gfx_setCursor(op->startx, op->starty);
    gfx_print(op->s);

    //back to normal text type
    if (op->set_or_reset)
    {
        gfx_setTextColor(1);
        gfx_setTextBg(0);
    }
}


// void Display_SetOptions (unsigned char line, const char * s)
// {
//     char pretty [ALL_LINE_LENGTH_NULL] = { 0 };
//     unsigned char line_offset = LINE_HEIGHT * (line - 1);

//     if (strlen(s) > (ALL_LINE_LENGTH - 2))
//         return;

//     //clean line
//     gfx_fillRect(SRT_X, line_offset, WIDTH, LINE_HEIGHT, 0);

//     //change text type
//     gfx_setTextColor(0);
//     gfx_setTextBg(1);

//     //write prettify string
//     gfx_setCursor(SRT_X, line_offset);    
//     sprintf(pretty, " %s ", s);
//     gfx_print(pretty);

//     //back to normal text type
//     gfx_setTextColor(1);
//     gfx_setTextBg(0);
    
// }


#define WIDTH    128
#define LINE_HEIGHT    16

#define SRT_X_OP0    0
#define SRT_X_OP1    (6 * 6)
#define SRT_X_OP2    (6 * 14)
// #define SRT_X_OP3    (6 * 5)
// #define SRT_X_OP4    (6 * 18)
// #define SRT_X_OP5    (6 * 18)
// #define SRT_X_OP6    (6 * 18)

#define SRT_Y_OP0    (LINE_HEIGHT)
#define SRT_Y_OP1    (LINE_HEIGHT)
#define SRT_Y_OP2    (LINE_HEIGHT)
// #define SRT_Y_OP3    (LINE_HEIGHT * 4)
// #define SRT_Y_OP4    (LINE_HEIGHT * 2)
// #define SRT_Y_OP5    (LINE_HEIGHT * 3)
// #define SRT_Y_OP6    (LINE_HEIGHT * 4)

#define WIDTH_OP0    (6 * 4)
#define WIDTH_OP1    (6 * 6)
#define WIDTH_OP2    (6 * 10)
// #define WIDTH_OP3    (6 * 3)
// #define WIDTH_OP4    (6 * 3)
// #define WIDTH_OP5    (6 * 3)
// #define WIDTH_OP6    (6 * 3)

void Option_Selected_To_Box (unsigned char option,
                             unsigned char * line_x,
                             unsigned char * line_y,
                             unsigned char * line_w)
{
    switch (option)
    {
    case 0:
        *line_x = SRT_X_OP0;
        *line_y = SRT_Y_OP0;
        *line_w = WIDTH_OP0;
        break;

    case 1:
        *line_x = SRT_X_OP1;
        *line_y = SRT_Y_OP1;
        *line_w = WIDTH_OP1;
        break;

    case 2:
        *line_x = SRT_X_OP2;
        *line_y = SRT_Y_OP2;
        *line_w = WIDTH_OP2;        
        break;

    // case 3:
    //     *line_x = SRT_X_OP3;
    //     *line_y = SRT_Y_OP3;
    //     *line_w = WIDTH_OP3;        
    //     break;

    // case 4:
    //     *line_x = SRT_X_OP4;
    //     *line_y = SRT_Y_OP4;
    //     *line_w = WIDTH_OP4;        
    //     break;

    // case 5:
    //     *line_x = SRT_X_OP5;
    //     *line_y = SRT_Y_OP5;
    //     *line_w = WIDTH_OP5;        
    //     break;

    // case 6:
    //     *line_x = SRT_X_OP6;
    //     *line_y = SRT_Y_OP6;
    //     *line_w = WIDTH_OP6;        
    //     break;
    }
}


void Option_Selected_Pretty (unsigned char enable, unsigned char selection, char * s)
{
    options_st options;
    unsigned char line_x = 0;
    unsigned char line_y = 0;
    unsigned char line_w = 0;
    
    Option_Selected_To_Box(selection, &line_x, &line_y, &line_w);

    if (enable)
        options.set_or_reset = 1;
    else
        options.set_or_reset = 0;
    
    options.startx = line_x;
    options.starty = line_y;
    options.box_width = line_w;
    options.box_height = LINE_HEIGHT;
    options.s = s;
    Display_FloatingOptions(&options);
    
}

//--- end of file ---//
