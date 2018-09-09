/**
  ******************************************************************************
  * @file    usage_desktop.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   USB Human Interface Device Class
  *          Desktop Usage Page definitions
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
#ifndef __HID_USAGE_DESKTOP_H_
#define __HID_USAGE_DESKTOP_H_
#include <hid/report.h>

#define HID_USAGE_PAGE_DESKTOP          0x05, 0x01 /* Usage page: Generic Desktop */

#define HID_USAGE_DT_POINTER            0x09, 0x01
#define HID_USAGE_DT_MOUSE              0x09, 0x02
#define HID_USAGE_DT_KEYBOARD           0x09, 0x06
#define HID_USAGE_DT_KEYPAD             0x09, 0x07

#define HID_USAGE_DT_X                  0x09, 0x30
#define HID_USAGE_DT_Y                  0x09, 0x31
#define HID_USAGE_DT_Z                  0x09, 0x32
#define HID_USAGE_DT_RX                 0x09, 0x33
#define HID_USAGE_DT_RY                 0x09, 0x34
#define HID_USAGE_DT_RZ                 0x09, 0x35
#define HID_USAGE_DT_WHEEL              0x09, 0x38
#define HID_USAGE_DT_MOTION_WAKEUP      0x09, 0x3C

#define HID_USAGE_DT_RESOLUTION_MULT    0x09, 0x48

#define HID_USAGE_DT_SYS_POWER_DOWN     0x09, 0x81
#define HID_USAGE_DT_SYS_SLEEP          0x09, 0x82
#define HID_USAGE_DT_SYS_WAKEUP         0x09, 0x83


#endif /* __HID_USAGE_DESKTOP_H_ */
