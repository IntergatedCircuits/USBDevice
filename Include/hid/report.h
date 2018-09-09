/**
  ******************************************************************************
  * @file    report.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   USB Human Interface Device Class
  *          Report Descriptor generic elements definition
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
#ifndef __HID_REPORT_H_
#define __HID_REPORT_H_
#include <stdint.h>

/* Constructing HID report items */

typedef enum {
    HID_ITEM_MAIN    = 0x00,
    HID_ITEM_GLOBAL  = 0x04,
    HID_ITEM_LOCAL   = 0x08,
}HIDItemType;

#define HID_ITEM_0(TAG,TYPE)            (((TAG) << 4) | (HID_ITEM_##TYPE))
#define HID_ITEM_1(TAG,TYPE,VAL)        (((TAG) << 4) | (HID_ITEM_##TYPE) | 0x1), (uint8_t)(VAL)
#define HID_ITEM_2(TAG,TYPE,VAL)        (((TAG) << 4) | (HID_ITEM_##TYPE) | 0x2), _HID_INTWO(VAL)
#define HID_ITEM_4(TAG,TYPE,VAL)        (((TAG) << 4) | (HID_ITEM_##TYPE) | 0x3), _HID_INFOUR(VAL)
#define HID_ITEM_LONG(TAG,SIZE,...)     0xFE, (uint8_t)(SIZE), (uint8_t)(TAG), __VA_ARGS__

#define _HID_INTWO(VAL)                 (uint8_t)((VAL) & 0xFF), (uint8_t)((VAL) >> 8)
#define _HID_INFOUR(VAL)                _HID_INTWO(VAL), _HID_INTWO((VAL) >> 16)

/* HID Report Main Items */

typedef enum
{
    Data_Arr_Abs        = 0x00,
    Const_Arr_Abs       = 0x01,
    Data_Var_Abs        = 0x02,
    Const_Var_Abs       = 0x03,
    Data_Var_Rel        = 0x06,
    Wrap_Flag           = 0x08,
    Nonlinear_Flag      = 0x10,
    NoPreferred_Flag    = 0x20,
    NullState_Flag      = 0x40,
    Volatile_Flag       = 0x80,
    BufferedBytes_Flag  = 0x100,
}HIDFlagType;

#define HID_ITEM_FLAG_CONST             0x01 /* Constant vs Data */
#define HID_ITEM_FLAG_VAR               0x02 /* Variable vs Array */
#define HID_ITEM_FLAG_RELATIVE          0x04 /* Relative vs Absolute */
#define HID_ITEM_FLAG_WRAP              0x08 /* Wrap? */
#define HID_ITEM_FLAG_NONLINEAR         0x10 /* Linear vs Non-linear */
#define HID_ITEM_FLAG_NO_PREFERRED      0x20 /* Preferred State exists? */
#define HID_ITEM_FLAG_NULL_STATE        0x40 /* NULL State exists? */
#define HID_ITEM_FLAG_VOLATILE          0x80 /* Volatile? */
#define HID_ITEM_FLAG_BUFF_BYTES        0x0100 /* Buffered bytes vs Bit-field */

#define HID_INPUT(FLAGS)                HID_ITEM_1(0x8, MAIN, FLAGS)
#define HID_OUTPUT(FLAGS)               HID_ITEM_2(0x9, MAIN, FLAGS)
#define HID_FEATURE(FLAGS)              HID_ITEM_2(0xB, MAIN, FLAGS)

#define HID_COLLECTION_PHYSICAL(...)    HID_ITEM_1(0xA, MAIN, 0x00), __VA_ARGS__ HID_ITEM_0(0xC, MAIN)
#define HID_COLLECTION_APPLICATION(...) HID_ITEM_1(0xA, MAIN, 0x01), __VA_ARGS__ HID_ITEM_0(0xC, MAIN)
#define HID_COLLECTION_LOGICAL(...)     HID_ITEM_1(0xA, MAIN, 0x02), __VA_ARGS__ HID_ITEM_0(0xC, MAIN)
#define HID_COLLECTION_REPORT(...)      HID_ITEM_1(0xA, MAIN, 0x03), __VA_ARGS__ HID_ITEM_0(0xC, MAIN)
#define HID_COLLECTION_NAMED_ARRAY(...) HID_ITEM_1(0xA, MAIN, 0x04), __VA_ARGS__ HID_ITEM_0(0xC, MAIN)
#define HID_COLLECTION_USAGE_SWITCH(...)HID_ITEM_1(0xA, MAIN, 0x05), __VA_ARGS__ HID_ITEM_0(0xC, MAIN)
#define HID_COLLECTION_USAGE_MOD(...)   HID_ITEM_1(0xA, MAIN, 0x06), __VA_ARGS__ HID_ITEM_0(0xC, MAIN)

/* HID Report Global Items */

#define HID_USAGE_PAGE_VENDOR_SPEC      HID_ITEM_1(0x0, GLOBAL, 0xFF)

#define HID_LOGICAL_MIN_8(VAL)          HID_ITEM_1(0x1, GLOBAL, VAL)
#define HID_LOGICAL_MIN_16(VAL)         HID_ITEM_2(0x1, GLOBAL, VAL)
#define HID_LOGICAL_MIN_32(VAL)         HID_ITEM_4(0x1, GLOBAL, VAL)

#define HID_LOGICAL_MAX_8(VAL)          HID_ITEM_1(0x2, GLOBAL, VAL)
#define HID_LOGICAL_MAX_16(VAL)         HID_ITEM_2(0x2, GLOBAL, VAL)
#define HID_LOGICAL_MAX_32(VAL)         HID_ITEM_4(0x2, GLOBAL, VAL)

#define HID_PHYSICAL_MIN_8(VAL)         HID_ITEM_1(0x3, GLOBAL, VAL)
#define HID_PHYSICAL_MIN_16(VAL)        HID_ITEM_2(0x3, GLOBAL, VAL)
#define HID_PHYSICAL_MIN_32(VAL)        HID_ITEM_4(0x3, GLOBAL, VAL)

#define HID_PHYSICAL_MAX_8(VAL)         HID_ITEM_1(0x4, GLOBAL, VAL)
#define HID_PHYSICAL_MAX_16(VAL)        HID_ITEM_2(0x4, GLOBAL, VAL)
#define HID_PHYSICAL_MAX_32(VAL)        HID_ITEM_4(0x4, GLOBAL, VAL)

/* valid range: -8 .. 7 */
#define HID_UNIT_EXPONENT(VAL)          HID_ITEM_1(0x5, GLOBAL, (uint8_t)((VAL) & 0xF))

/* Since [cm] and [g] are used instead of SI [m] and [kg],
 * the latter and their derived units also have an exponent defined */
#define HID_UNIT_NONE                   HID_ITEM_1(0x6, GLOBAL, 0x00000000)
#define HID_UNIT_CENTIMETER             HID_ITEM_1(0x6, GLOBAL, 0x00000011)
#define HID_UNIT_METER                  HID_UNIT_CENTIMETER
#define HID_UNIT_METER_EXP              2
#define HID_UNIT_RADIAN                 HID_ITEM_1(0x6, GLOBAL, 0x00000012)
#define HID_UNIT_DEGREE                 HID_ITEM_1(0x6, GLOBAL, 0x00000014)
#define HID_UNIT_GRAM                   HID_ITEM_2(0x6, GLOBAL, 0x00000101)
#define HID_UNIT_KILOGRAM               HID_UNIT_GRAM
#define HID_UNIT_KILOGRAM_EXP           3
#define HID_UNIT_SECOND                 HID_ITEM_2(0x6, GLOBAL, 0x00001001)
#define HID_UNIT_KELVIN                 HID_ITEM_4(0x6, GLOBAL, 0x00010001)
#define HID_UNIT_FAHRENHEIT             HID_ITEM_4(0x6, GLOBAL, 0x00010003)
#define HID_UNIT_AMPERE                 HID_ITEM_4(0x6, GLOBAL, 0x00100001)
#define HID_UNIT_CANDELA                HID_ITEM_4(0x6, GLOBAL, 0x01000001)
#define HID_UNIT_NEWTON                 HID_ITEM_2(0x6, GLOBAL, 0x0000E111)
#define HID_UNIT_NEWTON_EXP             (HID_UNIT_METER_EXP + HID_UNIT_KILOGRAM_EXP)
#define HID_UNIT_METER_PER_SEC          HID_ITEM_2(0x6, GLOBAL, 0x0000F011)
#define HID_UNIT_METER_PER_SEC_EXP      (HID_UNIT_METER_EXP)
#define HID_UNIT_METER_PER_SEC2         HID_ITEM_2(0x6, GLOBAL, 0x0000E011)
#define HID_UNIT_METER_PER_SEC2_EXP     (HID_UNIT_METER_EXP)
#define HID_UNIT_PASCAL                 HID_ITEM_2(0x6, GLOBAL, 0x0000E1F1)
#define HID_UNIT_PASCAL_EXP             (HID_UNIT_KILOGRAM_EXP - HID_UNIT_METER_EXP)
#define HID_UNIT_JOULE                  HID_ITEM_2(0x6, GLOBAL, 0x0000E121)
#define HID_UNIT_JOULE_EXP              ((HID_UNIT_METER_EXP*2) + HID_UNIT_KILOGRAM_EXP)
#define HID_UNIT_HERTZ                  HID_ITEM_2(0x6, GLOBAL, 0x0000F001)
#define HID_UNIT_DEGREE_PER_SEC         HID_ITEM_2(0x6, GLOBAL, 0x0000F014)
#define HID_UNIT_DEGREE_PER_SEC2        HID_ITEM_2(0x6, GLOBAL, 0x0000E014)
#define HID_UNIT_RADIAN_PER_SEC         HID_ITEM_2(0x6, GLOBAL, 0x0000F012)
#define HID_UNIT_RADIAN_PER_SEC2        HID_ITEM_2(0x6, GLOBAL, 0x0000E012)
#define HID_UNIT_WATT                   HID_ITEM_2(0x6, GLOBAL, 0x0000D121)
#define HID_UNIT_WATT_EXP               ((HID_UNIT_METER_EXP*2) + HID_UNIT_KILOGRAM_EXP)
#define HID_UNIT_AMPERE_PER_SEC         HID_ITEM_4(0x6, GLOBAL, 0x0010F001)
#define HID_UNIT_COULOMB                HID_ITEM_4(0x6, GLOBAL, 0x00101001)
#define HID_UNIT_FARAD                  HID_ITEM_4(0x6, GLOBAL, 0x00204FE1)
#define HID_UNIT_FARAD_EXP              ((HID_UNIT_METER_EXP*-2) - HID_UNIT_KILOGRAM_EXP)
#define HID_UNIT_GAUSS                  HID_ITEM_4(0x6, GLOBAL, 0x00F0E101)
#define HID_UNIT_GAUSS_EXP              (HID_UNIT_KILOGRAM_EXP)
#define HID_UNIT_HENRY                  HID_ITEM_4(0x6, GLOBAL, 0x00E0E121)
#define HID_UNIT_HENRY_EXP              ((HID_UNIT_METER_EXP*2) + HID_UNIT_KILOGRAM_EXP)
#define HID_UNIT_LUX                    HID_ITEM_4(0x6, GLOBAL, 0x010000E1)
#define HID_UNIT_LUX_EXP                (HID_UNIT_METER_EXP*-2)
#define HID_UNIT_OHM                    HID_ITEM_4(0x6, GLOBAL, 0x00E0D121)
#define HID_UNIT_OHM_EXP                ((HID_UNIT_METER_EXP*2) + HID_UNIT_KILOGRAM_EXP)
#define HID_UNIT_SIEMENS                HID_ITEM_4(0x6, GLOBAL, 0x00203FE1)
#define HID_UNIT_SIEMENS_EXP            ((HID_UNIT_METER_EXP*-2) - HID_UNIT_KILOGRAM_EXP)
#define HID_UNIT_TESLA                  HID_ITEM_4(0x6, GLOBAL, 0x00F0E101)
#define HID_UNIT_TESLA_EXP              (HID_UNIT_KILOGRAM_EXP)
#define HID_UNIT_VOLT                   HID_ITEM_4(0x6, GLOBAL, 0x00F0D121)
#define HID_UNIT_VOLT_EXP               ((HID_UNIT_METER_EXP*2) + HID_UNIT_KILOGRAM_EXP)
#define HID_UNIT_WEBER                  HID_ITEM_4(0x6, GLOBAL, 0x00F0E121)
#define HID_UNIT_WEBER_EXP              ((HID_UNIT_METER_EXP*2) + HID_UNIT_KILOGRAM_EXP)

#define HID_REPORT_SIZE(VAL)            HID_ITEM_1(0x7, GLOBAL, VAL)
#define HID_REPORT_ID(VAL)              HID_ITEM_1(0x8, GLOBAL, VAL)
#define HID_REPORT_COUNT(VAL)           HID_ITEM_1(0x9, GLOBAL, VAL)

#define HID_PUSH                        HID_ITEM_0(0xA, GLOBAL)
#define HID_POP                         HID_ITEM_0(0xA, GLOBAL)

/* HID Report Local Items */
#define HID_USAGE(VAL)                  HID_ITEM_1(0x0, LOCAL, VAL)

#define HID_USAGE_MIN_8(VAL)            HID_ITEM_1(0x1, LOCAL, VAL)
#define HID_USAGE_MIN_16(VAL)           HID_ITEM_2(0x1, LOCAL, VAL)

#define HID_USAGE_MAX_8(VAL)            HID_ITEM_1(0x2, LOCAL, VAL)
#define HID_USAGE_MAX_16(VAL)           HID_ITEM_2(0x2, LOCAL, VAL)

#define HID_STRING_INDEX(VAL)           HID_ITEM_1(0x7, LOCAL, VAL)

#endif /* __HID_REPORT_H_ */
