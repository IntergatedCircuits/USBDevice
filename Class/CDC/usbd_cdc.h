/**
  ******************************************************************************
  * @file    usbd_cdc.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   USB Communications Device Class
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
#ifndef __USBD_CDC_H
#define __USBD_CDC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <usbd_types.h>

/** @ingroup USBD
 * @addtogroup USBD_Class USBD Classes
 * @{ */

/** @defgroup USBD_CDC Communications Device Class (CDC)
 * @{ */

/** @defgroup USBD_CDC_Exported_Types CDC Exported Types
 * @{ */

/** @brief Standard CDC control requests */
typedef enum
{
    CDC_REQ_SEND_ENCAPSULATED_COMMAND   = 0x00, /*!< Host command in the supported control protocol format */
    CDC_REQ_GET_ENCAPSULATED_RESPONSE   = 0x01, /*!< Device response in the supported control protocol format */
    CDC_REQ_SET_COMM_FEATURE            = 0x02, /*!< Change a feature (indexed by wIndex) */
    CDC_REQ_GET_COMM_FEATURE            = 0x03, /*!< Read a feature (indexed by wIndex) */
    CDC_REQ_CLEAR_COMM_FEATURE          = 0x04, /*!< Reset a feature (indexed by wIndex) to default state */
    CDC_REQ_SET_LINE_CODING             = 0x20, /*!< Set asynchronous line-character formatting properties */
    CDC_REQ_GET_LINE_CODING             = 0x21, /*!< Read the current line coding */
    CDC_REQ_SET_CONTROL_LINE_STATE      = 0x22, /*!< Generate RS-232 control signals
                                                     (wValue = 0b[RTS activation][DTR activation]) */
    CDC_REQ_SEND_BREAK                  = 0x23, /*!< Generates an RS-232 style break
                                                     (wValue = break length in ms,
                                                     if = 0xFFFF it is continuous until
                                                     another request with 0 is received) */
}USBD_CDC_RequestType;


/** @brief CDC line coding configuration structure */
typedef struct
{
    uint32_t DTERate;       /*!< Data terminal rate, in bits per second */
    uint8_t  CharFormat;    /*!< Stop bits:
                                    @arg 0 -> 1 Stop bit
                                    @arg 1 -> 1.5 Stop bits
                                    @arg 2 -> 2 Stop bits */
    uint8_t  ParityType;    /*!< Parity:
                                    @arg 0 -> None
                                    @arg 1 -> Odd
                                    @arg 2 -> Even
                                    @arg 3 -> Mark
                                    @arg 4 -> Space */
    uint8_t  DataBits;      /*!< Data bits: (5, 6, 7, 8 or 16) */
}__packed USBD_CDC_LineCodingType;


/** @brief CDC application structure */
typedef struct
{
    const char* Name;           /*!< String description of the application */

    void (*Open)        (USBD_CDC_LineCodingType * coding); /*!< Open port */

    void (*Close)       (void); /*!< Close port */

#if (USBD_CDC_CONTROL == 1)
    void (*Control)     (USB_SetupRequestType * req,
                         uint8_t * data);   /*!< Interface interaction through the control channel */
#endif /* USBD_CDC_CONTROL */

    void (*Received)    (uint8_t * data,
                         uint16_t length);  /*!< Received data available */

    void (*Transmitted) (uint8_t * data,
                         uint16_t length);  /*!< Transmission of data completed */
}USBD_CDC_AppType;


/** @brief CDC interface configuration */
typedef struct
{
    uint8_t Protocol;   /*!< Protocol used for Control requests */
    uint8_t OutEpNum;   /*!< OUT endpoint address */
    uint8_t InEpNum;    /*!< IN endpoint address */
    uint8_t NotEpNum;   /*!< Notification endpoint address
                             @note This address should be distinct and valid even if
                             the endpoint is not used. */
}USBD_CDC_ConfigType;


/** @brief CDC class interface structure */
typedef struct
{
    USBD_IfHandleType Base;             /*!< Class-independent interface base */
    const USBD_CDC_AppType* App;        /*!< CDC application reference */
    USBD_CDC_ConfigType Config;         /*!< CDC interface configuration */
    USBD_CDC_LineCodingType LineCoding; /*!< CDC line coding */
}USBD_CDC_IfHandleType;

/** @} */

/** @addtogroup USBD_CDC_Exported_Functions
 * @{ */
USBD_ReturnType USBD_CDC_MountInterface (USBD_CDC_IfHandleType *itf,
                                         USBD_HandleType *dev);

USBD_ReturnType USBD_CDC_Transmit       (USBD_CDC_IfHandleType *itf,
                                         uint8_t *data,
                                         uint16_t length);

USBD_ReturnType USBD_CDC_Receive        (USBD_CDC_IfHandleType *itf,
                                         uint8_t *data,
                                         uint16_t length);
/** @} */

/** @} */

/** @} */

#ifdef __cplusplus
}
#endif

#endif  /* __USBD_CDC_H */
