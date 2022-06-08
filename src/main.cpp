#include "MyDisplay.h"

lv_obj_t * mainScreen;
// lv_obj_t * titleImage;
lv_obj_t * qrCode;
lv_obj_t *anyaGif;

LV_IMG_DECLARE(anya_compressed_raw);

void displayFlush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
  uint32_t w = ( area->x2 - area->x1 + 1 );
  uint32_t h = ( area->y2 - area->y1 + 1 );

  myDisplay.startWrite();
  myDisplay.setAddrWindow( area->x1, area->y1, w, h );
  myDisplay.writePixels((lgfx::rgb565_t *)&color_p->full, w * h);
  myDisplay.endWrite();
  lv_disp_flush_ready(disp);
}

void readTouchPad( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
    uint16_t touchX, touchY;
    bool touched = myDisplay.getTouch(&touchX, &touchY);
    if (!touched)
    {
      data->state = LV_INDEV_STATE_REL;
    }
    else
    {
      data->state = LV_INDEV_STATE_PR;
      /*Set the coordinates*/
      data->point.x = touchX;
      data->point.y = touchY;
      // Serial.printf("Touch (x,y): (%03d,%03d)\n",touchX,touchY );
    }
}

const char * text;

static void ta_event_cb(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * ta = lv_event_get_target(e);
  lv_obj_t * kb = (lv_obj_t*)lv_event_get_user_data(e);




  if (code == LV_EVENT_READY) {
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    text = lv_textarea_get_text(ta);
    if (strlen(text) == 0) return;
    lv_qrcode_update(qrCode, text, strlen(text));    
    Serial.println(text);
    lv_obj_clear_flag(qrCode, LV_OBJ_FLAG_HIDDEN);
  }

  if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED) {
    lv_keyboard_set_textarea(kb, ta);
    lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    Serial.println("Keyboard opened");
    lv_obj_add_flag(qrCode, LV_OBJ_FLAG_HIDDEN);
  }

  if (code == LV_EVENT_DEFOCUSED) {
    lv_keyboard_set_textarea(kb, NULL);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
  }
}

void ui_background() {
  mainScreen = lv_obj_create(NULL);
  lv_obj_clear_flag(mainScreen, LV_OBJ_FLAG_SCROLLABLE);

  // titleImage = lv_img_create(mainScreen); 
   // lv_img_set_src(titleImage, &anya_compressed);
  // lv_obj_set_size(titleImage, 160, 90);
  // lv_obj_set_pos(titleImage, 0, 0);
  // lv_obj_set_align(titleImage, LV_ALIGN_TOP_MID);
  // lv_obj_add_flag(titleImage, LV_OBJ_FLAG_ADV_HITTEST);
  // lv_obj_clear_flag(titleImage, LV_OBJ_FLAG_SCROLLABLE);

  // anyaGif= lv_gif_create_from_data(mainScreen, anya_compressed.data);
  // anyaGif = lv_img_create(mainScreen);
  // lv_img_set_src(anyaGif, &anya_compressed);
  anyaGif= lv_gif_create(mainScreen);
  lv_gif_set_src(anyaGif, &anya_compressed_raw); //hrus ambil map data
  lv_obj_set_size(anyaGif, 160, 90);
  lv_obj_set_pos(anyaGif, 0, 0);
  lv_obj_set_align(anyaGif, LV_ALIGN_TOP_MID);
  lv_obj_add_flag(anyaGif, LV_OBJ_FLAG_ADV_HITTEST); 
  lv_obj_clear_flag(anyaGif, LV_OBJ_FLAG_SCROLLABLE); 

  lv_obj_t * titleLabel = lv_label_create(mainScreen);
  lv_obj_set_pos(titleLabel, 0, 150);
  lv_obj_set_align(titleLabel, LV_ALIGN_TOP_MID);
  lv_label_set_text(titleLabel, "QR Code Maker");
  lv_obj_clear_flag(titleLabel, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
}

void ui_dynamic_obj(void) {
  lv_obj_t * kb = lv_keyboard_create(mainScreen);
  lv_obj_t * ta = lv_textarea_create(mainScreen);
  lv_obj_align(ta, LV_ALIGN_CENTER, 0, -20);
  lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_ALL, kb);
  lv_obj_set_size(ta, screenWidth - 80, 30);
  lv_keyboard_set_textarea(kb, ta);

  qrCode = lv_qrcode_create(mainScreen, 100, lv_color_hex3(0x000), lv_color_hex3(0xeef));
  lv_obj_set_pos(qrCode, 0, 0);
  lv_obj_set_align(qrCode, LV_ALIGN_TOP_MID);
  lv_obj_add_flag(qrCode, LV_OBJ_FLAG_HIDDEN);
}

void ui_init() {
  ui_background();
  ui_dynamic_obj();
  lv_disp_load_scr(mainScreen);
}

void setup()
{
  Serial.begin(115200);
  myDisplay.begin();
  myDisplay.init();
  //to set rotation
  // if (myDisplay.width() < myDisplay.height()){
  //   myDisplay.setRotation(myDisplay.getRotation() ^ 1);
  // }
  // myDisplay.setRotation(myDisplay.getRotation() ^ 1);
  myDisplay.setBrightness(255);
  lv_init();
  lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * 10 );

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);

  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = displayFlush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = readTouchPad;
  lv_indev_drv_register(&indev_drv);

  ui_init();
}

void loop() {
  lv_timer_handler();
  delay( 5 );
}

