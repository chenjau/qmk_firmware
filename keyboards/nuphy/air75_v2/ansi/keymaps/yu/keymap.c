// Copyright 2026 Yu
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#ifdef VIA_ENABLE
#    include "dynamic_keymap.h"
#    include "via.h"
#endif

enum layer_names {
    L_MAC,
    L_MAC_FN,
    L_WIN,
    L_WIN_FN,
    L_RESERVED_4,
    L_RESERVED_5,
    L_COMMON,
    L_RESERVED_7,
};

enum user_keycodes {
    LAYOUT_HELP = QK_USER_0,
};

// VIA's legacy names for the JIS yen and ro keys.
#define KC_JYEN KC_INT3
#define KC_RO   KC_INT1

#define LAYOUT_HELP_TEXT          \
    "AIR75 V2 - LAYER 6 HELP\n"   \
    "Hold Cat or Esc = Layer 6\n" \
    "1 2 3 4 = BT1 BT2 BT3 RF\n"  \
    "Q/A = side mode/color\n"     \
    "W/S = side speed +/-\n"      \
    "E/D = side brightness +/-\n" \
    "T/G = main mode next/prev\n" \
    "Y/H = main speed +/-\n"      \
    "U/J = main brightness +/-\n" \
    "I/K = main hue +/-\n"        \
    "O/L = main saturation +/-\n" \
    "[ = reset (hold)\n"          \
    "] = sleep\n"                 \
    "Backslash = battery\n"       \
    "Caps = Caps Lock\n"          \
    "F1-F4 = Hyper F1-F4"

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [L_MAC] = LAYOUT_75_ansi(LT(L_COMMON, KC_ESC), KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11, KC_F12, LSG(KC_4), KC_INS, KC_DEL, KC_GRV, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_EQL, KC_BSPC, KC_PGUP, KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC, KC_RBRC, KC_BSLS, KC_PGDN, MT(MOD_LGUI, KC_ESC), KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT, KC_ENT, KC_HOME, KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT, KC_UP, KC_END, KC_LCTL, KC_LALT, MO(L_MAC_FN), KC_SPC, KC_RGUI, MO(L_COMMON), KC_RCTL, KC_LEFT, KC_DOWN, KC_RGHT),

    [L_MAC_FN] = LAYOUT_75_ansi(_______, KC_BRID, KC_BRIU, KC_MCTL, G(KC_SPC), MAC_VOICE, MAC_DND, KC_MPRV, KC_MPLY, KC_MNXT, KC_MUTE, KC_VOLD, KC_VOLU, LSG(KC_3), QK_MACRO_0, QK_MACRO_1, HYPR(KC_GRV), HYPR(KC_1), HYPR(KC_2), HYPR(KC_3), HYPR(KC_4), HYPR(KC_5), HYPR(KC_6), HYPR(KC_7), HYPR(KC_8), HYPR(KC_9), HYPR(KC_0), HYPR(KC_MINS), HYPR(KC_EQL), _______, KC_MS_WH_UP, _______, KC_F13, KC_UP, KC_F14, KC_F15, KC_F16, _______, _______, _______, _______, _______, _______, _______, QK_MACRO_2, KC_MS_WH_DOWN, S(KC_GRV), KC_LEFT, KC_DOWN, KC_RGHT, KC_F17, KC_F18, _______, KC_PGUP, KC_HOME, KC_INS, _______, _______, _______, _______, _______, KC_F19, KC_F20, KC_JYEN, KC_RO, KC_NUBS, _______, KC_PGDN, KC_END, KC_DEL, LAYOUT_HELP, KC_MS_BTN1, KC_MS_UP, KC_MS_BTN2, _______, _______, _______, _______, _______, MO(L_MAC_FN), _______, KC_MS_LEFT, KC_MS_DOWN, KC_MS_RIGHT),

    [L_WIN] = LAYOUT_75_ansi(LT(L_COMMON, KC_ESC), KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11, KC_F12, KC_PSCR, KC_INS, KC_DEL, KC_GRV, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_EQL, KC_BSPC, KC_PGUP, KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC, KC_RBRC, KC_BSLS, KC_PGDN, MT(MOD_LCTL, KC_ESC), KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT, KC_ENT, KC_HOME, KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT, KC_UP, KC_END, KC_LGUI, KC_LALT, MO(L_WIN_FN), KC_SPC, KC_RGUI, MO(L_COMMON), KC_RCTL, KC_LEFT, KC_DOWN, KC_RGHT),

    [L_WIN_FN] = LAYOUT_75_ansi(_______, KC_BRID, KC_BRIU, G(KC_TAB), G(KC_S), G(KC_H), G(KC_N), KC_MPRV, KC_MPLY, KC_MNXT, KC_MUTE, KC_VOLD, KC_VOLU, _______, QK_MACRO_0, QK_MACRO_1, HYPR(KC_GRV), HYPR(KC_1), HYPR(KC_2), HYPR(KC_3), HYPR(KC_4), HYPR(KC_5), HYPR(KC_6), HYPR(KC_7), HYPR(KC_8), HYPR(KC_9), HYPR(KC_0), HYPR(KC_MINS), HYPR(KC_EQL), _______, KC_MS_WH_UP, _______, KC_F13, KC_UP, KC_F14, KC_F15, KC_F16, _______, _______, _______, _______, _______, _______, _______, QK_MACRO_2, KC_MS_WH_DOWN, S(KC_GRV), KC_LEFT, KC_DOWN, KC_RGHT, KC_F17, KC_F18, _______, KC_PGUP, KC_HOME, KC_INS, _______, _______, _______, _______, _______, KC_F19, KC_F20, KC_JYEN, KC_RO, KC_NUBS, _______, KC_PGDN, KC_END, KC_DEL, _______, KC_MS_BTN1, KC_MS_UP, KC_MS_BTN2, _______, _______, _______, _______, _______, MO(L_WIN_FN), _______, KC_MS_LEFT, KC_MS_DOWN, KC_MS_RIGHT),

    [L_RESERVED_4] = LAYOUT_75_ansi(_______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, MO(L_RESERVED_4), _______, _______, _______, _______),

    [L_RESERVED_5] = LAYOUT_75_ansi(_______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, MO(L_RESERVED_5), _______, _______, _______, _______),

    [L_COMMON] = LAYOUT_75_ansi(_______, HYPR(KC_F1), HYPR(KC_F2), HYPR(KC_F3), HYPR(KC_F4), _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, LNK_BLE1, LNK_BLE2, LNK_BLE3, LNK_RF, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, SIDE_MOD, SIDE_SPI, SIDE_VAI, _______, RGB_MOD, RGB_SPI, RGB_VAI, RGB_HUI, RGB_SAI, _______, DEV_RESET, SLEEP_MODE, BAT_SHOW, _______, KC_CAPS, SIDE_HUI, SIDE_SPD, SIDE_VAD, _______, RGB_RMOD, RGB_SPD, RGB_VAD, RGB_HUD, RGB_SAD, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, MO(L_COMMON), _______, _______, _______, _______),

    [L_RESERVED_7] = LAYOUT_75_ansi(_______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, MO(L_RESERVED_7), _______, _______, _______, _______),
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (keycode == LAYOUT_HELP) {
        if (record->event.pressed) {
            SEND_STRING(LAYOUT_HELP_TEXT);
        }
        return false;
    }

    return true;
}

#ifdef VIA_ENABLE
void via_init_kb(void) {
    if (via_eeprom_is_valid() && via_get_layout_options() != YU_LAYOUT_REVISION) {
        dynamic_keymap_reset();
        dynamic_keymap_macro_reset();
        via_set_layout_options(YU_LAYOUT_REVISION);
    }
}
#endif
