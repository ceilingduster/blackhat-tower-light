#include <M5Core2.h>
#include <Arduino.h>
#include <lvgl.h>
#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <ArduinoNvs.h>
#include "screenlib.h"

M5Display *tft;

// update the rings on the tft
void updateRing(int ring_number, const char* ring_message) 
{
  lv_label_set_text(ring_info[ring_number], ring_message);
}

// sets a message on the spinner label
void setMessage(String strMessage)
{
  Serial.println(strMessage);
  lv_label_set_text_fmt(labelSpinner, "%s", strMessage.c_str());
}

static void ta_event_cb(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *ta = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED)
  {
    /*Focus on the clicked text area*/
    if (kb != NULL)
      lv_keyboard_set_textarea(kb, ta);
  }

  else if (code == LV_EVENT_READY)
  {
    LV_LOG_USER("Ready, current text: %s", lv_textarea_get_text(ta));
  }
}

void tft_lv_initialization()
{
  M5.begin();
  lv_init();

  static lv_color_t buf1[(LV_HOR_RES_MAX * LV_VER_RES_MAX) / 10]; // Declare a buffer for 1/10 screen siz
  static lv_color_t buf2[(LV_HOR_RES_MAX * LV_VER_RES_MAX) / 10]; // second buffer is optionnal

  // Initialize `disp_buf` display buffer with the buffer(s).
  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, (LV_HOR_RES_MAX * LV_VER_RES_MAX) / 10);

  tft = &M5.Lcd;
}

// Display flushing
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft->startWrite();
  tft->setAddrWindow(area->x1, area->y1, w, h);
  tft->pushColors((uint16_t *)&color_p->full, w * h, true);
  tft->endWrite();

  lv_disp_flush_ready(disp);
}

void init_disp_driver()
{
  lv_disp_drv_init(&disp_drv); // Basic initialization

  disp_drv.flush_cb = my_disp_flush; // Set your driver function
  disp_drv.draw_buf = &draw_buf;     // Assign the buffer to the display
  disp_drv.hor_res = LV_HOR_RES_MAX; // Set the horizontal resolution of the display
  disp_drv.ver_res = LV_VER_RES_MAX; // Set the vertical resolution of the display
  disp_drv.dpi = 180;

  lv_disp_drv_register(&disp_drv);                  // Finally register the driver
  lv_disp_set_bg_color(NULL, lv_color_hex3(0x000)); // Set default background color to black
}

void my_touchpad_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
  TouchPoint_t pos = M5.Touch.getPressPoint();
  bool touched = (pos.x == -1) ? false : true;

  if (!touched)
  {
    data->state = LV_INDEV_STATE_RELEASED;
  }
  else
  {
    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = pos.x;
    data->point.y = pos.y;
  }
}

void init_touch_driver()
{
  lv_disp_drv_register(&disp_drv);

  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_t *my_indev = lv_indev_drv_register(&indev_drv); // register
}

static void shutdown_poweroff(lv_event_t *event)
{
  M5.Axp.PowerOff(); // user wants us to poweroff/shutdown
}

static void reset_nvs_settings(lv_event_t *event)
{
  lv_obj_add_flag(msgbox_panel, LV_OBJ_FLAG_HIDDEN);

  bool res;
  res = NVS.setString("ssid", "");
  res = NVS.setString("psk", "");

  WiFi.disconnect();
  WiFi.mode(WIFI_AP_STA);

  Serial.println("Starting Smart Config.");
  setMessage("Use Smart Config on your phone.");
  WiFi.beginSmartConfig();
}

static void close_msgbox_panel(lv_event_t *event)
{
  lv_obj_add_flag(msgbox_panel, LV_OBJ_FLAG_HIDDEN);
}

static void button_monitor(lv_timer_t *timer)
{
  /* check the buttons for wasPressed Events */
  M5.update();
  
  if (M5.BtnA.wasPressed())
  {
    lv_obj_clear_flag(msgbox_panel, LV_OBJ_FLAG_HIDDEN);                                    // show the confirmation dialogue
    lv_obj_add_event_cb(panel_btn_confirmation, shutdown_poweroff, LV_EVENT_CLICKED, NULL); // set the button to power off when clicked
  }

  if (M5.BtnC.wasPressed())
  {
    lv_obj_clear_flag(msgbox_panel, LV_OBJ_FLAG_HIDDEN);                                     // show the confirmation dialogue
    lv_obj_add_event_cb(panel_btn_confirmation, reset_nvs_settings, LV_EVENT_CLICKED, NULL); // set the button to power off when clicked
  }
}

static void sys_timer(lv_timer_t *timer)
{
  /* Use the user_data */
  bool bCharging = false;
  if (M5.Axp.GetVinCurrent() > 0)
  {
    bCharging = true;
  }
  else
  {
    bCharging = false;
  }

  // calculate battery voltage for percentagef
  float batVoltage = M5.Axp.GetBatVoltage();
  float batPercentage = (batVoltage < 3.2) ? 0 : (batVoltage - 3.2) * 100;

  const char *strCharging = "";
  const char *strBatLevel = "\xef\x89\x84";

  // if charging, display charging icon
  if (bCharging)
  {
    strCharging = "\xef\x83\xa7";
  }
  else
  {
    strCharging = "";
  }

  // calculate which bat icon to use
  if (batPercentage == 0 || batPercentage < 20)
  {
    strBatLevel = "\xef\x89\x84";
  }
  else if (batPercentage >= 20 && batPercentage <= 40)
  {
    strBatLevel = "\xef\x89\x83";
  }
  else if (batPercentage >= 40 && batPercentage <= 60)
  {
    strBatLevel = "\xef\x89\x82";
  }
  else if (batPercentage >= 60 && batPercentage <= 80)
  {
    strBatLevel = "\xef\x89\x81";
  }
  else if (batPercentage >= 80)
  {
    strBatLevel = "\xef\x89\x80";
  }

  // set the bat label
  // for debug: lv_label_set_text_fmt(sys_labels[3], "%s", String(batPercentage) + "%");
  lv_label_set_text_fmt(sys_labels[1], "%s %s", strCharging, strBatLevel);

  /* wifi info */
  if (WiFi.status() == WL_CONNECTED)
  {
    // set the wifi ssi
    lv_label_set_text_fmt(sys_labels[0], "%s \xef\x87\xab", WiFi.SSID());

    // set the ip address
    lv_label_set_text_fmt(sys_labels[2], "%s", WiFi.localIP().toString());
    lv_obj_add_flag(objSpinner, LV_OBJ_FLAG_HIDDEN);
  }
  else
  {
    // set the wifi ssid
    lv_label_set_text(sys_labels[0], "");

    // set the ip address
    lv_label_set_text(sys_labels[2], "");
    lv_obj_clear_flag(objSpinner, LV_OBJ_FLAG_HIDDEN);
  }
}

void setup_styles()
{
  // background black style
  lv_style_init(&black_bg);
  lv_style_set_bg_color(&black_bg, lv_color_hex(0x000000));
  lv_style_set_radius(&black_bg, 0);
  lv_style_set_border_width(&black_bg, 1);
  lv_style_set_border_color(&black_bg, lv_color_hex(0x000000));
  lv_style_set_height(&black_bg, 240);
  lv_style_set_width(&black_bg, 320);

  // white text style
  lv_style_init(&white_text);
  lv_style_set_text_color(&white_text, lv_color_hex(0xffffff));
}

void start_screen_task()
{
  setup_styles();

  /* set background color of display */
  static lv_obj_t *main_panel = lv_obj_create(lv_scr_act());
  lv_color_t c = lv_color_make(0, 0, 0);
  lv_obj_add_style(main_panel, &black_bg, 0);

  /* RING INFO */
  ring_info[0] = lv_label_create(lv_scr_act());
  lv_obj_align(ring_info[0], LV_ALIGN_LEFT_MID, 10, 0);
  lv_label_set_text(ring_info[0], "");
  lv_obj_set_style_text_font(ring_info[0], &lv_font_montserrat_14, 0);
  lv_obj_add_style(ring_info[0], &white_text, 0);

  ring_info[1] = lv_label_create(lv_scr_act());
  lv_obj_align(ring_info[1], LV_ALIGN_CENTER, 0, 0);
  lv_label_set_text(ring_info[1], "");
  lv_obj_set_style_text_font(ring_info[1], &lv_font_montserrat_14, 0);
  lv_obj_add_style(ring_info[1], &white_text, 0);

  ring_info[2] = lv_label_create(lv_scr_act());
  lv_obj_align(ring_info[2], LV_ALIGN_RIGHT_MID, -10, 0);
  lv_label_set_text(ring_info[2], "");
  lv_obj_set_style_text_font(ring_info[2], &lv_font_montserrat_14, 0);
  lv_obj_add_style(ring_info[2], &white_text, 0);

  /* spinner goes here, by default we'll hide it */
  objSpinner = lv_spinner_create(lv_scr_act(), 1000, 60);
  lv_obj_set_size(objSpinner, 100, 100);
  lv_obj_center(objSpinner);
  lv_obj_add_flag(objSpinner, LV_OBJ_FLAG_HIDDEN);

  /* header/os information at top */
  labelSpinner = lv_label_create(lv_scr_act());
  lv_obj_align(labelSpinner, LV_ALIGN_CENTER, 0, 70);
  lv_label_set_text(labelSpinner, NULL);
  lv_obj_add_style(labelSpinner, &white_text, 0);

  sys_labels[0] = lv_label_create(lv_scr_act());
  lv_obj_align(sys_labels[0], LV_ALIGN_TOP_RIGHT, -10, 10);
  lv_label_set_text(sys_labels[0], "");
  lv_obj_add_style(sys_labels[0], &white_text, 0);

  sys_labels[1] = lv_label_create(lv_scr_act());
  lv_obj_align(sys_labels[1], LV_ALIGN_TOP_LEFT, 10, 10);
  lv_label_set_text(sys_labels[1], "");
  lv_obj_add_style(sys_labels[1], &white_text, 0);

  sys_labels[2] = lv_label_create(lv_scr_act());
  lv_obj_align(sys_labels[2], LV_ALIGN_TOP_RIGHT, -15, 27);
  lv_label_set_text(sys_labels[2], "");
  lv_timer_create(sys_timer, 1000, NULL);
  lv_obj_add_style(sys_labels[2], &white_text, 0);

  sys_labels[3] = lv_label_create(lv_scr_act());
  lv_obj_align(sys_labels[3], LV_ALIGN_CENTER, 0, -100);
  lv_label_set_text(sys_labels[3], "");
  lv_obj_set_style_text_font(sys_labels[3], &lv_font_montserrat_10, 0);
  lv_obj_add_style(sys_labels[3], &white_text, 0);

  /* footer/button labels at bottom */
  footer_labels[0] = lv_label_create(lv_scr_act());
  lv_obj_align(footer_labels[0], LV_ALIGN_BOTTOM_LEFT, 30, 0);
  lv_label_set_text(footer_labels[0], "Power");
  lv_obj_set_style_text_font(footer_labels[0], &lv_font_montserrat_14, 0);
  lv_obj_add_style(footer_labels[0], &white_text, 0);

  footer_labels[1] = lv_label_create(lv_scr_act());
  lv_obj_align(footer_labels[1], LV_ALIGN_BOTTOM_RIGHT, -35, 0);
  lv_label_set_text(footer_labels[1], "Reset");
  lv_obj_set_style_text_font(footer_labels[1], &lv_font_montserrat_14, 0);
  lv_obj_add_style(footer_labels[1], &white_text, 0);

  /* create the messaget box */
  msgbox_panel = lv_obj_create(lv_scr_act());
  lv_obj_set_height(msgbox_panel, 160);
  lv_obj_set_width(msgbox_panel, 300);
  lv_obj_align(msgbox_panel, LV_ALIGN_CENTER, 0, 0);

  static lv_obj_t *panel_warning_label = lv_label_create(msgbox_panel);
  lv_obj_align(panel_warning_label, LV_ALIGN_CENTER, 0, -30);
  lv_label_set_text(panel_warning_label, "Are you sure?");
  lv_obj_set_style_text_font(panel_warning_label, &lv_font_montserrat_28, 0);

  panel_btn_confirmation = lv_btn_create(msgbox_panel);
  lv_obj_t *panel_btn_confirmation_label = lv_label_create(panel_btn_confirmation);
  lv_obj_align(panel_btn_confirmation, LV_ALIGN_BOTTOM_LEFT, 10, -10);
  lv_label_set_text(panel_btn_confirmation_label, "Yes");
  lv_obj_center(panel_btn_confirmation_label);
  lv_obj_set_style_text_font(panel_btn_confirmation_label, &lv_font_montserrat_22, 0);

  static lv_obj_t *panel_btn_cancel = lv_btn_create(msgbox_panel);
  lv_obj_t *panel_btn_cancel_label = lv_label_create(panel_btn_cancel);
  lv_obj_align(panel_btn_cancel, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
  lv_label_set_text(panel_btn_cancel_label, "No");
  lv_obj_center(panel_btn_cancel_label);
  lv_obj_set_style_text_font(panel_btn_cancel_label, &lv_font_montserrat_22, 0);
  lv_obj_add_flag(msgbox_panel, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_event_cb(panel_btn_cancel, close_msgbox_panel, LV_EVENT_CLICKED, NULL); // set the button to power off when clicked

  /* button monitor */ 
  lv_timer_create(button_monitor, 20, NULL);
}
