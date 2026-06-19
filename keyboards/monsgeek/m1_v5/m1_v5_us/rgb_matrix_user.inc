// Copyright 2024 yangzheng20003 (@yangzheng20003)
// SPDX-License-Identifier: GPL-2.0-or-later


// Declare custom effects using the RGB_MATRIX_EFFECT macro
RGB_MATRIX_EFFECT(PBT_NEON)

#ifdef ENABLE_RGB_MATRIX_RGBR_PLAY
RGB_MATRIX_EFFECT(RGBR_PLAY)
#endif

#    ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

bool RGBR_PLAY(effect_params_t *params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    extern void rgbrec_play(uint8_t led_min, uint8_t led_max);
    rgbrec_play(led_min, led_max);

    return rgb_matrix_check_finished_leds(led_max);
}


// PBT_NEON effect start here

#define NUM_PRESET_COLORS 5
#define NUM_CHASER_LEDS 4
#define INCREMENT 1
// Glitter fades this many LEDs in and the same number out each cycle.
#define NUM_GLITTER_FADE_LEDS 3


// Color presets
static const HSV PROGMEM preset_colors[] = {
    {215,  200, 255}, // PURPLE
    {0,  240, 255}, // RED
    {9,  250, 255}, // AMBER
    {85,  180, 255},  // GREEN
    {136,  197, 255}  // BLUE
};

// Pre-sleep VU bar: a monotonic color bar scrolls down the PgDn..Home column,
// one LED per second, before wireless sleep. The bar is a 20-cell stream made
// of 5 colors (purple->red) at NUM_VU_CELLS_PER_COLOR cells each. Cell 0
// (purple) enters at HOME when the window opens; the stream scrolls down and
// off the bottom until the final red cell comes to rest alone at PgDn, where it
// stays until the keyboard sleeps. The per-second step is supplied by
// hs_idle_countdown_step() in m1_v5_us.c.
#define NUM_VU_COLORS 5
#define NUM_VU_CELLS_PER_COLOR 4
#define NUM_VU_CELLS (NUM_VU_COLORS * NUM_VU_CELLS_PER_COLOR)  // 20
// Head step at which the final (red) cell rests at PgDn (bottom): last cell
// index (NUM_VU_CELLS-1) + bottom position (NUM_CHASER_LEDS-1).
#define VU_PARK_STEP ((NUM_VU_CELLS - 1) + (NUM_CHASER_LEDS - 1))  // 22
static const HSV PROGMEM vu_colors[NUM_VU_COLORS] = {
    {215, 200, 255}, // PURPLE
    {136, 197, 255}, // BLUE
    {85,  180, 255}, // GREEN
    {9,   250, 255}, // AMBER
    {0,   240, 255}, // RED
};
extern int8_t hs_idle_countdown_step(void);

typedef struct {
    HSV hot_color;
    HSV cool_color;
    HSV base_color;
} color_t;

// IDs of the 4 LEDs used for the chaser effect
static const uint8_t chaser_leds[NUM_CHASER_LEDS] PROGMEM = {HS_RGB_INDEX_PGDN, HS_RGB_INDEX_PGUP, HS_RGB_INDEX_END, HS_RGB_INDEX_HOME};
// Chaser states and config
static bool chaser_leds_state[NUM_CHASER_LEDS] = {0};
typedef struct {
    uint8_t color_index;
    uint8_t last_hit;
    uint8_t repeat_count;
    uint8_t delay;
} chaser_state_t;
static chaser_state_t chaser_state = { 0, 255, 0, 0 };

typedef struct {
    bool fade_in_active;
    bool fade_out_active;
    // The picked LEDs for one cycle, back-to-back: indices [0, NUM_GLITTER_FADE_LEDS)
    // fade in, the rest fade out. One active flag per group (not per LED) keeps the
    // state small -- a group stays active until every LED in it reaches its target.
    uint8_t target_leds[NUM_GLITTER_FADE_LEDS * 2];
} glitter_state_t;
static glitter_state_t glitter_state = { false, false, {0} };

typedef struct {
    uint8_t index;
    uint16_t duration;
} badge_state_t;
static badge_state_t badge = { 0, 0 };

#define NUM_ENTER_SEQ_LEDS 7
#define ENTER_SEQ_STEP_MS 50
#define ENTER_SEQ_FILL_MS (ENTER_SEQ_STEP_MS * NUM_ENTER_SEQ_LEDS)
#define ENTER_SEQ_HOLD_MS 300

// Keys around Enter, in lighting order
static const uint8_t enter_seq_leds[NUM_ENTER_SEQ_LEDS] PROGMEM = {
    HS_RGB_INDEX_BSLS, HS_RGB_INDEX_RBRC, HS_RGB_INDEX_LBRC,
    HS_RGB_INDEX_QUOT, HS_RGB_INDEX_SCLN, HS_RGB_INDEX_SLSH,
    HS_RGB_INDEX_DOT,
};

static const HSV PROGMEM enter_seq_colors[NUM_ENTER_SEQ_LEDS] = {
    {215, 200, 255}, // BSLS PURPLE
    {0,   240, 255}, // RBRC RED
    {20,  50,  180},  // LBRC WHITE
    {85, 180, 255}, // QUOT GREEN
    {9,   250, 255}, // SCLN AMBER
    {136, 197, 255}, // SLSH BLUE
    {20,  50,  180},  // DOT WHITE
};

typedef struct {
    bool active;
    uint32_t start; // fill-progress baseline; set once per animation, never reset by re-presses
} enter_seq_state_t;
static enter_seq_state_t enter_seq = { false, 0 };

// Array to store HSV colors of all LEDs
static HSV led_hsv[RGB_MATRIX_LED_COUNT];
static uint8_t led_color_delay = 0;

static inline bool isColorEqual(HSV a, HSV b) {
    return a.h == b.h && a.s == b.s && a.v == b.v;
}

static inline bool isRestrictedLed(uint8_t i) {
    return i == HS_RGB_INDEX_GLOBE || i == HS_RGB_INDEX_HOME || i == HS_RGB_INDEX_END || i == HS_RGB_INDEX_PGUP || i == HS_RGB_INDEX_PGDN;
}

static void adjust_hsv_towards_target(HSV* current, const HSV target, uint8_t increment) {
    
    // If the target Brightness is lower, adjust the Brightness by increment value
    if (current->v > target.v) {
        current->v = qsub8(current->v, increment);
        if (current->v < target.v) current->v = target.v;
        return;
    }
    
    // If the target Saturation is lower, adjust the Saturation by increment value
    if (current->s > target.s) {
        current->s = qsub8(current->s, increment);
        if (current->s < target.s) current->s = target.s;
        return;
    }
    
    // When hue color is not visible, skip fading color.
    if (current->s == 0 || current->v == 0) {
        current->h = target.h;
    }
    
    // Adjust the hue toward the target along the shorter way around the wheel.
    // diff is int8_t, so the uint8_t subtraction wraps to the signed shortest
    // delta in [-128, 127]: positive -> step up, negative -> step down.
    int8_t diff = target.h - current->h;
    uint8_t divisor = (7 > increment) ? (7 - increment) : 1;
    if (diff > 0) {
        if (led_color_delay % divisor == 0) current->h++;   // uint8_t wraps 255->0 automatically
        return;
    } else if (diff < 0) {
        if (led_color_delay % divisor == 0) current->h--;   // uint8_t wraps 0->255 automatically
        return;
    }
    
    // Increase the Saturation to target by increment value
    if (current->s < target.s) {
        current->s = qadd8(current->s, increment);
        if (current->s > target.s) current->s = target.s;
        return;
    }

    // Increase the Brightness to target by increment value
    if (current->v < target.v) {
        current->v = qadd8(current->v, increment);
        if (current->v > target.v) current->v = target.v;
    }
}

static void glitter_effect(effect_params_t* params) {
    // One target color per fade-in LED (fade-out just dims to black, so it needs none).
    static HSV target_hsv[NUM_GLITTER_FADE_LEDS];
    // If not currently fading, start a new cycle: pick the 6 LEDs to fade.
    if (!glitter_state.fade_in_active && !glitter_state.fade_out_active) {
        // Re-roll each pick until it is non-restricted, in range, and not already
        // chosen this cycle. (A restricted index could be one the chaser owns; an
        // out-of-range one would be an out-of-bounds write; a duplicate would make
        // two of the six animations fight over one LED.)
        for (uint8_t i = 0; i < NUM_GLITTER_FADE_LEDS * 2; i++) {
            uint8_t led;
            bool reroll;
            do {
                led = random8_max(RGB_MATRIX_LED_COUNT);
                reroll = isRestrictedLed(led);
                for (uint8_t j = 0; j < i && !reroll; j++) {
                    if (glitter_state.target_leds[j] == led) reroll = true;
                }
            } while (reroll);
            glitter_state.target_leds[i] = led;
        }

        // Give each fade-in LED its own color, re-rolling so no two share one this
        // cycle. Comparing against the colors already picked works because the
        // presets are distinct (requires NUM_GLITTER_FADE_LEDS <= NUM_PRESET_COLORS).
        for (uint8_t i = 0; i < NUM_GLITTER_FADE_LEDS; i++) {
            HSV color;
            bool reroll;
            do {
                memcpy_P(&color, &preset_colors[random8_max(NUM_PRESET_COLORS)], sizeof(HSV));
                reroll = false;
                for (uint8_t j = 0; j < i && !reroll; j++) {
                    if (isColorEqual(target_hsv[j], color)) reroll = true;
                }
            } while (reroll);
            target_hsv[i] = color;
        }

        glitter_state.fade_in_active = true;
        glitter_state.fade_out_active = true;
    }

    if (glitter_state.fade_in_active) {
        // Keep the group active until every fade-in LED has reached the target.
        bool all_done = true;
        for (uint8_t i = 0; i < NUM_GLITTER_FADE_LEDS; i++) {
            HSV* hsv = &led_hsv[glitter_state.target_leds[i]];
            adjust_hsv_towards_target(hsv, target_hsv[i], INCREMENT);
            if (!isColorEqual(*hsv, target_hsv[i])) all_done = false;
        }
        if (all_done) glitter_state.fade_in_active = false;
    }

    if (glitter_state.fade_out_active) {
        // Keep the group active until every fade-out LED has gone dark.
        bool all_done = true;
        for (uint8_t i = NUM_GLITTER_FADE_LEDS; i < NUM_GLITTER_FADE_LEDS * 2; i++) {
            HSV* hsv = &led_hsv[glitter_state.target_leds[i]];
            hsv->v = qsub8(hsv->v, INCREMENT);
            if (hsv->v != 0) all_done = false;
        }
        if (all_done) glitter_state.fade_out_active = false;
    }
}

static void reactive_effect(effect_params_t* params, color_t* color) {
    for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
        if (isRestrictedLed(i)) continue;

        uint16_t tick = 2500;
        for (int8_t j = g_last_hit_tracker.count - 1; j >= 0; j--) {
            if (g_last_hit_tracker.index[j] == i) {
                tick = g_last_hit_tracker.tick[j];
                break;
            }
        }

        if (tick < 500) {
            memcpy_P(&led_hsv[i], &color->hot_color, sizeof(HSV));
        } else if (tick < 1500) {
            adjust_hsv_towards_target(&led_hsv[i], color->cool_color, 4);
        } else if (tick < 2500) {
            memcpy_P(&led_hsv[i], &color->cool_color, sizeof(HSV));
        } else {
            adjust_hsv_towards_target(&led_hsv[i], color->base_color, 1);
        }
    }
}

static void badge_effect(effect_params_t* params) {
    HSV target;
    memcpy_P(&target, &preset_colors[badge.index], sizeof(HSV));
    if (isColorEqual(led_hsv[HS_RGB_INDEX_GLOBE], target)) {
        if (++badge.duration >= 600) {
            badge.duration = 0;
            badge.index = (badge.index + 1) % NUM_PRESET_COLORS;
        }
    } else {
        adjust_hsv_towards_target(&led_hsv[HS_RGB_INDEX_GLOBE], target, 1);
    }
}

static void chaser_reset_all(void) {
    for (uint8_t i = 0; i < NUM_CHASER_LEDS; i++) {
        chaser_leds_state[i] = false;
    }
}

static void chaser_shift_right(void) {
    for (uint8_t i = NUM_CHASER_LEDS - 1; i > 0; i--) {
        chaser_leds_state[i] = chaser_leds_state[i - 1];
    }
    chaser_leds_state[0] = false;
}

static void chaser_shift_left(void) {
    for (uint8_t i = 0; i < NUM_CHASER_LEDS - 1; i++) {
        chaser_leds_state[i] = chaser_leds_state[i + 1];
    }
    chaser_leds_state[NUM_CHASER_LEDS - 1] = false;
}

static void chaser_turn_on_next(void) {
    for (uint8_t i = 0; i < NUM_CHASER_LEDS; i++) {
        if (!chaser_leds_state[i]) {
            chaser_leds_state[i] = true;
            return;
        }
    }
}

static void chaser_turn_off_next(void) {
    for (uint8_t i = 0; i < NUM_CHASER_LEDS; i++) {
        if (chaser_leds_state[i]) {
            chaser_leds_state[i] = false;
            return;
        }
    }
}

static void chaser_change_color(void) {
    chaser_state.color_index = (chaser_state.color_index + 1) % NUM_PRESET_COLORS;
}

static bool chaser_is_all_on(void) {
    for (uint8_t i = 0; i < NUM_CHASER_LEDS; i++) {
        if (!chaser_leds_state[i]) return false;
    }
    return true;
}

static bool chaser_is_all_off(void) {
    for (uint8_t i = 0; i < NUM_CHASER_LEDS; i++) {
        if (chaser_leds_state[i]) return false;
    }
    return true;
}

// Shared chaser steps used by the Up/Down arrow keys and by the volume knob/keys
// (volume events are queued in volume_chaser_steps, defined in m1_v5_us.c).
extern int8_t volume_chaser_steps;

static void chaser_step_up(void) {
    if (chaser_is_all_off()) {
        chaser_change_color();
        chaser_leds_state[0] = true;
    } else {
        chaser_shift_right();
    }
}

static void chaser_step_down(void) {
    if (chaser_is_all_off()) {
        chaser_change_color();
        chaser_leds_state[NUM_CHASER_LEDS - 1] = true;
    } else {
        if (chaser_leds_state[0] && !chaser_leds_state[1] && chaser_leds_state[NUM_CHASER_LEDS - 1]) {
            chaser_state.color_index = ((chaser_state.color_index - 1) % NUM_PRESET_COLORS + NUM_PRESET_COLORS) % NUM_PRESET_COLORS;
        }
        chaser_shift_left();
    }
}

static void chaser_effect(effect_params_t* params) {
    // Volume knob/keys drive the chaser: consume one queued step per frame.
    if (volume_chaser_steps > 0)      { chaser_step_up();   volume_chaser_steps--; }
    else if (volume_chaser_steps < 0) { chaser_step_down(); volume_chaser_steps++; }

    uint8_t last_hit_index = g_last_hit_tracker.count - 1;
    if (g_last_hit_tracker.count > 0 && g_last_hit_tracker.tick[last_hit_index] < 255) {
        uint8_t current_hit = g_last_hit_tracker.index[last_hit_index];
        bool is_repeat = false;
        if (last_hit_index - chaser_state.repeat_count - 1 >= 0) {
            is_repeat = g_last_hit_tracker.index[last_hit_index - chaser_state.repeat_count - 1] == current_hit;
        }
        if (current_hit != chaser_state.last_hit || is_repeat) {
            if (chaser_state.delay % 50 == 0) {
                chaser_state.last_hit = current_hit;
                if (is_repeat) {
                    chaser_state.repeat_count++;        
                } else {
                    chaser_state.repeat_count = 0;
                }
                switch (current_hit) {
                    case HS_RGB_INDEX_LCTL:
                    case HS_RGB_INDEX_LWIN:
                    case HS_RGB_INDEX_LALT:
                    case HS_RGB_INDEX_GLOBE:
                    case HS_RGB_INDEX_FN:
                    case HS_RGB_INDEX_LEFT:
                    case HS_RGB_INDEX_RIGHT:
                    case HS_RGB_INDEX_RSFT:
                    case HS_RGB_INDEX_LSFT:
                    case HS_RGB_INDEX_CAPS:
                    case HS_RGB_INDEX_F11:
                    case HS_RGB_INDEX_F12:
                        break;
                    case HS_RGB_INDEX_DOWN:
                        chaser_step_down();
                        break;
                    case HS_RGB_INDEX_UP:
                        chaser_step_up();
                        break;
                    case HS_RGB_INDEX_DELETE:
                        chaser_shift_right();
                        break;
                    case HS_RGB_INDEX_BSPC:
                        if (chaser_leds_state[0] && !chaser_leds_state[1] && chaser_leds_state[NUM_CHASER_LEDS - 1]) {
                            chaser_state.color_index = ((chaser_state.color_index - 1) % NUM_PRESET_COLORS + NUM_PRESET_COLORS) % NUM_PRESET_COLORS;
                        }
                        chaser_shift_left();
                        break;
                    case HS_RGB_INDEX_ESC:
                    case HS_RGB_INDEX_ENT:
                        chaser_reset_all();
                        break;
                    default:
                        if (chaser_is_all_on()) {
                            chaser_leds_state[0] = false;
                        } else if (chaser_is_all_off()) {
                            chaser_change_color();
                            chaser_leds_state[0] = true;
                        } else if (chaser_leds_state[0] && !chaser_leds_state[NUM_CHASER_LEDS - 1]) {
                            chaser_turn_on_next();
                        } else if (!chaser_leds_state[0] && chaser_leds_state[NUM_CHASER_LEDS - 1]) {
                            chaser_turn_off_next();
                        } else {
                            chaser_shift_right();
                            chaser_leds_state[0] = true;
                            if (chaser_leds_state[0] && !chaser_leds_state[1] && chaser_leds_state[NUM_CHASER_LEDS - 1]) {
                                chaser_change_color();
                            }
                        }
                        break;
                }
            }
        }
    } else if (g_last_hit_tracker.count == 0) {
        if (chaser_state.delay % 100 == 0) {
            chaser_turn_off_next();
        }
    }
    chaser_state.delay++;
    chaser_state.delay %= 201;

    for (uint8_t i = 0; i < NUM_CHASER_LEDS; i++) {
        if (chaser_leds_state[i]) {
            uint8_t led_index = pgm_read_byte(&chaser_leds[i]);
            HSV newColor;
            if (i >= 2 && chaser_leds_state[0] && !chaser_leds_state[1] && chaser_leds_state[NUM_CHASER_LEDS - 1]) {
                newColor = preset_colors[((chaser_state.color_index - 1) % NUM_PRESET_COLORS + NUM_PRESET_COLORS) % NUM_PRESET_COLORS];
            } else {
                newColor = preset_colors[chaser_state.color_index % NUM_PRESET_COLORS];
            }
            adjust_hsv_towards_target(&led_hsv[led_index], newColor, 4);
        } else {
            uint8_t led_index = pgm_read_byte(&chaser_leds[i]);
            adjust_hsv_towards_target(&led_hsv[led_index], (HSV){0, 0, 0}, 4);
        }
    }

}

// Pre-sleep VU bar: a 20-cell colored stream (purple->red, 4 cells each) scrolls
// down the column one LED per second. `step` is the head position from the top:
// the visible LED at top-offset p shows stream cell (step - p), so cell 0 enters
// at HOME (top) at step 0, the bar scrolls down, and once the final red cell
// reaches PgDn (VU_PARK_STEP) the bar holds there until sleep. Off-stream
// positions fade to black; transitions use the shared fade for a smooth scroll.
static void countdown_effect(effect_params_t* params, int8_t step) {
    uint8_t head = (step > VU_PARK_STEP) ? VU_PARK_STEP : (uint8_t)step;  // clamp & hold at PgDn
    for (uint8_t p = 0; p < NUM_CHASER_LEDS; p++) {  // p: 0 = HOME (top) .. NUM_CHASER_LEDS-1 = PgDn (bottom)
        uint8_t led_index = pgm_read_byte(&chaser_leds[NUM_CHASER_LEDS - 1 - p]);  // chaser_leds[0] = bottom (PgDn)
        int8_t cell = (int8_t)head - (int8_t)p;  // stream cell currently at this position
        HSV target;
        if (cell >= 0 && cell < NUM_VU_CELLS) {
            memcpy_P(&target, &vu_colors[cell / NUM_VU_CELLS_PER_COLOR], sizeof(HSV));
        } else {
            target = (HSV){0, 0, 0};
        }
        adjust_hsv_towards_target(&led_hsv[led_index], target, 4);
    }
}

static void enter_seq_effect(effect_params_t* params) {
    // ms since the most recent Enter press (0xFFFF if none in tracker)
    uint16_t ent_tick = 0xFFFF;
    for (uint8_t j = 0; j < g_last_hit_tracker.count; j++) {
        if (g_last_hit_tracker.index[j] == HS_RGB_INDEX_ENT && g_last_hit_tracker.tick[j] < ent_tick) {
            ent_tick = g_last_hit_tracker.tick[j];
        }
    }

    if (!enter_seq.active) {
        if (ent_tick < ENTER_SEQ_STEP_MS) {
            enter_seq.active = true;
            enter_seq.start  = g_rgb_timer;
        } else {
            return;
        }
    }

    uint32_t fill_elapsed = g_rgb_timer - enter_seq.start;
    uint8_t lit = fill_elapsed / ENTER_SEQ_STEP_MS + 1;
    if (lit > NUM_ENTER_SEQ_LEDS) lit = NUM_ENTER_SEQ_LEDS;

    for (uint8_t i = 0; i < lit; i++) {
        uint8_t led = pgm_read_byte(&enter_seq_leds[i]);
        memcpy_P(&led_hsv[led], &enter_seq_colors[i], sizeof(HSV));
    }

    // End only when the fill is done AND no Enter press happened recently —
    // each re-press shrinks ent_tick, extending how long the keys stay lit.
    if (fill_elapsed >= ENTER_SEQ_FILL_MS && ent_tick >= ENTER_SEQ_FILL_MS + ENTER_SEQ_HOLD_MS) {
        enter_seq.active = false;
    }
}

static bool pbt_neon(effect_params_t* params, color_t* color) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    if (params->init) {
        glitter_state.fade_in_active = glitter_state.fade_out_active = false;
        enter_seq.active = false;
        led_color_delay = 0;
        for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
            led_hsv[i] = (HSV){145, 230, 0};
        }
    }

    // Run RGB effects.
    badge_effect(params);
    g_last_hit_tracker.count == 0 ? glitter_effect(params) : reactive_effect(params, color);
    // During the pre-sleep idle window, the VU bar owns the PgDn..Home column
    // (same LEDs as the chaser); otherwise run the normal chaser.
    int8_t countdown_step = hs_idle_countdown_step();
    if (countdown_step >= 0) {
        countdown_effect(params, countdown_step);
    } else {
        chaser_effect(params);
    }
    enter_seq_effect(params);

    // Set RGB color for all LEDs.
    for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
        HSV new_hsv = led_hsv[i];
        // Adjust brightness level by user using RGB_VAI / RGB_VAD
        new_hsv.v = (new_hsv.v * rgb_matrix_config.hsv.v) / 255;
        // Convert the modified HSV back to RGB
        RGB new_rgb = rgb_matrix_hsv_to_rgb(new_hsv);
        // Set the new RGB values for the LED
        rgb_matrix_set_color(i, new_rgb.r, new_rgb.g, new_rgb.b);
    }

    led_color_delay = (led_color_delay + 1) % 201;

    return rgb_matrix_check_finished_leds(led_max);
}

static bool PBT_NEON(effect_params_t* params) {
    color_t color = { 
        {20,   50,   255}, // bright WHITE
        {215,  200, 255}, // PURPLE
        {215,  200, 0}, // BLACK
    };
    return pbt_neon(params, &color);
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
