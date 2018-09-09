/**
  ******************************************************************************
  * @file    keyboard.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-09-04
  * @brief   USB Human Interface Device Class
  *          Keyboard report definitions
  *
  * Copyright (c) 2018 Benedek Kupper
  *
  * Licensed under the Apache License, Version 2.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *     http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */
#ifndef __HID_KEYBOARD_H_
#define __HID_KEYBOARD_H_
#include <hid/report.h>
#include <hid/usage_desktop.h>
#include <hid/usage_keyboard.h>
#include <hid/usage_led.h>

/** @ingroup USBD_HID
 * @addtogroup USBD_HID_Common_Reports Common HID Reports
 * @{ */

/** @brief HID keyboard report descriptor */
#define HID_KEYBOARD_REPORT_DESC(...)   \
HID_USAGE_PAGE_DESKTOP,                 \
    HID_USAGE_DT_KEYBOARD,              \
    HID_COLLECTION_APPLICATION(         \
HID_USAGE_PAGE_KEYBOARD,                \
        /* Key modifier flags */        \
        HID_USAGE_MIN_8(KLeftCtrl),     \
        HID_USAGE_MAX_8(KRightGUI),     \
        HID_LOGICAL_MIN_8(0),           \
        HID_LOGICAL_MAX_8(1),           \
        HID_REPORT_SIZE(1),             \
        HID_REPORT_COUNT(8),            \
        HID_INPUT(Data_Var_Abs),        \
        /* Reserved byte */             \
        HID_REPORT_SIZE(8),             \
        HID_REPORT_COUNT(1),            \
        HID_INPUT(Const_Arr_Abs),       \
        /* 6 keys buffer */             \
        HID_USAGE_MIN_8(KNone),         \
        HID_USAGE_MAX_8(KApplication),  \
        HID_LOGICAL_MIN_8(KNone),       \
        HID_LOGICAL_MAX_8(KApplication),\
        HID_REPORT_SIZE(8),             \
        HID_REPORT_COUNT(6),            \
        HID_INPUT(Data_Arr_Abs),        \
        __VA_ARGS__)


/** @brief LED indicator extension for @ref HID_KEYBOARD_REPORT_DESC */
#define HID_KEYBOARD_LED_DESC           \
        __HID_KEYBOARD_REPORT_DESC,     \
HID_USAGE_PAGE_LED,                     \
            /* LED indicators */        \
            HID_USAGE_MIN_8(1),         \
            HID_USAGE_MAX_8(5),         \
            HID_REPORT_SIZE(1),         \
            HID_REPORT_COUNT(5),        \
            HID_OUTPUT(Data_Var_Abs),   \
            HID_REPORT_SIZE(3),         \
            HID_REPORT_COUNT(1),        \
            HID_OUTPUT(Const_Arr_Abs),


/** @brief HID keyboard IN report layout */
typedef struct {
    union {
        struct {
            uint8_t lctrl : 1;
            uint8_t lshift : 1;
            uint8_t lalt : 1;
            uint8_t lgui : 1;
            uint8_t rctrl : 1;
            uint8_t rshift : 1;
            uint8_t ralt : 1;
            uint8_t rgui : 1;
        };
        uint8_t mod;
    };
    uint8_t reserved;
    uint8_t key[6];
}HID_KeyReportType;


/** @brief HID keyboard OUT report layout (LEDs for ~locks) */
typedef union {
    struct {
        uint8_t num : 1;
        uint8_t caps : 1;
        uint8_t scroll : 1;
        uint8_t compose : 1;
        uint8_t kana : 1;
        uint8_t : 3;
    };
    uint8_t b;
}HID_KeyLedReportType;

/** @} */

#endif /* __HID_KEYBOARD_H_ */
