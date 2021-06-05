/**
  ******************************************************************************
  * @file    usbd_hid.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   USB Human Interface Device Class
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
#ifndef __USBD_HID_H
#define __USBD_HID_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <usbd_types.h>

/** @ingroup USBD
 * @addtogroup USBD_Class USBD Classes
 * @{ */

/** @defgroup USBD_HID Human Interface Device Class (HID)
 * @{ */

/** @defgroup USBD_HID_Exported_Macros HID Exported Macros
 * @{ */

#define HID_IDLE_RATE_INDEFINITE        0xFFFF

/** @} */

/** @defgroup USBD_HID_Exported_Types HID Exported Types
 * @{ */

/** @brief Standard HID control requests */
typedef enum
{
    HID_REQ_SET_REPORT      = 0x09, /*!< Send a report to the device, setting the state of
                                         input, output, or feature controls.
                                         wValue.upper = @ref USBD_HID_ReportType
                                         wValue.lower = Report ID */
    HID_REQ_GET_REPORT      = 0x01, /*!< Receive a report via the Control pipe.
                                         wValue.upper = @ref USBD_HID_ReportType
                                         wValue.lower = Report ID */
    HID_REQ_SET_IDLE        = 0x0A, /*!< Silence an Input report until the data changes
                                         or (wValue.upper * 4) ms elapses (0 => infinite).
                                         wValue.lower = Report ID (0 applies to all reports) */
    HID_REQ_GET_IDLE        = 0x02, /*!< Read the current idle rate for an Input report.
                                         wValue.lower = Report ID (0 applies to all reports) */

/* Required for Boot subclass devices only */
    HID_REQ_SET_PROTOCOL    = 0x0B, /*!< Sets the new protocol.
                                         wValue = @ref USBD_HID_ProtocolType */
    HID_REQ_GET_PROTOCOL    = 0x03, /*!< Read which @ref USBD_HID_ProtocolType is currently active. */
}HID_RequestType, USBD_HID_RequestType;


/** @brief HID standard descriptor types */
typedef enum
{
    HID_DESC_TYPE_HID       = 0x21, /*!< HID class descriptor */
    HID_DESC_TYPE_REPORT    = 0x22, /*!< HID report descriptor */
    HID_DESC_TYPE_PHYSICAL  = 0x23, /*!< HID physical descriptor */
}HID_DescriptorType, USBD_HID_DescriptorType;


/** @brief HID report types */
typedef enum
{
    HID_REPORT_INPUT    = 0x01, /*!< Report sent by the device */
    HID_REPORT_OUTPUT   = 0x02, /*!< Report sent to the device */
    HID_REPORT_FEATURE  = 0x03, /*!< Bidirectional configuration report */
}HID_ReportType, USBD_HID_ReportType;


/** @brief HID protocol types */
typedef enum
{
    HID_PROTOCOL_REPORT     = 0x01, /*!< Default HID protocol */
    HID_PROTOCOL_BOOT       = 0x00, /*!< BOOT protocol */
}HID_ProtocolType, USBD_HID_ProtocolType;


/** @brief HID report configuration structure */
typedef struct {
    const uint8_t*  Desc;       /*!< Pointer to the report descriptor */
    uint16_t        DescLength; /*!< Byte size of the report descriptor */
    uint8_t         MaxId;      /*!< Largest used report ID (0 if report IDs aren't used) */
    struct {
        uint8_t     Interval_ms;/*!< Input frame interval in ms */
        uint16_t    MaxSize;    /*!< Maximal input report size */
    }Input;
    struct {
        uint16_t    MaxSize;    /*!< Maximal feature report size */
    }Feature;
    struct {
        uint8_t     Interval_ms;/*!< Output frame interval in ms */
        uint16_t    MaxSize;    /*!< Maximal output report size */
    }Output;
}HID_ReportConfigType, USBD_HID_ReportConfigType;


/** @brief HID application structure */
typedef struct
{
    const char* Name;                           /*!< String description of the application */

    const HID_ReportConfigType* Report;         /*!< Report configuration */

    void (*Init)            (void* itf);        /*!< Initialization request */

    void (*Deinit)          (void* itf);        /*!< Shutdown request */

    void (*SetReport)       (void* itf,
                             USBD_HID_ReportType type,
                             uint8_t * data,
                             uint16_t length);  /*!< Process a received report */

    void (*GetReport)       (void* itf,
                             USBD_HID_ReportType type,
                             uint8_t reportId); /*!< A report transmission is requested */

    void (*SetIdle)         (void* itf,
                             uint16_t idleRate_ms,
                             uint8_t reportId); /*!< Limit the IN reporting frequency */

    void (*InReportSent)    (void* itf, uint8_t reportId); /*!< The last IN report transmit is complete */

#if (USBD_HID_REPORT_STRINGS != 0)
    const char* (*GetString)(void* itf,
                             uint8_t intNum);   /*!< Return a string from the report description */

#endif
}HID_AppType, USBD_HID_AppType;


/** @brief HID interface configuration */
typedef struct
{
    uint8_t InEpNum;    /*!< IN endpoint address */
#if (USBD_HID_OUT_SUPPORT == 1)
    uint8_t OutEpNum;   /*!< OUT endpoint address */
#endif
}USBD_HID_ConfigType;


/** @brief HID class interface structure */
typedef struct
{
    USBD_IfHandleType Base;         /*!< Class-independent interface base */
    const USBD_HID_AppType* App;    /*!< HID application reference */
    USBD_HID_ConfigType Config;     /*!< HID interface configuration */

    uint8_t IdleRate;               /*!< Contains the current idle rate
                                         @note Report ID separate idle rates are
                                         not readable with the current API. */
    volatile uint8_t Request;       /*!< Holds the @ref USBD_HID_ReportType during
                                         control report transfers, otherwise it is 0 */
}USBD_HID_IfHandleType;

/** @} */

/** @addtogroup USBD_HID_Exported_Functions
 * @{ */
USBD_ReturnType USBD_HID_MountInterface (USBD_HID_IfHandleType *itf,
                                         USBD_HandleType *dev);

USBD_ReturnType USBD_HID_ReportIn       (USBD_HID_IfHandleType *itf,
                                         void *data,
                                         uint16_t length);

#if (USBD_HID_OUT_SUPPORT == 1)
USBD_ReturnType USBD_HID_ReportOut      (USBD_HID_IfHandleType *itf,
                                         void *data,
                                         uint16_t length);
#endif
/** @} */

/** @} */

/** @} */

#ifdef __cplusplus
}
#endif

#endif  /* __USBD_HID_H */
