#include "kb.h"
#include "math.h"
#include "colors.h"
#include "timer.h"
#include "fonts.h"

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128
#define STATUS_BAR_HEIGHT 12
#define STATUS_BAR_R DARKGRAY_R
#define STATUS_BAR_G DARKGRAY_G
#define STATUS_BAR_B DARKGRAY_B
#define STATUS_BAR_FONT_R WHITE_R
#define STATUS_BAR_FONT_G WHITE_G
#define STATUS_BAR_FONT_B WHITE_B

#define BACKGROUND_R BLACK_R
#define BACKGROUND_G BLACK_G
#define BACKGROUND_B BLACK_B

uint8_t character_shift  = 0;
bool switchKey = false;
bool holdSwitch = false;
bool recordingMacro = false;
bool capsLock = false;
char input[100] = "";

void submitInput(void);
void setupOLED(void);
void unhighlightScreen(void);
void highlightScreen(void);
void refreshStatusBar(void);
void drawPixel(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b);
void fillRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t r, uint8_t g, uint8_t b);
void fillScreen(uint8_t r, uint8_t g, uint8_t b);
void drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t r, uint8_t g, uint8_t b);
void drawHLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t r, uint8_t g, uint8_t b);
void drawVLine(uint8_t x0, uint8_t y0, uint8_t y1, uint8_t r, uint8_t g, uint8_t b);
void drawHLineFull(uint8_t y0, uint8_t r, uint8_t g, uint8_t b);
void drawVLineFull(uint8_t x0, uint8_t r, uint8_t g, uint8_t b);
void drawRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t r, uint8_t g, uint8_t b);
void drawCircle(uint8_t x0, uint8_t y0, uint8_t radius, uint8_t r, uint8_t g, uint8_t b);
void fillCircle(uint8_t x0, uint8_t y0, uint8_t radius, uint8_t r, uint8_t g, uint8_t b);
void fillCircleHelper(uint8_t x0, uint8_t y0, uint8_t radius, uint8_t corners, uint8_t delta, uint8_t r, uint8_t g, uint8_t b);
void drawTriangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t r, uint8_t g, uint8_t b);


typedef struct {
        bool is_press_action;
        int state;
} tap;

enum custom_keycodes {
        TRIPLE_ZERO = SAFE_RANGE,
        KC_DELAY = SAFE_RANGE + 1,
        KC_SWITCH = SAFE_RANGE + 2
};

void printText(char text[100], uint8_t x0, uint8_t y0, uint8_t r, uint8_t g, uint8_t b, uint8_t scale);

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (switchKey) {
      switch (keycode) {
        case KC_SWITCH:
          if (record->event.pressed) {
            switchKey = false;
            unhighlightScreen();
            return false;
          }
         case KC_A ... KC_Z:
            if (record->event.pressed) {
             strcat(input, characters[(keycode - KC_A)]);
             return false;
            }
         case KC_1 ... KC_0:
           if (record->event.pressed) {
            strcat(input, characters[keycode - KC_1 + 26]);
            return false;
           }
         case KC_ENT:
           if (record->event.pressed) {
            submitInput();
            return false;
           }
         case KC_SPC:
           if (record->event.pressed) {
            strcat(input, " ");
            return false;
          }
      }
      return false;
    }
    switch (keycode) {
        case TRIPLE_ZERO:
            if (record->event.pressed) {
                SEND_STRING("000");
                return false;
            }
        case KC_DELAY:
            if (record->event.pressed) {
                _delay_ms(2500);
                return false;
            }
        case KC_LSFT:
          if (record->event.pressed) {
            character_shift++;
          } else {
            character_shift--;
          }
          return true;
        case KC_RSFT:
          if (record->event.pressed) {
            character_shift++;
          } else {
            character_shift--;
          }
          return true;
        case KC_SWITCH:
          if (record->event.pressed) {
             switchKey = true;
             highlightScreen();
             return false;
          }
    }
    return true;
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

	KEYMAP(
		KC_ESC, KC_F2, KC_F3, KC_F4, KC_F5, KC_VOLD, KC_VOLU, KC_F8, KC_F9, KC_F10, KC_F11, KC_F12, TG(3), KC_DEL,
		KC_TILD, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_EQL, KC_BSPC,
		KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC, KC_RBRC, KC_BSLS,
		MO(1), KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, MT(MOD_LSFT, KC_QUOT), KC_ENT, KC_PGUP, DYN_MACRO_PLAY1,
		KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, MT(MOD_LSFT, KC_SLSH), KC_LEFT, KC_UP, KC_PGDN, DYN_MACRO_PLAY2,
		LM(2, MOD_LCTL), KC_LGUI, KC_LALT, KC_SPC, KC_HOME, KC_END, KC_DOWN, KC_RGHT, KC_SWITCH),

	KEYMAP(
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_F6, KC_F7, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, RESET, KC_TRNS,
		KC_TRNS, KC_1, KC_2, KC_3, KC_DLR, KC_PERC, KC_CIRC, KC_AMPR, KC_ASTR, KC_LPRN, KC_RPRN, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_MINS, KC_4, KC_5, KC_6, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_7, KC_8, KC_9, KC_ENT, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_0, TRIPLE_ZERO, KC_DOT, KC_ENT, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		LM(2, MOD_LCTL), KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),

	KEYMAP(
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_LOCK, KC_DELAY, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_GRV, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TAB, KC_Q, KC_W, KC_E, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_NO, KC_A, KC_S, KC_D, KC_F, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, DYN_REC_START1,
		KC_TRNS, KC_Z, KC_X, KC_C, KC_V, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, DYN_REC_START2,
		KC_LCTL, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, DYN_REC_STOP),

	KEYMAP(
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, TG(3), KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_MS_ACCEL0, KC_MS_ACCEL1, KC_MS_ACCEL2,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_MS_LEFT, KC_MS_UP, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_MS_BTN2, KC_MS_BTN1, KC_MS_DOWN, KC_MS_RIGHT, KC_TRNS),

	KEYMAP(
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),

	KEYMAP(
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),

	KEYMAP(
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),

	KEYMAP(
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),

	KEYMAP(
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),

	KEYMAP(
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),

	KEYMAP(
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),

	KEYMAP(
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),

	KEYMAP(
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),

	KEYMAP(
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),

	KEYMAP(
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),

	KEYMAP(
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS)

};

void led_set_user(uint8_t usb_led) {

	if (usb_led & (1 << USB_LED_NUM_LOCK)) {

	} else {

	}

	if (usb_led & (1 << USB_LED_CAPS_LOCK)) {

	} else {

	}

	if (usb_led & (1 << USB_LED_SCROLL_LOCK)) {

	} else {

	}

	if (usb_led & (1 << USB_LED_COMPOSE)) {

	} else {

	}

	if (usb_led & (1 << USB_LED_KANA)) {

	} else {

	}
}

/****/

void submitInput () {
  printText(input, 0, SCREEN_HEIGHT - 1, WHITE_R, WHITE_G, WHITE_B, 1);
  printText(input, 0, 70, WHITE_R, WHITE_G, WHITE_B, 2);
  input[0] = '\0';
}

/****/

#define sclk B1
#define  mosi B2
#define  cs B0
const int DCpin =  B4;
const int RSTpin =  B5;

uint8_t font_a[2][16] = {{0x02,0x02,0x02,0x1f,0x02,0x02,0x03,0x04,0x0c,0x14,0x24,0x23,0x25,0x18,0x00,0x00},
                        {0x00,0x00,0x60,0x80,0x00,0x40,0xf0,0x48,0x44,0x82,0x82,0x02,0x04,0x18,0x60,0x00}};

void writeData(uint8_t c);
void writeCommand(uint8_t c);
void SPIwrite(uint8_t c);
void Font2x2(uint8_t StartX, uint8_t StartY, uint8_t Red, uint8_t Green, uint8_t Blue, uint8_t* buf);
void Font1x1(uint8_t StartX, uint8_t StartY, uint8_t Red, uint8_t Green, uint8_t Blue, uint8_t* buf);
void SSD1351_BlackOut(void);
void SSD1351_Init(void);

void refreshStatusBar() {
  fillRect(0,SCREEN_HEIGHT - STATUS_BAR_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT, STATUS_BAR_R, STATUS_BAR_G, STATUS_BAR_B);
  uint8_t startx = 0;
  if (capsLock) {
    // drawSymbol
    startx += 7;
  }
  if (recordingMacro) {
    //drawSymbol
    startx += 7;
  }
  if (holdSwitch) {
    //drawSymbol
    startx += 7;
  }
}



void setupOLED() {
  setPinOutput(DCpin);
  setPinOutput(sclk);
  setPinOutput(mosi);
  setPinOutput(RSTpin);

  setPinOutput(cs);
  writePinLow(cs);

  writePinHigh(RSTpin);
  _delay_ms(500);
  writePinLow(RSTpin);
  _delay_ms(500);
  writePinHigh(RSTpin);
  _delay_ms(500);

  SSD1351_Init();

  _delay_ms(100);

  fillRect(0,0,127,127,0,0,0);
}

void SSD1351_Init(){
  writeCommand(0xFD); //Set Command Lock
    writeData(0x12); //Unlock OLED driver IC MCU interface from entering command
  writeCommand(0xFD); //Set Command Lock
    writeData(0xB1); //Command A2,B1,B3,BB,BE,C1 accessible if in unlock state
  writeCommand(0xAE); //Sleep mode On (Display OFF)
  writeCommand(0xB3); //Front Clock Divider
    writeCommand(0xF1); // 7:4 = Oscillator Frequency, 3:0 = CLK Div Ratio (A[3:0]+1 = 1..16)
  writeCommand(0xCA); //Set MUX Ratio
    writeData(127);
  writeCommand(0xA0); //Set Re-map
    //writeData(B01110100); //65k color
    //writeData(B10110100); //262k color
    //writeData(B11110100); //262k color, 16-bit format 2
  writeCommand(0x15); //Set Column
    writeData(0); //start
    writeData(127); //end
  writeCommand(0x75); //Set Row
    writeData(0); //start
    writeData(127); //end
  writeCommand(0xA1); //Set Display Start Line
    writeData(0);
  writeCommand(0xA2); //Set Display Offset
    writeData(0);
  writeCommand(0xB5); //Set GPIO
    writeData(0);
  writeCommand(0xAB); //Function Selection
    writeData(0x01); //Enable internal Vdd /8-bit parallel
    //writeData(B01000001); //Enable internal Vdd /Select 16-bit parallel interface
  writeCommand(0xB1); //Set Reset(Phase 1) /Pre-charge(Phase 2)
    //writeCommand(B00110010); //5 DCLKs / 3 DCLKs
    writeCommand(0x74);
  writeCommand(0xBE); //Set VCOMH Voltage
    writeCommand(0x05); //0.82 x VCC [reset]
  writeCommand(0xA6); //Reset to normal display
  writeCommand(0xC1); //Set Contrast
    writeData(0xC8); //Red contrast (reset=0x8A)
    writeData(0x80); //Green contrast (reset=0x51)
    writeData(0xC8); //Blue contrast (reset=0x8A)
  writeCommand(0xC7); //Master Contrast Current Control
    writeData(0x0F); //0-15
  writeCommand(0xB4); //Set Segment Low Voltage(VSL)
    writeData(0xA0);
    writeData(0xB5);
    writeData(0x55);
  writeCommand(0xB6); //Set Second Precharge Period
    writeData(0x01); //1 DCLKS
  writeCommand(0x9E); //Scroll Stop Moving
  writeCommand(0xAF); //Sleep mode On (Display ON)
}



void drawPixel(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b) {
  uint8_t RGBbit1, RGBbit2;
  RGBbit1 = (r<<3) | (g>>3);
  RGBbit2 = (g<<5) | b;
  writeCommand(0x15);
  writeData(x);
  writeData(x);
  writeCommand(0x75);
  writeData(y);
  writeData(y);
  writeCommand(0x5C);
  writeData(RGBbit1);
  writeData(RGBbit2);
}
void fillRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t r, uint8_t g, uint8_t b){
  uint8_t RGBbit1, RGBbit2;
  RGBbit1 = (r<<3) | (g>>3);
  RGBbit2 = (g<<5) | b;

  if (x0 > x1) {
    uint8_t temp = x0;
    x0 = x1;
    x1 = temp;
  }
  if (y0 > y1) {
    uint8_t temp = y0;
    y0 = y1;
    y1 = temp;
  }

  writeCommand(0x15); //Set Column
    writeData(x0);
    writeData(x1);
  writeCommand(0x75); //Set Row
    writeData(y0);
    writeData(y1);
  writeCommand(0x5C); //Write RAM
  for(int i=x0; i<=x1; i++){
    for (int j=y0; j<=y1; j++) {
        writeData(RGBbit1);
        writeData(RGBbit2);
    }
  }
}
void fillScreen(uint8_t r, uint8_t g, uint8_t b) {
  fillRect(0,0,SCREEN_WIDTH-1, SCREEN_HEIGHT-1, r, g, b);
}
void drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t r, uint8_t g, uint8_t b) {
  bool steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    uint8_t temp = x0;
    x0 = y0;
    y0 = temp;
    temp = x1;
    x1 = y1;
    y1 = temp;
  }
  if (x0 > x1) {
    uint8_t temp = x0;
    x0 = x1;
    x1 = temp;
  }

  int8_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int8_t err = dx / 2;
  int8_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0 <= x1; x0++) {
    if (steep) {
      drawPixel(y0, x0, r, g, b);
    } else {
      drawPixel(x0, y0, r, g, b);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}
void drawHLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t r, uint8_t g, uint8_t b) {
  fillRect(x0, y0, x1, y0, r, g, b);
}
void highlightScreen() {
  drawHLine(0, 0, SCREEN_WIDTH-1, GREEN_R, GREEN_G, GREEN_B);
  drawHLine(0, 1, SCREEN_WIDTH-1, GREEN_R, GREEN_G, GREEN_B);
}
void unhighlightScreen() {
  drawHLine(0, 0, SCREEN_WIDTH-1, BACKGROUND_R, BACKGROUND_G, BACKGROUND_B);
  drawHLine(0, 1, SCREEN_WIDTH-1, BACKGROUND_R, BACKGROUND_G, BACKGROUND_B);
}
void drawVLine(uint8_t x0, uint8_t y0, uint8_t y1, uint8_t r, uint8_t g, uint8_t b) {
  fillRect(x0, y0, x0, y1, r, g, b);
}
void drawHLineFull(uint8_t y0, uint8_t r, uint8_t g, uint8_t b) {
  drawHLine(0, y0, SCREEN_WIDTH - 1, r, g, b);
}
void drawVLineFull(uint8_t x0, uint8_t r, uint8_t g, uint8_t b) {
  drawVLine(x0, 0, SCREEN_HEIGHT - 1, r, g, b);
}
void drawRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t r, uint8_t g, uint8_t b) {
  drawVLine(x0, y0, y1, r, g, b);
  drawHLine(x0, y1, x1, r, g, b);
  drawVLine(x1, y1, y0, r, g, b);
  drawHLine(x1, y0, x0, r, g, b);
}
void drawCircle(uint8_t x0, uint8_t y0, uint8_t radius, uint8_t r, uint8_t g, uint8_t b) {
  int8_t f = 1 - radius;
  int8_t ddF_x = 1;
  int8_t ddF_y = -2 * radius;
  int8_t x = 0;
  int8_t y = radius;

  drawPixel(x0, y0 + radius, r, g, b);
  drawPixel(x0, y0 - radius, r, g, b);
  drawPixel(x0 + radius, y0, r, g, b);
  drawPixel(x0 - radius, y0, r, g, b);

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    drawPixel(x0 + x, y0 + y, r, g, b);
    drawPixel(x0 - x, y0 + y, r, g, b);
    drawPixel(x0 + x, y0 - y, r, g, b);
    drawPixel(x0 - x, y0 - y, r, g, b);
    drawPixel(x0 + y, y0 + x, r, g, b);
    drawPixel(x0 - y, y0 + x, r, g, b);
    drawPixel(x0 + y, y0 - x, r, g, b);
    drawPixel(x0 - y, y0 - x, r, g, b);
  }
}
void fillCircle(uint8_t x0, uint8_t y0, uint8_t radius, uint8_t r, uint8_t g, uint8_t b) {
  int8_t f = 1 - radius;
  int8_t ddF_x = 1;
  int8_t ddF_y = -2 * radius;
  int8_t x = 0;
  int8_t y = radius;

  drawPixel(x0 + radius, y0, r, g, b);
  drawPixel(x0 - radius, y0, r, g, b);
  drawVLine(x0, y0 - radius, y0 + radius, r, g, b);
  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    drawVLine(x0 + x, y0 - y, y0 + y, r, g, b);
    drawVLine(x0 - x, y0 - y, y0 + y, r, g, b);
    drawVLine(x0 + y, y0 - x, y0 + x, r, g, b);
    drawVLine(x0 - y, y0 - x, y0 + x, r, g, b);
  }
}
void fillCircleHelper(uint8_t x0, uint8_t y0, uint8_t radius, uint8_t corners, uint8_t delta, uint8_t r, uint8_t g, uint8_t b) {
  uint8_t f = 1 - radius;
  uint8_t ddF_x = 1;
  uint8_t ddF_y = -2 * radius;
  uint8_t x = 0;
  uint8_t y = radius;
  uint8_t px = x;
  uint8_t py = y;
  delta = delta + 1;
  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    if (x < (y + 1)) {
      if (corners & 1)
        drawVLine(x0 + x, y0 - y, y0 - y + 2 * y + delta, r, g, b);
      if (corners & 2)
        drawVLine(x0 - x, y0 - y, y0 - y + 2 * y + delta, r, g, b);
    }
    if (y != py) {
      if (corners & 1)
        drawVLine(x0 + py, y0 - px, y0 - px + 2 * px + delta, r, g, b);
      if (corners & 2)
        drawVLine(x0 - py, y0 - px, y0 - px + 2 * px + delta, r, g, b);
      py = y;
    }
    px = x;
  }
}
void drawTriangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t r, uint8_t g, uint8_t b) {
  drawLine(x0, y0, x1, y1, r, g, b);
  drawLine(x2, y2, x1, y1, r, g, b);
  drawLine(x0, y0, x2, y2, r, g, b);
}

void printText(char text[100], uint8_t x0, uint8_t y0, uint8_t r, uint8_t g, uint8_t b, uint8_t scale){
  int16_t i;
  uint8_t RGBbit1, RGBbit2;
  RGBbit1 = (r<<3) | (g>>3);
  RGBbit2 = (g<<5) | b;
  int8_t lineReset = 0;
  for (int k = 0; k < strlen(text); k++) {
    while (((k - lineReset) * 4 * scale + x0 + 3 * scale) > SCREEN_WIDTH) {
       if ((y0 - 16 * scale + 1) < 0) {
         return;
       }
       lineReset = k;
       y0 = y0 - 8 * scale;
    }
    writeCommand(0x15); //Set Column
      writeData(x0 + (k - lineReset) * 4 * scale);
      writeData(x0 + (k - lineReset) * 4  * scale + (3 * scale) - 1);
    writeCommand(0x75); //Set Row
      writeData(y0 - 7 * scale + 1);
      writeData(y0);
    writeCommand(0x5C); //Write RAM

    int printing = ((int)text[k] - 65);
    if (printing < 0 && printing != -33) {
      printing = printing + 18 + 26;
    }
    if (printing > 35) {
      printing = 26;
    }
    if (printing < 0) {
      for (i= 21*scale*scale; i>0; i--) {
        writeData(0);
        writeData(0);
      }
    }
    for(i= 7 * scale - 1; i>=0; i--){
      for (int j= 3 * scale - 1; j>=0; j--) {
        int row = (int) (i / scale);
        int col = (int) ((scale * 3 - j - 1)/ scale);
        int index = col + row * 3;
        if(fonts[printing][index] == 1) {
            writeData(RGBbit1);
            writeData(RGBbit2);
        } else {
            writeData(0);
            writeData(0);
        }
      }
    }
  }
}

void SPIwrite(uint8_t c){
  writePinHigh(sclk);
  int8_t i; //signed intでなければならない。負の数になると255という値になって、例外エラーになる。
  for (i=7; i>=0; i--) {
      writePinLow(sclk);
      if (c & _BV(i)) {
          writePinHigh(mosi);
      } else {
          writePinLow(mosi);
      }
      writePinHigh(sclk);
  }
}

void writeCommand(uint8_t c) {
    writePinLow(DCpin);
    writePinLow(cs);
    SPIwrite(c);
    writePinHigh(cs);
}

void writeData(uint8_t c) {
    writePinHigh(DCpin);
    writePinLow(cs);
    SPIwrite(c);
    writePinHigh(cs);
}

void keyboard_post_init_user() {
    setupOLED();
    fillScreen(BLACK_R, BLACK_G, BLACK_B);
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        fillCircle(i * 30 + 15, j * 30 + 15, 15, WHITE_R, WHITE_G, WHITE_B);
      }
    }
    _delay_ms(1000);
    fillScreen(BLACK_R, BLACK_G, BLACK_B);
}

void matrix_init_user(void) {
}

void matrix_scan_user(void) {

}
