#include "tab_manager.h"
#include "src/core/lv_disp.h"
#include "src/core/lv_group.h"
#include "src/core/lv_obj.h"
#include "steering.h"

lv_obj_t *tab_racing_ptr;
lv_obj_t *tab_sensors_ptr;
lv_obj_t *tab_hv_ptr;
lv_obj_t *tab_lv_ptr;
lv_obj_t *tab_test_ptr;

#if STEER_TAB_CALIBRATION_ENABLED == 1
lv_obj_t *tab_calibration_ptr;
#endif

lv_obj_t *tab_track_test_ptr;

#if STEER_TAB_DEBUG_ENABLED == 1
lv_obj_t *tab_debug_ptr;
#endif

lv_obj_t *notif_screen;
lv_obj_t *tab_fatal_error_ptr;

#if CANSNIFFER_ENABLED == 1
lv_obj_t *tab_primary_cansniffer_ptr;
lv_obj_t *tab_secondary_cansniffer_ptr;
#endif

lv_obj_t *tab_terminal_ptr;
lv_obj_t *tab_balancing_status_ptr;

lv_timer_t *notification_timer;
lv_group_t *steering_group;

racing_tab_t current_racing_tab;
engineer_tab_t current_engineer_tab;

bool steering_initialized = false;

void load_current_racing_tab();
void load_current_engineering_tab();

void tab_manager(void) {
    init_custom_styles();

    

    //tab_racing_ptr  = lv_obj_create(NULL);

    tab_test_ptr = lv_obj_create(NULL);

    /*Create a Tab view object*/
    lv_obj_t *tabview;
    tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 50);

    /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
    lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "Tab 1");
    lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Tab 2");
    lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "Tab 3");

    /*Add content to the tabs*/
    lv_obj_t * label = lv_label_create(tab1);
    lv_label_set_text(label, "This the first tab\n\n"
                             "If the content\n"
                             "of a tab\n"
                             "becomes too\n"
                             "longer\n"
                             "than the\n"
                             "container\n"
                             "then it\n"
                             "automatically\n"
                             "becomes\n"
                             "scrollable.\n"
                             "\n"
                             "\n"
                             "\n"
                             "Can you see it?");

    /*Create a spinner*/
    lv_obj_t * spinner = lv_spinner_create(tab1, 1000,60);
    lv_obj_set_size(spinner, 100, 100);
    lv_obj_center(spinner);

    // label = lv_label_create(tab2);
    // lv_label_set_text(label, "Second tab");

    // label = lv_label_create(tab3);
    // lv_label_set_text(label, "Third tab");

    lv_obj_scroll_to_view_recursive(label, LV_ANIM_OFF);

//     tab_sensors_ptr = lv_obj_create(NULL);
//     tab_hv_ptr      = lv_obj_create(NULL);
//     tab_lv_ptr      = lv_obj_create(NULL);
// #if STEER_TAB_CALIBRATION_ENABLED == 1
//     tab_calibration_ptr = lv_obj_create(NULL);
// #endif
//     tab_track_test_ptr = lv_obj_create(NULL);
// #if STEER_TAB_DEBUG_ENABLED == 1
//     tab_debug_ptr = lv_obj_create(NULL);
// #endif
//     notif_screen = lv_obj_create(NULL);

// #if CANSNIFFER_ENABLED == 1
//     tab_primary_cansniffer_ptr   = lv_obj_create(NULL);
//     tab_secondary_cansniffer_ptr = lv_obj_create(NULL);
// #endif
//     tab_terminal_ptr         = lv_obj_create(NULL);
//     tab_fatal_error_ptr      = lv_obj_create(NULL);
//     tab_balancing_status_ptr = lv_obj_create(NULL);

//     lv_group_add_obj(steering_group, tab_racing_ptr);
//     lv_group_add_obj(steering_group, tab_sensors_ptr);
//     lv_group_add_obj(steering_group, tab_hv_ptr);
//     lv_group_add_obj(steering_group, tab_lv_ptr);

// #if STEER_TAB_CALIBRATION_ENABLED == 1
//     lv_group_add_obj(steering_group, tab_calibration_ptr);
// #endif

// #if STEER_TAB_DEBUG_ENABLED == 1
//     lv_group_add_obj(steering_group, tab_debug_ptr);
// #endif

//     lv_group_add_obj(steering_group, tab_track_test_ptr);
//     lv_group_add_obj(steering_group, notif_screen);

// #if CANSNIFFER_ENABLED == 1
//     lv_group_add_obj(steering_group, tab_primary_cansniffer_ptr);
//     lv_group_add_obj(steering_group, tab_secondary_cansniffer_ptr);
// #endif
//     lv_group_add_obj(steering_group, tab_terminal_ptr);
//     lv_group_add_obj(steering_group, tab_fatal_error_ptr);
//     lv_group_add_obj(steering_group, tab_balancing_status_ptr);

    //tab_racing_create(tab_racing_ptr);

// ----------------------------------------------
// TESTING
// ----------------------------------------------


    // lv_obj_t * label1 = lv_label_create(tab_test_ptr);
    // lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP);     /*Break the long lines*/
    // lv_label_set_recolor(label1, true);                      /*Enable re-coloring by commands in the text*/
    // lv_label_set_text(label1, "#0000ff Re-color# #ff00ff words# #ff0000 of a# label, align the lines to the center "
    //                           "and wrap long text automatically.");
    // lv_obj_set_width(label1, 150);  /*Set smaller width to make the lines wrap*/
    // lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
    // lv_obj_align(label1, LV_ALIGN_CENTER, 0, -40);


    // lv_obj_t * label2 = lv_label_create(lv_scr_act());
    // lv_label_set_long_mode(label2, LV_LABEL_LONG_SCROLL_CIRCULAR);     /*Circular scroll*/
    // lv_obj_set_width(label2, 150);
    // lv_label_set_text(label2, "It is a circularly scrolling text. ");
    // lv_obj_align(label2, LV_ALIGN_CENTER, 0, 40);

// ----------------------------------------------
// END TESTING
// ----------------------------------------------


// TESTING 
    // lv_group_add_obj(steering_group, tab_test_ptr);

    // lv_obj_t * ltr_label = lv_label_create(tab_test_ptr);
    // lv_label_set_text(ltr_label, "In modern terminology, a microcontroller is similar to a system on a chip (SoC).");
    // lv_obj_set_style_text_font(ltr_label, &lv_font_montserrat_16, 0);
    // lv_obj_set_width(ltr_label, 310);
    // lv_obj_align(ltr_label, LV_ALIGN_TOP_LEFT, 5, 5);

//END TESTING

    
//     tab_sensors_create(tab_sensors_ptr);
//     tab_hv_create(tab_hv_ptr);
//     tab_lv_create(tab_lv_ptr);

// #if STEER_TAB_CALIBRATION_ENABLED == 1
//     tab_calibration_create(tab_calibration_ptr);
// #endif

// #if STEER_TAB_DEBUG_ENABLED == 1
//     tab_debug_create(tab_debug_ptr);
// #endif

//     tab_track_test_create(tab_track_test_ptr);
//     tab_notification_screen_create(notif_screen);

// #if CANSNIFFER_ENABLED == 1
//     primary_tab_cansniffer_create(tab_primary_cansniffer_ptr);
//     secondary_tab_cansniffer_create(tab_secondary_cansniffer_ptr);
// #endif
//     tab_terminal_create(tab_terminal_ptr);
//     tab_fatal_error_create(tab_fatal_error_ptr);
//     tab_balancing_status_screen_create(tab_balancing_status_ptr);

    //lv_scr_load(tab_racing_ptr);




//    lv_scr_load(tab_test_ptr);



    // current_racing_tab   = TAB_RACING;
    // current_engineer_tab = TAB_TERMINAL;
    // steering_initialized = true;
}

/***
 * Engineer Mode
 */

/**
 * @brief Reload all the messages by setting the last state to 0
 */
void reload_all(void) {
#if STRICT_RELOAD_ALL_ENABLED == 1
    memset((void *)primary_messages_last_state, 0, primary_MESSAGE_COUNT * primary_MAX_STRUCT_SIZE_CONVERSION);
    memset((void *)secondary_messages_last_state, 0, secondary_MESSAGE_COUNT * secondary_MAX_STRUCT_SIZE_CONVERSION);
    memset((void *)inverters_messages_last_state, 0, inverters_MESSAGE_COUNT * inverters_MAX_STRUCT_SIZE_CONVERSION);
#endif
}

bool engineer_mode = false;

void load_engineer_mode_screen(void) {
    load_current_engineering_tab();
}
void remove_engineer_mode_screen(void) {
    load_current_racing_tab();
}

void switch_mode(void) {
    if (engineer_mode) {
        engineer_mode = false;
        remove_engineer_mode_screen();
    } else {
        engineer_mode = true;
        load_engineer_mode_screen();
    }
    reload_all();
}

/***
 * Tab Manager
 */
/**
 * @brief Load the current racing tab
 */
void load_current_racing_tab() {
    switch (current_racing_tab) {
        case TAB_RACING:
            tab_racing_resync();
            lv_scr_load(tab_racing_ptr);
            break;
#if STEER_TAB_CALIBRATION_ENABLED == 1
        case TAB_CALIBRATION:
            lv_scr_load(tab_calibration_ptr);
            break;
#endif
#if STEER_TAB_DEBUG_ENABLED == 1
        case TAB_DEBUG:
            lv_scr_load(tab_debug_ptr);
            break;
#endif
        case TAB_TRACK_TEST:
            tab_track_test_resync();
            lv_scr_load(tab_track_test_ptr);
            break;
        case TAB_SENSORS:
            tab_sensor_resync();
            lv_scr_load(tab_sensors_ptr);
            break;
        case TAB_HV:
            tab_hv_resync();
            lv_scr_load(tab_hv_ptr);
            break;
        case TAB_LV:
            tab_lv_resync();
            lv_scr_load(tab_lv_ptr);
            break;
        default:
            break;
    }
}

void load_current_engineering_tab() {
    switch (current_engineer_tab) {
#if CANSNIFFER_ENABLED == 1
        case TAB_SECONDARY_CANSNIFFER:
            lv_scr_load(tab_secondary_cansniffer_ptr);
            break;
        case TAB_PRIMARY_CANSNIFFER:
            lv_scr_load(tab_primary_cansniffer_ptr);
            break;
#endif
        case TAB_TERMINAL:
            lv_scr_load(tab_terminal_ptr);
            break;
        case TAB_BALANCING_STATUS:
            lv_scr_load(tab_balancing_status_ptr);
            break;
        default:
            break;
    }
}

void change_racing_tab(bool forward) {
    if (forward)
        current_racing_tab = (current_racing_tab + 1) % NUM_RACING_TABS;
    else
        current_racing_tab = (current_racing_tab + NUM_RACING_TABS - 1) % NUM_RACING_TABS;

    load_current_racing_tab();
    reload_all();
}

void change_engineer_tab(bool forward) {
    if (forward)
        current_engineer_tab = (current_engineer_tab + 1) % NUM_ENGINEER_TABS;
    else
        current_engineer_tab = (current_engineer_tab + NUM_ENGINEER_TABS - 1) % NUM_ENGINEER_TABS;

    load_current_engineering_tab();
    reload_all();
}

void steering_change_tab(bool forward) {
    if (!engineer_mode) {
        change_racing_tab(forward);
    } else {
        change_engineer_tab(forward);
    }
}

/***
 * Tab Fatal Error
 */

void enter_fatal_error_mode(const char *message) {
    set_tab_fatal_error_text(message);
    lv_scr_load(tab_fatal_error_ptr);
}

/***
 * Notification Screen
 */

bool on_animation = false;

void display_notification(const char *label_content, uint32_t timeout_ms) {
    if (on_animation) {
        lv_timer_set_repeat_count(notification_timer, 0);
    }
    on_animation       = true;
    notification_timer = lv_timer_create(restore_previous_screen, timeout_ms, NULL);
    lv_timer_set_repeat_count(notification_timer, 1);
    set_notification_screen_label(label_content);
    lv_scr_load(notif_screen);
    lv_timer_reset(notification_timer);
}

void restore_previous_screen(lv_timer_t *timer) {
    if (engineer_mode) {
        load_current_engineering_tab();
    } else {
        load_current_racing_tab();
    }
    on_animation = false;
}

/***
 * Keep Lap Counter on screen for a couple of seconds
 * to allow the driver to see the lap time
 */

bool on_lap_keep = false;
lv_timer_t *lap_counter_timer;

void keep_lap_counter_value(uint32_t timeout_ms) {
    if (on_lap_keep) {
        lv_timer_set_repeat_count(lap_counter_timer, 0);
    }
    on_lap_keep       = true;
    lap_counter_timer = lv_timer_create(remove_keep_lap_counter, timeout_ms, NULL);
    lv_timer_set_repeat_count(lap_counter_timer, 1);
    lv_scr_load(tab_racing_ptr);
    lv_timer_reset(lap_counter_timer);
}

void remove_keep_lap_counter(lv_timer_t *timer) {
    on_lap_keep = false;
    lv_timer_set_repeat_count(lap_counter_timer, 0);
}
