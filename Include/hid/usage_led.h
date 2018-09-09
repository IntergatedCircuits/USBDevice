/**
  ******************************************************************************
  * @file    usage_led.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   USB Human Interface Device Class
  *          LED Usage Page definitions
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
#ifndef __HID_USAGE_LED_H_
#define __HID_USAGE_LED_H_
#include <hid/report.h>

#define HID_USAGE_PAGE_LED          0x05, 0x08

#define HID_USAGE_LED_NUMLOCK       0x09, 0x01
#define HID_USAGE_LED_CAPSLOCK      0x09, 0x02
#define HID_USAGE_LED_SCROLLLOCK    0x09, 0x03
#define HID_USAGE_LED_POWER         0x09, 0x06
#define HID_USAGE_LED_MUTE          0x09, 0x09
#define HID_USAGE_LED_MICROPHONE    0x09, 0x21
#define HID_USAGE_LED_CAMERA_ON     0x09, 0x28
#define HID_USAGE_LED_CAMERA_OFF    0x09, 0x29

#endif /* __HID_USAGE_LED_H_ */
