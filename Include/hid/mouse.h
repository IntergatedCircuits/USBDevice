/**
  ******************************************************************************
  * @file    mouse.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-09-04
  * @brief   USB Human Interface Device Class
  *          Mouse report definitions
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
#ifndef __HID_MOUSE_H_
#define __HID_MOUSE_H_
#include <HID/report.h>
#include <HID/usage_button.h>
#include <HID/usage_desktop.h>

/** @ingroup USBD_HID
 * @addtogroup USBD_HID_Common_Reports Common HID Reports
 * @{ */

/** @brief HID mouse report descriptor with scroll wheel */
#define HID_MOUSE_REPORT_DESC(...)      \
HID_USAGE_PAGE_DESKTOP,                 \
    HID_USAGE_DT_MOUSE,                 \
    HID_COLLECTION_APPLICATION(         \
        HID_USAGE_DT_POINTER,           \
        HID_COLLECTION_PHYSICAL(        \
HID_USAGE_PAGE_BUTTON,                  \
            /* 3  buttons */            \
            HID_USAGE_MIN_8(0),         \
            HID_USAGE_MAX_8(3),         \
            HID_LOGICAL_MIN_8(0),       \
            HID_LOGICAL_MAX_8(1),       \
            HID_REPORT_SIZE(1),         \
            HID_REPORT_COUNT(3),        \
            HID_INPUT(Data_Var_Abs),    \
            HID_REPORT_SIZE(5),         \
            HID_REPORT_COUNT(1),        \
            HID_OUTPUT(Const_Arr_Abs),  \
HID_USAGE_PAGE_DESKTOP,                 \
            /* X-Y movement + wheel */  \
            HID_USAGE_DT_X,             \
            HID_USAGE_DT_Y,             \
            HID_USAGE_DT_WHEEL,         \
            HID_LOGICAL_MIN_8(-127),    \
            HID_LOGICAL_MAX_8(127),     \
            HID_REPORT_SIZE(8),         \
            HID_REPORT_COUNT(3),        \
            HID_INPUT(Data_Var_Rel),    \
        ),                              \
        __VA_ARGS__)


/** @brief Remote motion wakeup extension for @ref HID_MOUSE_WAKEUP_DESC */
#define HID_MOUSE_WAKEUP_DESC           \
        HID_USAGE_DT_MOTION_WAKEUP,     \
HID_USAGE_PAGE_VENDOR_SPEC,             \
        /* 2 flags configuration */     \
        HID_USAGE(1),                   \
        HID_LOGICAL_MIN_8(0),           \
        HID_LOGICAL_MAX_8(1),           \
        HID_REPORT_SIZE(1),             \
        HID_REPORT_COUNT(2),            \
        HID_FEATURE(Data_Var_Abs |      \
                    NoPreferred_Flag),  \
        HID_REPORT_SIZE(6),             \
        HID_REPORT_COUNT(1),            \
        HID_FEATURE(Const_Arr_Abs),


/** @brief HID mouse IN report layout */
typedef struct {
    union {
        struct {
            uint8_t left : 1;
            uint8_t right : 1;
            uint8_t mid : 1;
            uint8_t : 5;
        };
        uint8_t buttons;
    };
    uint8_t x;
    uint8_t y;
    uint8_t wheel;
}HID_KeyReportType;

/** @} */

#endif /* __HID_MOUSE_H_ */
