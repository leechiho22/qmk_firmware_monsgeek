// Copyright 2024 yangzheng20003 (@yangzheng20003)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#define USB_POWER_EN_PIN                    B1 // USB ENABLE pin
#define LED_POWER_EN_PIN                    A5 // LED ENABLE pin
#define HS_BAT_CABLE_PIN                    A7 // USB insertion detection pin
#define HS_LED_BOOSTING_PIN                 D2 // LED BOOSTING

#define BAT_FULL_PIN                        A15
#define BAT_FULL_STATE                      1

#define HS_RGB_INDICATOR_COUNT              99
#define HS_RGB_BAT_COUNT                    1

#define MD_BT1_NAME                         "M1 V5"
#define MD_BT2_NAME                         "M1 V5"
#define MD_BT3_NAME                         "M1 V5"
#define MD_DONGLE_PRODUCT                   "M1 V5"

/* Device Connection RGB Indicator Light Index And Color */
#define HS_RGB_BLINK_INDEX_BT1              48
#define HS_RGB_BLINK_INDEX_BT2              47
#define HS_RGB_BLINK_INDEX_BT3              46
#define HS_RGB_BLINK_INDEX_2G4              45
#define HS_RGB_BLINK_INDEX_USB              44

#define HS_LBACK_COLOR_BT1                  RGB_BLUE
#define HS_LBACK_COLOR_BT2                  RGB_BLUE
#define HS_LBACK_COLOR_BT3                  RGB_BLUE
#define HS_LBACK_COLOR_2G4                  RGB_RED
#define HS_LBACK_COLOR_USB                  RGB_WHITE

#define HS_PAIR_COLOR_BT1                   RGB_BLUE
#define HS_PAIR_COLOR_BT2                   RGB_BLUE
#define HS_PAIR_COLOR_BT3                   RGB_BLUE
#define HS_PAIR_COLOR_2G4                   RGB_RED
#define HS_PAIR_COLOR_USB                   RGB_WHITE

/* Battery */
#define BATTERY_CAPACITY_LOW                15
#define BATTERY_CAPACITY_STOP               0
#define RGB_MATRIX_BAT_INDEX_MAP            {25, 26, 27, 28, 29, 30, 31, 32, 33, 34}

/* Status Indicator Lamp */
#define HS_MATRIX_BLINK_INDEX_BAT           1
#define HS_RGB_INDEX_WIN_LOCK               79

#define HS_RGB_BLINK_INDEX_WIN              53
#define HS_RGB_BLINK_INDEX_MAC              54

#define SYSTEM_WIN_PIN                      C15
#define SYSTEM_MAC_PIN                      C14

/* UART */
#define SERIAL_DRIVER                       SD3
#define SD1_TX_PIN                          C10
#define SD1_RX_PIN                          C11

/* Encoder */
#define ENCODER_MAP_KEY_DELAY               1

/* SPI */
#define SPI_DRIVER                          SPIDQ
#define SPI_SCK_PIN                         B3
#define SPI_MOSI_PIN                        B5
#define SPI_MISO_PIN                        B4

/* Flash */
#define EXTERNAL_FLASH_SPI_SLAVE_SELECT_PIN C12
#define WEAR_LEVELING_LOGICAL_SIZE          (WEAR_LEVELING_BACKING_SIZE / 2)

/* RGB Matrix */
#define RGB_MATRIX_FRAMEBUFFER_EFFECTS
#define RGB_MATRIX_KEYPRESSES

/* WS2812 */
#define WS2812_SPI_DRIVER  SPIDM2
#define WS2812_SPI_DIVISOR 32

/* rgb_record */
#define ENABLE_RGB_MATRIX_RGBR_PLAY
#define RGBREC_CHANNEL_NUM         4
#define EECONFIG_CONFINFO_USE_SIZE (4 + 16)
#define EECONFIG_RGBREC_USE_SIZE   (RGBREC_CHANNEL_NUM * MATRIX_ROWS * MATRIX_COLS * 2)
#define EECONFIG_USER_DATA_SIZE    (EECONFIG_RGBREC_USE_SIZE + EECONFIG_CONFINFO_USE_SIZE)
#define RGBREC_EECONFIG_ADDR       (uint8_t *)(EECONFIG_USER_DATABLOCK)
#define CONFINFO_EECONFIG_ADDR     (uint32_t *)((uint32_t)RGBREC_EECONFIG_ADDR + (uint32_t)EECONFIG_RGBREC_USE_SIZE)

/* All HS_RGB_INDEX */
#define HS_RGB_INDEX_FN                     1
#define HS_RGB_INDEX_GLOBE                  2   // Used to be right control
#define HS_RGB_INDEX_LEFT                   3
#define HS_RGB_INDEX_DOWN                   4
#define HS_RGB_INDEX_RIGHT                  5
#define HS_RGB_INDEX_PGDN                   6
#define HS_RGB_INDEX_PGUP                   7
#define HS_RGB_INDEX_END                    8
#define HS_RGB_INDEX_HOME                   9
#define HS_RGB_INDEX_DELETE                10
#define HS_RGB_INDEX_F12                   11
#define HS_RGB_INDEX_F11                   12
#define HS_RGB_INDEX_F10                   13
#define HS_RGB_INDEX_F9                    14
#define HS_RGB_INDEX_F8                    15
#define HS_RGB_INDEX_F7                    16
#define HS_RGB_INDEX_F6                    17
#define HS_RGB_INDEX_F5                    18
#define HS_RGB_INDEX_F4                    19
#define HS_RGB_INDEX_F3                    20
#define HS_RGB_INDEX_F2                    21
#define HS_RGB_INDEX_F1                    22
#define HS_RGB_INDEX_ESC                   23
#define HS_RGB_INDEX_GRV                   24   // `
#define HS_RGB_INDEX_1                     25
#define HS_RGB_INDEX_2                     26
#define HS_RGB_INDEX_3                     27
#define HS_RGB_INDEX_4                     28
#define HS_RGB_INDEX_5                     29
#define HS_RGB_INDEX_6                     30
#define HS_RGB_INDEX_7                     31
#define HS_RGB_INDEX_8                     32
#define HS_RGB_INDEX_9                     33
#define HS_RGB_INDEX_0                     34  
#define HS_RGB_INDEX_MINS                  35   // -
#define HS_RGB_INDEX_EQL                   36   // =
#define HS_RGB_INDEX_BSPC                  37   // Backspace
#define HS_RGB_INDEX_BSLS                  38   /* \ */
#define HS_RGB_INDEX_RBRC                  39   // ]
#define HS_RGB_INDEX_LBRC                  40   // [
#define HS_RGB_INDEX_P                     41
#define HS_RGB_INDEX_O                     42
#define HS_RGB_INDEX_I                     43
#define HS_RGB_INDEX_U                     44
#define HS_RGB_INDEX_Y                     45
#define HS_RGB_INDEX_T                     46
#define HS_RGB_INDEX_R                     47
#define HS_RGB_INDEX_E                     48
#define HS_RGB_INDEX_W                     49
#define HS_RGB_INDEX_Q                     50
#define HS_RGB_INDEX_TAB                   51
#define HS_RGB_INDEX_CAPS                  52
#define HS_RGB_INDEX_A                     53
#define HS_RGB_INDEX_S                     54
#define HS_RGB_INDEX_D                     55
#define HS_RGB_INDEX_F                     56
#define HS_RGB_INDEX_G                     57
#define HS_RGB_INDEX_H                     58
#define HS_RGB_INDEX_J                     59
#define HS_RGB_INDEX_K                     60
#define HS_RGB_INDEX_L                     61
#define HS_RGB_INDEX_SCLN                  62   // ;
#define HS_RGB_INDEX_QUOT                  63   // '
#define HS_RGB_INDEX_ENT                   64 
#define HS_RGB_INDEX_UP                    65 
#define HS_RGB_INDEX_RSFT                  66   // Right Shift
#define HS_RGB_INDEX_SLSH                  67   // ?
#define HS_RGB_INDEX_DOT                   68   // .
#define HS_RGB_INDEX_COMM                  69   // ,
#define HS_RGB_INDEX_M                     70
#define HS_RGB_INDEX_N                     71
#define HS_RGB_INDEX_B                     72
#define HS_RGB_INDEX_V                     73
#define HS_RGB_INDEX_C                     74
#define HS_RGB_INDEX_X                     75
#define HS_RGB_INDEX_Z                     76
#define HS_RGB_INDEX_LSFT                  77   // Left Shift
#define HS_RGB_INDEX_LCTL                  78   // Left Control
#define HS_RGB_INDEX_LWIN                  79   // Left Win / Option
#define HS_RGB_INDEX_LALT                  80
#define HS_RGB_INDEX_SPC                   81   // Space
#define HS_RGB_INDEX_RALT                  82

/* Pre-sleep idle countdown */
#define HS_IDLE_COUNTDOWN_TIME             (24 * 1000)  // window (ms) before sleep to run the LED VU-bar effect
