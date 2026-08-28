// Copyright 2026 Yu
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#ifdef VIA_ENABLE
#    include "dynamic_keymap.h"
#    include "via.h"
#endif

enum layer_names {
    L_MAC,
    L_MAC_CUSTOM,
    L_WIN,
    L_WIN_CUSTOM,
    L_RESERVED_4,
    L_RESERVED_5,
    L_COMMON,
    L_RESERVED_7,
};

enum user_keycodes {
    LAYOUT_HELP = QK_USER_0,
    CAPS_GUI_ESC,
    CAPS_CTL_ESC,
    MAC_GLOBE,
    MAC_CUSTOM_GLOBE,
    WIN_CUSTOM_INPUT,
};

enum caps_dual_role_state {
    CAPS_DUAL_ROLE_IDLE,
    CAPS_DUAL_ROLE_PENDING,
    CAPS_DUAL_ROLE_MODIFIER,
};

enum esc_pulse_state {
    ESC_PULSE_IDLE,
    ESC_PULSE_DOWN,
    ESC_PULSE_GAP,
};

enum globe_pulse_state {
    GLOBE_PULSE_IDLE,
    GLOBE_PULSE_DOWN,
    GLOBE_PULSE_GAP,
};

enum win_input_pulse_state {
    WIN_INPUT_PULSE_IDLE,
    WIN_INPUT_PULSE_DOWN,
    WIN_INPUT_PULSE_GAP,
};

enum custom_layer_tap_action {
    CUSTOM_LAYER_TAP_NONE,
    CUSTOM_LAYER_TAP_MAC_GLOBE,
    CUSTOM_LAYER_TAP_WIN_INPUT,
};

#define CAPS_HOLD_MODIFIER_MS  500
#define CUSTOM_LAYER_CANCEL_MS 500
#define ESC_PULSE_DOWN_MS      24
#define ESC_PULSE_GAP_MS       8
#define GLOBE_PULSE_DOWN_MS    24
#define GLOBE_PULSE_GAP_MS     8
#define WIN_INPUT_PULSE_DOWN_MS 24
#define WIN_INPUT_PULSE_GAP_MS  8

static enum caps_dual_role_state    caps_dual_role_state    = CAPS_DUAL_ROLE_IDLE;
static enum esc_pulse_state         esc_pulse_state         = ESC_PULSE_IDLE;
static enum globe_pulse_state       globe_pulse_state       = GLOBE_PULSE_IDLE;
static enum win_input_pulse_state   win_input_pulse_state   = WIN_INPUT_PULSE_IDLE;
static enum custom_layer_tap_action custom_layer_tap_action = CUSTOM_LAYER_TAP_NONE;
static uint8_t                      caps_modifier           = KC_NO;
static uint8_t                      queued_esc_taps         = 0;
static uint8_t                      queued_globe_taps       = 0;
static uint8_t                      queued_win_input_taps   = 0;
static uint8_t                      custom_layer_number     = 0;
static uint16_t                     caps_hold_timer         = 0;
static uint16_t                     custom_layer_timer      = 0;
static uint16_t                     esc_pulse_timer         = 0;
static uint16_t                     globe_pulse_timer       = 0;
static uint16_t                     win_input_pulse_timer   = 0;
static bool                         mac_globe_down          = false;
static bool                         mac_globe_report_down   = false;
static bool                         custom_layer_down       = false;
static bool                         custom_layer_used       = false;
static keypos_t                     custom_layer_key;

static bool is_caps_dual_role_key(uint16_t keycode) {
    return keycode == CAPS_GUI_ESC || keycode == CAPS_CTL_ESC;
}

static void start_next_esc_pulse(void) {
    if (esc_pulse_state == ESC_PULSE_IDLE && queued_esc_taps > 0) {
        queued_esc_taps--;
        register_code(KC_ESC);
        esc_pulse_state = ESC_PULSE_DOWN;
        esc_pulse_timer = timer_read();
    }
}

static void queue_esc_tap(void) {
    if (queued_esc_taps < UINT8_MAX) {
        queued_esc_taps++;
    }
    start_next_esc_pulse();
}

static void esc_pulse_task(void) {
    switch (esc_pulse_state) {
        case ESC_PULSE_DOWN:
            if (timer_elapsed(esc_pulse_timer) >= ESC_PULSE_DOWN_MS) {
                unregister_code(KC_ESC);
                esc_pulse_state = ESC_PULSE_GAP;
                esc_pulse_timer = timer_read();
            }
            break;

        case ESC_PULSE_GAP:
            if (timer_elapsed(esc_pulse_timer) >= ESC_PULSE_GAP_MS) {
                esc_pulse_state = ESC_PULSE_IDLE;
                start_next_esc_pulse();
            }
            break;

        case ESC_PULSE_IDLE:
            start_next_esc_pulse();
            break;
    }
}

static void cancel_esc_pulses(void) {
    if (esc_pulse_state == ESC_PULSE_DOWN) {
        unregister_code(KC_ESC);
    }
    esc_pulse_state = ESC_PULSE_IDLE;
    queued_esc_taps = 0;
}

static void release_caps_modifier(void) {
    if (caps_dual_role_state == CAPS_DUAL_ROLE_MODIFIER && caps_modifier != KC_NO) {
        unregister_code(caps_modifier);
    }
    caps_dual_role_state = CAPS_DUAL_ROLE_IDLE;
    caps_modifier        = KC_NO;
    caps_hold_timer      = 0;
}

static void activate_caps_modifier(void) {
    if (caps_dual_role_state == CAPS_DUAL_ROLE_PENDING) {
        register_code(caps_modifier);
        caps_dual_role_state = CAPS_DUAL_ROLE_MODIFIER;
    } else if (caps_dual_role_state == CAPS_DUAL_ROLE_MODIFIER && !(get_mods() & MOD_BIT(caps_modifier))) {
        // A connection or OS-mode switch may have cleared QMK's report while
        // the physical Caps key is still held. Restore the modifier before
        // processing the next key.
        register_code(caps_modifier);
    }
}

static void caps_dual_role_task(void) {
    if (caps_dual_role_state == CAPS_DUAL_ROLE_PENDING && timer_elapsed(caps_hold_timer) >= CAPS_HOLD_MODIFIER_MS) {
        activate_caps_modifier();
    }
}

static bool process_caps_dual_role(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        release_caps_modifier();
        caps_modifier        = keycode == CAPS_GUI_ESC ? KC_LGUI : KC_LCTL;
        caps_dual_role_state = CAPS_DUAL_ROLE_PENDING;
        caps_hold_timer      = timer_read();
    } else {
        if (caps_dual_role_state == CAPS_DUAL_ROLE_PENDING && timer_elapsed(caps_hold_timer) >= CAPS_HOLD_MODIFIER_MS) {
            // Make the boundary deterministic even when release is processed
            // before this scan cycle's housekeeping task.
            activate_caps_modifier();
        }

        if (caps_dual_role_state == CAPS_DUAL_ROLE_PENDING) {
            queue_esc_tap();
        } else if (caps_dual_role_state == CAPS_DUAL_ROLE_MODIFIER) {
            unregister_code(caps_modifier);
        }
        caps_dual_role_state = CAPS_DUAL_ROLE_IDLE;
        caps_modifier        = KC_NO;
        caps_hold_timer      = 0;
    }

    return false;
}

static void clear_caps_dual_role(void) {
    release_caps_modifier();
    cancel_esc_pulses();
}

static void update_mac_globe_report(void) {
    bool should_be_down = mac_globe_down || globe_pulse_state == GLOBE_PULSE_DOWN;
    if (mac_globe_report_down == should_be_down) {
        return;
    }

    mac_globe_report_down = should_be_down;
    host_consumer_send(should_be_down ? AC_NEXT_KEYBOARD_LAYOUT_SELECT : 0);
}

static void start_next_globe_pulse(void) {
    if (globe_pulse_state == GLOBE_PULSE_IDLE && queued_globe_taps > 0 && !mac_globe_down) {
        queued_globe_taps--;
        globe_pulse_state = GLOBE_PULSE_DOWN;
        globe_pulse_timer = timer_read();
        update_mac_globe_report();
    }
}

static void queue_globe_tap(void) {
    if (queued_globe_taps < UINT8_MAX) {
        queued_globe_taps++;
    }
    start_next_globe_pulse();
}

static void finish_globe_pulse_before_keypress(void) {
    if (!mac_globe_down && globe_pulse_state == GLOBE_PULSE_DOWN) {
        // Do not let the synthetic Globe tap overlap a quickly following key
        // press and accidentally turn it into a Globe shortcut.
        globe_pulse_state = GLOBE_PULSE_GAP;
        globe_pulse_timer = timer_read();
        update_mac_globe_report();
    }
}

static void globe_pulse_task(void) {
    switch (globe_pulse_state) {
        case GLOBE_PULSE_DOWN:
            if (timer_elapsed(globe_pulse_timer) >= GLOBE_PULSE_DOWN_MS) {
                globe_pulse_state = GLOBE_PULSE_GAP;
                globe_pulse_timer = timer_read();
                update_mac_globe_report();
            }
            break;

        case GLOBE_PULSE_GAP:
            if (timer_elapsed(globe_pulse_timer) >= GLOBE_PULSE_GAP_MS) {
                globe_pulse_state = GLOBE_PULSE_IDLE;
                start_next_globe_pulse();
            }
            break;

        case GLOBE_PULSE_IDLE:
            start_next_globe_pulse();
            break;
    }
}

static void cancel_globe_pulses(void) {
    globe_pulse_state = GLOBE_PULSE_IDLE;
    queued_globe_taps = 0;
    update_mac_globe_report();
}

static void set_mac_globe(bool pressed) {
    if (mac_globe_down == pressed) {
        return;
    }

    mac_globe_down = pressed;
    update_mac_globe_report();

    if (!pressed && queued_globe_taps > 0 && globe_pulse_state == GLOBE_PULSE_IDLE) {
        // Preserve a visible release between a held Globe and a queued tap.
        globe_pulse_state = GLOBE_PULSE_GAP;
        globe_pulse_timer = timer_read();
    }
}

static void set_win_input_chord(bool pressed) {
    if (pressed) {
        add_weak_mods(MOD_BIT(KC_LGUI));
        add_key(KC_SPC);
    } else {
        del_key(KC_SPC);
        del_weak_mods(MOD_BIT(KC_LGUI));
    }
    send_keyboard_report();
}

static void start_next_win_input_pulse(void) {
    if (win_input_pulse_state == WIN_INPUT_PULSE_IDLE && queued_win_input_taps > 0) {
        queued_win_input_taps--;
        set_win_input_chord(true);
        win_input_pulse_state = WIN_INPUT_PULSE_DOWN;
        win_input_pulse_timer = timer_read();
    }
}

static void queue_win_input_tap(void) {
    if (queued_win_input_taps < UINT8_MAX) {
        queued_win_input_taps++;
    }
    start_next_win_input_pulse();
}

static void finish_win_input_pulse_before_keypress(void) {
    if (win_input_pulse_state == WIN_INPUT_PULSE_DOWN) {
        // Release synthetic Win+Space before a quickly following key press so
        // it cannot accidentally become a Windows-key shortcut.
        set_win_input_chord(false);
        win_input_pulse_state = WIN_INPUT_PULSE_GAP;
        win_input_pulse_timer = timer_read();
    }
}

static void win_input_pulse_task(void) {
    switch (win_input_pulse_state) {
        case WIN_INPUT_PULSE_DOWN:
            if (timer_elapsed(win_input_pulse_timer) >= WIN_INPUT_PULSE_DOWN_MS) {
                set_win_input_chord(false);
                win_input_pulse_state = WIN_INPUT_PULSE_GAP;
                win_input_pulse_timer = timer_read();
            }
            break;

        case WIN_INPUT_PULSE_GAP:
            if (timer_elapsed(win_input_pulse_timer) >= WIN_INPUT_PULSE_GAP_MS) {
                win_input_pulse_state = WIN_INPUT_PULSE_IDLE;
                start_next_win_input_pulse();
            }
            break;

        case WIN_INPUT_PULSE_IDLE:
            start_next_win_input_pulse();
            break;
    }
}

static void cancel_win_input_pulses(void) {
    if (win_input_pulse_state == WIN_INPUT_PULSE_DOWN) {
        set_win_input_chord(false);
    }
    win_input_pulse_state = WIN_INPUT_PULSE_IDLE;
    queued_win_input_taps = 0;
}

static bool is_custom_layer_keycode(uint16_t keycode) {
    return keycode == MAC_CUSTOM_GLOBE || keycode == WIN_CUSTOM_INPUT;
}

static bool is_custom_layer_position(keyrecord_t *record) {
    return record->event.key.row == custom_layer_key.row && record->event.key.col == custom_layer_key.col;
}

static void custom_layer_task(void) {
    if (custom_layer_down && !custom_layer_used && timer_elapsed(custom_layer_timer) >= CUSTOM_LAYER_CANCEL_MS) {
        // The layer was available from physical press. Crossing the threshold
        // only suppresses the tap action, providing a deliberate soft exit.
        custom_layer_used = true;
    }
}

static bool process_custom_layer_key(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        if (!custom_layer_down) {
            custom_layer_down       = true;
            custom_layer_used       = has_anykey() || has_anymod();
            custom_layer_key        = record->event.key;
            custom_layer_number     = keycode == MAC_CUSTOM_GLOBE ? L_MAC_CUSTOM : L_WIN_CUSTOM;
            custom_layer_tap_action = keycode == MAC_CUSTOM_GLOBE ? CUSTOM_LAYER_TAP_MAC_GLOBE : CUSTOM_LAYER_TAP_WIN_INPUT;
            custom_layer_timer      = timer_read();
            layer_on(custom_layer_number);
        }
    } else if (custom_layer_down && is_custom_layer_position(record)) {
        bool                         send_tap   = !custom_layer_used && timer_elapsed(custom_layer_timer) < CUSTOM_LAYER_CANCEL_MS;
        enum custom_layer_tap_action tap_action = custom_layer_tap_action;

        layer_off(custom_layer_number);
        custom_layer_down       = false;
        custom_layer_used       = false;
        custom_layer_number     = 0;
        custom_layer_tap_action = CUSTOM_LAYER_TAP_NONE;
        custom_layer_timer      = 0;

        if (send_tap) {
            if (tap_action == CUSTOM_LAYER_TAP_MAC_GLOBE) {
                queue_globe_tap();
            } else if (tap_action == CUSTOM_LAYER_TAP_WIN_INPUT) {
                queue_win_input_tap();
            }
        }
    }

    return false;
}

static void clear_custom_layer_key(void) {
    if (custom_layer_down) {
        layer_off(custom_layer_number);
    }
    custom_layer_down       = false;
    custom_layer_used       = false;
    custom_layer_number     = 0;
    custom_layer_tap_action = CUSTOM_LAYER_TAP_NONE;
    custom_layer_timer      = 0;
}

static void clear_user_key_state(void) {
    clear_caps_dual_role();
    clear_custom_layer_key();
    cancel_globe_pulses();
    cancel_win_input_pulses();
    set_mac_globe(false);
}

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
    [L_MAC] = LAYOUT_75_ansi(LT(L_COMMON, KC_ESC), KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11, KC_F12, LSG(KC_4), KC_INS, KC_DEL, KC_GRV, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_EQL, KC_BSPC, KC_PGUP, KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC, KC_RBRC, KC_BSLS, KC_PGDN, CAPS_GUI_ESC, KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT, KC_ENT, KC_HOME, KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT, KC_UP, KC_END, KC_LCTL, KC_LALT, MAC_CUSTOM_GLOBE, KC_SPC, MAC_GLOBE, KC_RGUI, MO(L_COMMON), KC_LEFT, KC_DOWN, KC_RGHT),

    [L_MAC_CUSTOM] = LAYOUT_75_ansi(_______, KC_BRID, KC_BRIU, KC_MCTL, G(KC_SPC), MAC_VOICE, MAC_DND, KC_MPRV, KC_MPLY, KC_MNXT, KC_MUTE, KC_VOLD, KC_VOLU, LSG(KC_3), QK_MACRO_0, QK_MACRO_1, HYPR(KC_GRV), HYPR(KC_1), HYPR(KC_2), HYPR(KC_3), HYPR(KC_4), HYPR(KC_5), HYPR(KC_6), HYPR(KC_7), HYPR(KC_8), HYPR(KC_9), HYPR(KC_0), HYPR(KC_MINS), HYPR(KC_EQL), _______, KC_MS_WH_UP, _______, KC_F13, KC_UP, KC_F14, KC_F15, KC_F16, _______, _______, _______, _______, _______, _______, _______, QK_MACRO_2, KC_MS_WH_DOWN, S(KC_GRV), KC_LEFT, KC_DOWN, KC_RGHT, KC_F17, KC_F18, _______, KC_PGUP, KC_HOME, KC_INS, _______, _______, _______, _______, _______, KC_F19, KC_F20, KC_JYEN, KC_RO, KC_NUBS, _______, KC_PGDN, KC_END, KC_DEL, _______, KC_MS_BTN1, KC_MS_UP, KC_MS_BTN2, _______, _______, _______, _______, _______, _______, MO(L_MAC_CUSTOM), KC_MS_LEFT, KC_MS_DOWN, KC_MS_RIGHT),

    [L_WIN] = LAYOUT_75_ansi(LT(L_COMMON, KC_ESC), KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11, KC_F12, KC_PSCR, KC_INS, KC_DEL, KC_GRV, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_EQL, KC_BSPC, KC_PGUP, KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC, KC_RBRC, KC_BSLS, KC_PGDN, CAPS_CTL_ESC, KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT, KC_ENT, KC_HOME, KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT, KC_UP, KC_END, KC_LGUI, KC_LALT, WIN_CUSTOM_INPUT, KC_SPC, KC_RCTL, KC_RGUI, MO(L_COMMON), KC_LEFT, KC_DOWN, KC_RGHT),

    [L_WIN_CUSTOM] = LAYOUT_75_ansi(_______, KC_BRID, KC_BRIU, G(KC_TAB), G(KC_S), G(KC_H), G(KC_N), KC_MPRV, KC_MPLY, KC_MNXT, KC_MUTE, KC_VOLD, KC_VOLU, _______, QK_MACRO_0, QK_MACRO_1, HYPR(KC_GRV), HYPR(KC_1), HYPR(KC_2), HYPR(KC_3), HYPR(KC_4), HYPR(KC_5), HYPR(KC_6), HYPR(KC_7), HYPR(KC_8), HYPR(KC_9), HYPR(KC_0), HYPR(KC_MINS), HYPR(KC_EQL), _______, KC_MS_WH_UP, _______, KC_F13, KC_UP, KC_F14, KC_F15, KC_F16, _______, _______, _______, _______, _______, _______, _______, QK_MACRO_2, KC_MS_WH_DOWN, S(KC_GRV), KC_LEFT, KC_DOWN, KC_RGHT, KC_F17, KC_F18, _______, KC_PGUP, KC_HOME, KC_INS, _______, _______, _______, _______, _______, KC_F19, KC_F20, KC_JYEN, KC_RO, KC_NUBS, _______, KC_PGDN, KC_END, KC_DEL, _______, KC_MS_BTN1, KC_MS_UP, KC_MS_BTN2, _______, _______, _______, _______, _______, _______, MO(L_WIN_CUSTOM), KC_MS_LEFT, KC_MS_DOWN, KC_MS_RIGHT),

    [L_RESERVED_4] = LAYOUT_75_ansi(_______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, MO(L_RESERVED_4), _______, _______, _______, _______),

    [L_RESERVED_5] = LAYOUT_75_ansi(_______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, MO(L_RESERVED_5), _______, _______, _______, _______),

    [L_COMMON] = LAYOUT_75_ansi(
        _______, HYPR(KC_F1), HYPR(KC_F2), HYPR(KC_F3), HYPR(KC_F4), _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, LNK_BLE1, LNK_BLE2, LNK_BLE3, LNK_RF, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, SIDE_MOD, SIDE_SPI, SIDE_VAI, _______, RGB_MOD, RGB_SPI, RGB_VAI, RGB_HUI, RGB_SAI, _______, DEV_RESET, SLEEP_MODE, BAT_SHOW, _______,
        KC_CAPS, SIDE_HUI, SIDE_SPD, SIDE_VAD, _______, RGB_RMOD, RGB_SPD, RGB_VAD, RGB_HUD, RGB_SAD, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, LAYOUT_HELP, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, MO(L_COMMON), _______, _______, _______),

    [L_RESERVED_7] = LAYOUT_75_ansi(_______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, MO(L_RESERVED_7), _______, _______, _______, _______),
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        finish_globe_pulse_before_keypress();
        finish_win_input_pulse_before_keypress();
    }

    if (record->event.pressed && custom_layer_down && !is_custom_layer_position(record)) {
        custom_layer_used = true;
    }

    if (is_caps_dual_role_key(keycode)) {
        return process_caps_dual_role(keycode, record);
    }

    if (record->event.pressed && caps_dual_role_state != CAPS_DUAL_ROLE_IDLE) {
        activate_caps_modifier();
    }

    if (is_custom_layer_keycode(keycode) || (custom_layer_down && is_custom_layer_position(record))) {
        return process_custom_layer_key(keycode, record);
    }

    if (keycode == MAC_GLOBE) {
        set_mac_globe(record->event.pressed);
        return false;
    }

    if (keycode == LAYOUT_HELP) {
        if (record->event.pressed) {
            SEND_STRING(LAYOUT_HELP_TEXT);
        }
        return false;
    }

    return true;
}

void housekeeping_task_user(void) {
    caps_dual_role_task();
    custom_layer_task();
    esc_pulse_task();
    globe_pulse_task();
    win_input_pulse_task();
}

void suspend_power_down_user(void) {
    clear_user_key_state();
}

void suspend_wakeup_init_user(void) {
    clear_user_key_state();
}

bool shutdown_user(bool jump_to_bootloader) {
    (void)jump_to_bootloader;
    clear_user_key_state();
    return true;
}

layer_state_t default_layer_state_set_user(layer_state_t state) {
    clear_user_key_state();
    return state;
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
