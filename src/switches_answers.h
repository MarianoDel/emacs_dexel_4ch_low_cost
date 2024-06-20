//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### SWITCHES_ANSWERS.H ####################
//---------------------------------------------

#ifndef _SWITCHES_ANSWERS_H_
#define _SWITCHES_ANSWERS_H_

// Exported Types Constants and Macros -----------------------------------------

// Switches actions
typedef enum {
    selection_none = 0,
    selection_up,
    selection_dwn,
    selection_enter,
    selection_back,
    selection_up_fast,
    selection_dwn_fast

} sw_actions_t;


typedef enum {
    resp_ok = 0,
    resp_continue,
    resp_selected,
    resp_change,
    resp_change_all_up,
    resp_working,
    resp_error,
    resp_need_to_save,
    resp_finish,
    resp_nok,
    resp_timeout,
    resp_ready,
    resp_no_answer,
    resp_save

} resp_t;


typedef enum {
    SW_NO = 0,
    SW_MIN,
    SW_HALF,
    SW_FULL
    
} resp_sw_t;

#endif /* _SWITCHES_ANSWERS_H_ */

//--- end of file ---//
