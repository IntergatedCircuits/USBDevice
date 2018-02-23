/**
  ******************************************************************************
  * @file    hid_report.h
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

#define HID_USAGE_PAGE_VENDOR_SPEC      0x05, 0xFF /* Usage page: Vendor specific */

#define HID_USAGE_MIN_8(VAL)            0x19, (uint8_t)(VAL) /* Usage minimum 8 bit */
#define HID_USAGE_MIN_16(VAL)           0x1A, _INTWO(VAL) /* Usage minimum 16 bit */

#define HID_USAGE_MAX_8(VAL)            0x29, (uint8_t)(VAL) /* Usage maximum 8 bit */
#define HID_USAGE_MAX_16(VAL)           0x2A, _INTWO(VAL) /* Usage maximum 16 bit */

#define HID_LOGICAL_MIN_8(VAL)          0x15, (uint8_t)(VAL) /* Logical minimum 8 bit */
#define HID_LOGICAL_MIN_16(VAL)         0x16, _INTWO(VAL) /* Logical minimum 16 bit */
#define HID_LOGICAL_MIN_32(VAL)         0x17, _INFOUR(VAL) /* Logical minimum 32 bit */

#define HID_LOGICAL_MAX_8(VAL)          0x25, (uint8_t)(VAL) /* Logical maximum 8 bit */
#define HID_LOGICAL_MAX_16(VAL)         0x26, _INTWO(VAL) /* Logical maximum 16 bit */
#define HID_LOGICAL_MAX_32(VAL)         0x27, _INFOUR(VAL) /* Logical maximum 32 bit */

#define HID_REPORT_SIZE(VAL)            0x75, (uint8_t)(VAL) /* Report size 8 bit */

#define HID_REPORT_COUNT(VAL)           0x95, (uint8_t)(VAL) /* Report count 8 bit */

#define HID_REPORT_ID(VAL)              0x85, (uint8_t)(VAL) /* Report identifier 8 bit */

#define HID_UNIT_EXPONENT(VAL)          0x55, (uint8_t)((VAL) & 0xF) /* Unit exponent 8 bit */

#define HID_COLLECTION_PHYSICAL         0xA1, 0x00 /* Collection: Physical (group of axes) */
#define HID_COLLECTION_APPLICATION      0xA1, 0x01 /* Collection: Application (mouse, keyboard) */
#define HID_COLLECTION_LOGICAL          0xA1, 0x02 /* Collection: Logical (interrelated data) */
#define HID_COLLECTION_REPORT           0xA1, 0x03 /* Collection: Report */
#define HID_COLLECTION_NAMED_ARRAY      0xA1, 0x04 /* Collection: Named array */
#define HID_COLLECTION_USAGE_SWITCH     0xA1, 0x05 /* Collection: Usage switch */
#define HID_COLLECTION_USAGE_MOD        0xA1, 0x03 /* Collection: Usage modifier */
#define HID_END_COLLECTION              0xc0       /* End Collection */

/* Units not directly specified here can be handled by the software.
 * Examples:
 *   degC=DegK+273.15
 *   Kilogram=Gram with UnitExponent 3
 *   %=Units not specified
 */
#define HID_UNIT_NONE                   0x65, 0x00
#define HID_UNIT_LUX                    0x67, 0xE1, 0x00, 0x00, 0x01
#define HID_UNIT_KELVIN                 0x67, 0x01, 0x00, 0x01, 0x00
#define HID_UNIT_FAHRENHEIT             0x67, 0x03, 0x00, 0x01, 0x00
#define HID_UNIT_PASCAL                 0x66, 0xF1, 0xE1
#define HID_UNIT_NEWTON                 0x66, 0x11, 0xE1
#define HID_UNIT_METERS_PER_SEC         0x66, 0x11, 0xF0
#define HID_UNIT_METERS_PER_SEC2        0x66, 0x11, 0xE0
#define HID_UNIT_FARAD                  0x67, 0xE1, 0x4F, 0x20, 0x00
#define HID_UNIT_AMPERE                 0x67, 0x01, 0x00, 0x10, 0x00
#define HID_UNIT_AMPERE_PER_SEC         0x67, 0x01, 0x10, 0x10, 0x00
#define HID_UNIT_WATT                   0x66, 0x21, 0xD1
#define HID_UNIT_HENRY                  0x67, 0x21, 0xE1, 0xE0, 0x00
#define HID_UNIT_OHM                    0x67, 0x21, 0xD1, 0xE0, 0x00
#define HID_UNIT_VOLT                   0x67, 0x21, 0xD1, 0xF0, 0x00
#define HID_UNIT_HERTZ                  0x66, 0x01, 0xF0
#define HID_UNIT_DEGREES                0x65, 0x14
#define HID_UNIT_DEGREES_PER_SEC        0x66, 0x14, 0xF0
#define HID_UNIT_DEGREES_PER_SEC2       0x66, 0x14, 0xE0
#define HID_UNIT_RADIANS                0x65, 0x12
#define HID_UNIT_RADIANS_PER_SEC        0x66, 0x12, 0xF0
#define HID_UNIT_RADIANS_PER_SEC2       0x66, 0x12, 0xE0
#define HID_UNIT_SECOND                 0x66, 0x01, 0x10
#define HID_UNIT_GAUSS                  0x67, 0x01, 0xE1, 0xF0, 0x00
#define HID_UNIT_GRAM                   0x66, 0x01, 0x01
#define HID_UNIT_CENTIMETER             0x65, 0x11

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

#define HID_INPUT(FLAGS)                0x81, (uint8_t)(FLAGS) /* Input with 1 byte */
#define HID_OUTPUT(FLAGS)               0x92, _INTWO(FLAGS) /* Output with 2 bytes */
#define HID_FEATURE(FLAGS)              0xB2, _INTWO(FLAGS) /* Feature with 2 bytes */

#define _INTWO(VAL)                     (uint8_t)((VAL) & 0xFF), (uint8_t)((VAL) >> 8)
#define _INFOUR(VAL)                    _INTWO(VAL), _INTWO((VAL) >> 16)

#endif /* __HID_REPORT_H_ */
