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
#ifndef __USB_CDC_H
#define __USB_CDC_H

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

/** @defgroup USBD_CDC_Exported_Macros CDC Exported Macros
 * @{ */

/* Standard CDC control requests */
#define CDC_SEND_ENCAPSULATED_COMMAND               0x00
#define CDC_GET_ENCAPSULATED_RESPONSE               0x01
#define CDC_SET_COMM_FEATURE                        0x02
#define CDC_GET_COMM_FEATURE                        0x03
#define CDC_CLEAR_COMM_FEATURE                      0x04
#define CDC_SET_LINE_CODING                         0x20
#define CDC_GET_LINE_CODING                         0x21
#define CDC_SET_CONTROL_LINE_STATE                  0x22
#define CDC_SEND_BREAK                              0x23

/** @} */

/** @defgroup USBD_CDC_Exported_Types CDC Exported Types
 * @{ */


/** @brief CDC application structure */
typedef struct
{
    const char* Name;           /*!< String description of the application */

    void (*Init)        (void); /*!< Initialization request */

    void (*Deinit)      (void); /*!< Shutdown request */

    void (*Control)     (USB_SetupRequestType * req,
                         uint8_t * data);   /*!< Interface interaction through the control channel */

    void (*Received)    (uint8_t * data,
                         uint16_t length);  /*!< Received data available */

    void (*Transmitted) (uint8_t * data,
                         uint16_t length);  /*!< Transmission of data completed */
}USBD_CDC_AppType;


/** @brief CDC interface configuration */
typedef struct
{
    uint8_t OutEpNum;   /*!< OUT endpoint address */
    uint8_t InEpNum;    /*!< IN endpoint address */
    uint8_t NotEpNum;   /*!< Notification endpoint address
                             @note This address should be distinct and valid even if
                             the endpoint is not used. */
}USBD_CDC_ConfigType;


/** @brief CDC class interface structure */
typedef struct
{
    USBD_IfHandleType Base;         /*!< Class-independent interface base */
    const USBD_CDC_AppType* App;    /*!< CDC application reference */
    USBD_CDC_ConfigType Config;     /*!< CDC interface configuration */
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

#endif  /* __USB_CDC_H */
