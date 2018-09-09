/**
  ******************************************************************************
  * @file    usage_power.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   USB Human Interface Device Class
  *          Power Device Usage Page definitions
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
#ifndef __HID_USAGE_POWER_H_
#define __HID_USAGE_POWER_H_
#include <hid/report.h>

#define HID_USAGE_PAGE_POWER_DEVICE     0x05, 0x84 /* Usage page: Power device */

#define HID_USAGE_PS_INAME              0x09, 0x01
#define HID_USAGE_PS_PRESENT_STATUS     0x09, 0x02
#define HID_USAGE_PS_CHANGED_STATUS     0x09, 0x03
#define HID_USAGE_PS_UPS                0x09, 0x04
#define HID_USAGE_PS_POWER_SUPPLY       0x09, 0x05
#define HID_USAGE_PS_BATTERY_SYSTEM     0x09, 0x10
#define HID_USAGE_PS_BATTERY_SYSTEM_ID  0x09, 0x11
#define HID_USAGE_PS_BATTERY            0x09, 0x12
#define HID_USAGE_PS_BATTERY_ID         0x09, 0x13
#define HID_USAGE_PS_CHARGER            0x09, 0x14
#define HID_USAGE_PS_CHARGER_ID         0x09, 0x15
#define HID_USAGE_PS_POWER_CONVERTER    0x09, 0x16
#define HID_USAGE_PS_POWER_CONVERTER_ID 0x09, 0x16
#define HID_USAGE_PS_OUTLET_SYSTEM      0x09, 0x18
#define HID_USAGE_PS_OUTLET_SYSTEM_ID   0x09, 0x19
#define HID_USAGE_PS_INPUT              0x09, 0x1A
#define HID_USAGE_PS_INPUT_ID           0x09, 0x1B
#define HID_USAGE_PS_OUTPUT             0x09, 0x1C
#define HID_USAGE_PS_OUTPUT_ID          0x09, 0x1D
#define HID_USAGE_PS_FLOW               0x09, 0x1E
#define HID_USAGE_PS_FLOW_ID            0x09, 0x1F
#define HID_USAGE_PS_OUTLET             0x09, 0x20
#define HID_USAGE_PS_OUTLET_ID          0x09, 0x21
#define HID_USAGE_PS_GANG               0x09, 0x22
#define HID_USAGE_PS_GANG_ID            0x09, 0x23
#define HID_USAGE_PS_POWER_SUMMARY      0x09, 0x24
#define HID_USAGE_PS_POWER_SUMMARY_ID   0x09, 0x25
#define HID_USAGE_PS_VOLTAGE            0x09, 0x30
#define HID_USAGE_PS_CURRENT            0x09, 0x31
#define HID_USAGE_PS_FREQUENCY          0x09, 0x32
#define HID_USAGE_PS_APPARENT_POWER     0x09, 0x33
#define HID_USAGE_PS_ACTIVE_POWER       0x09, 0x34
#define HID_USAGE_PS_TEMPERATURE        0x09, 0x36
#define HID_USAGE_PS_CONFIGVOLTAGE      0x09, 0x40
#define HID_USAGE_PS_CONFIGCURRENT      0x09, 0x41
#define HID_USAGE_PS_CONFIGFREQ         0x09, 0x42
#define HID_USAGE_PS_CONFIGAPP_POWER    0x09, 0x43
#define HID_USAGE_PS_CONFIGACT_POWER    0x09, 0x44
#define HID_USAGE_PS_CONFIGTEMPERATURE  0x09, 0x46
#define HID_USAGE_PS_SWITCHONCTRL       0x09, 0x50
#define HID_USAGE_PS_SWITCHOFFCTRL      0x09, 0x51
#define HID_USAGE_PS_TOGGLECTRL         0x09, 0x52
#define HID_USAGE_PS_PRESENT            0x09, 0x60
#define HID_USAGE_PS_GOOD               0x09, 0x61
#define HID_USAGE_PS_OVERLOAD           0x09, 0x65
#define HID_USAGE_PS_OVERCHARGED        0x09, 0x66
#define HID_USAGE_PS_OVERTEMP           0x09, 0x67
#define HID_USAGE_PS_RESERVED           0x09, 0x6A
#define HID_USAGE_PS_SWITCH_ON_OFF      0x09, 0x6B
#define HID_USAGE_PS_SWITCHABLE         0x09, 0x6C
#define HID_USAGE_PS_USED               0x09, 0x6D
#define HID_USAGE_PS_BOOST              0x09, 0x6E
#define HID_USAGE_PS_BUCK               0x09, 0x6F

#define HID_USAGE_PAGE_BATTERY_SYSTEM   0x05, 0x85 /* Usage page: Battery system */

#define HID_USAGE_BS_OUTPUT_CONNECTION  0x09, 0x16
#define HID_USAGE_BS_CHARGER_CONNECTION 0x09, 0x17
#define HID_USAGE_BS_BATTERY_INSERTION  0x09, 0x18
#define HID_USAGE_BS_CHARGING_INDICATOR 0x09, 0x1D
#define HID_USAGE_BS_CAPACITY_MODE      0x09, 0x2C
#define HID_USAGE_BS_DESIGN_CAP         0x09, 0x83
#define HID_USAGE_BS_REMAINING_CAP      0x09, 0x66
#define HID_USAGE_BS_FULLCHARGE_CAP     0x09, 0x67
#define HID_USAGE_BS_BATTERY_PRESENT    0x09, 0xD1
#define HID_USAGE_BS_TERM_CHARGE        0x09, 0x40
#define HID_USAGE_BS_TERM_DISCHARGE     0x09, 0x41
#define HID_USAGE_BS_BELOW_REMCAPLIMIT  0x09, 0x42
#define HID_USAGE_BS_CHARGING           0x09, 0x44
#define HID_USAGE_BS_DISCHARGING        0x09, 0x45
#define HID_USAGE_BS_FULLY_CHARGED      0x09, 0x46
#define HID_USAGE_BS_FULLY_DISCHARGED   0x09, 0x47
#define HID_USAGE_BS_INHIBIT_CHARGE     0x09, 0xC0

#endif /* __HID_USAGE_POWER_H_ */
