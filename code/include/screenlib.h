#include <M5Core2.h>
#include <Arduino.h>
#include <lvgl.h>
#include <Wire.h>
#include <SPI.h>

// style variables for screen
static lv_style_t black_bg;
static lv_style_t white_text;

// static variables
static lv_obj_t *sys_labels[5];
static lv_obj_t *footer_labels[5];
static lv_obj_t *ring_info[5];
static lv_obj_t *msgbox_panel;
static lv_obj_t *panel_btn_confirmation;
static lv_obj_t *objSpinner;
static lv_obj_t *labelSpinner;
static const char *sysText;

// init the tft espi
static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;   // Descriptor of a display driver
static lv_indev_drv_t indev_drv; // Descriptor of a touch driver
static void ta_event_cb(lv_event_t *e);
void setMessage(String strMessage);
static void btnPowerOff_event_confirmed(lv_event_t *event);
static lv_obj_t *kb;
static void ta_event_cb(lv_event_t *e);
void tft_lv_initialization();
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void init_disp_driver();
void my_touchpad_read(lv_indev_drv_t *drv, lv_indev_data_t *data);
void init_touch_driver();

// lvgl methods and task
void updateRing(int ring_number, const char* ring_message);
static void sys_timer(lv_timer_t *timer);
static void light_toggle_event_handler(lv_event_t *e);
void start_screen_task();