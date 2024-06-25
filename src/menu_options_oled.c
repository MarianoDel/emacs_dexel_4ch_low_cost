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
    options_wait_free
    
} options_e;


// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------
options_e options = options_init;
options_e options_last = options_init;


// Module Private Functions ----------------------------------------------------


// Module Functions ------------------------------------------------------------
void Options_Up_Dwn_Select_Reset (void)
{
    options = options_init;
}


resp_t Options_Up_Dwn_Select (sw_actions_t actions)
{
    resp_t ans = resp_continue;

    switch (options)
    {
    case options_init:
	SCREEN_Text2_Line2 ("   DWN OUT");
	
	//change text type	
        gfx_setTextColor(0);
        gfx_setTextBg(1);

	//clean the box
	gfx_fillRect(0, 16, 16, 16, 0);
	// gfx_setCursor(op->startx, op->starty);
	gfx_print("UP");

	//back to normal text type	
        gfx_setTextColor(1);
        gfx_setTextBg(0);
	
	options++;
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
	}
	break;

    case options_select:
	if (actions == selection_enter)
	{
	    options_last = options_select;
	    options = options_wait_free;
	}

	if (actions == selection_up)
	{
	    ans = resp_change;
	    options_last = options_changing_up;
	    options = options_wait_free;
	}	
	break;

    case options_wait_free:
	if (actions == selection_none)
	{
	    if (options == options_select)
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


//--- end of file ---//
