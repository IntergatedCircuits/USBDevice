/**
  ******************************************************************************
  * @file    usbd_ncm.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-06-02
  * @brief   USB CDC Network Control Model
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
#ifndef __USBD_NCM_H
#define __USBD_NCM_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <usbd_types.h>
#include <usbd_cdc.h>

/** @ingroup USBD
 * @addtogroup USBD_Class USBD Classes
 * @{ */

/** @defgroup USBD_NCM Network Control Model (NCM)
 * @{ */

/** @defgroup USBD_NCM_Exported_Macros NCM Exported Macros
 * @{ */

#ifndef USBD_NCM_MAX_OUT_SIZE
#define USBD_NCM_MAX_OUT_SIZE       2048
#endif
#ifndef USBD_NCM_MAX_IN_SIZE
#define USBD_NCM_MAX_IN_SIZE        2048
#endif

/** @} */

/** @defgroup USBD_NCM_Exported_Types NCM Exported Types
 * @{ */


/** @brief Network (MAC) address type */
typedef uint8_t USBD_NCM_NetAddressType[6];


/** @brief CDC application structure */
typedef struct
{
    const char* Name;               /*!< String description of the application */

    const USBD_NCM_NetAddressType* NetAddress; /*!< Fixed MAC address */

    void (*Init)        (void* itf);/*!< Initialization request */

    void (*Deinit)      (void* itf);/*!< Shutdown request */

    void (*Received)    (void* itf);/*!< Received datagrams available */

}USBD_NCM_AppType;


/** @brief NCM interface configuration */
typedef struct
{
    uint8_t OutEpNum;   /*!< OUT endpoint address */
    uint8_t InEpNum;    /*!< IN endpoint address */
    uint8_t NotEpNum;   /*!< Notification endpoint address */
}USBD_NCM_ConfigType;


/** @brief NCM class interface structure */
typedef struct
{
    USBD_IfHandleType Base;         /*!< Class-independent interface base */
    const USBD_NCM_AppType* App;    /*!< NCM application reference */
    USBD_NCM_ConfigType Config;     /*!< NCM interface configuration */
    USBD_PADDING_1();

    /* NCM class internal context */
    struct {
        USBD_CDC_NotifyHeaderType  SpeedChange;
        USBD_CDC_SpeedChangeType   SpeedData;
        USBD_CDC_NotifyHeaderType  Connection;
    }Notify;                        /*!< NCM message buffer for notifications */

    struct {
        uint32_t Data[2][USBD_NCM_MAX_OUT_SIZE / sizeof(uint32_t)];
        void*    PT;
        uint8_t  Page;
        uint8_t  Dx;
        volatile uint8_t State[2];
    }Out;                           /*!< Received NTB status and double buffer */

    struct {
        uint32_t Data[2][USBD_NCM_MAX_IN_SIZE  / sizeof(uint32_t)];
        uint32_t MaxSize;
        uint32_t RemSize;
        uint16_t Index;
        uint8_t  Page;
        uint8_t  DgCount;
        volatile uint8_t SendState;
        volatile uint8_t FillState;
    }In;                            /*!< Transmit NTB status and double buffer */
}USBD_NCM_IfHandleType;

/** @} */

/** @addtogroup USBD_NCM_Exported_Functions
 * @{ */
USBD_ReturnType USBD_NCM_MountInterface (USBD_NCM_IfHandleType *itf,
                                         USBD_HandleType *dev);

USBD_ReturnType USBD_NCM_Connect        (USBD_NCM_IfHandleType *itf,
                                         uint32_t bitrate);

USBD_ReturnType USBD_NCM_Disconnect     (USBD_NCM_IfHandleType *itf);

uint8_t*        USBD_NCM_GetDatagram    (USBD_NCM_IfHandleType *itf,
                                         uint16_t *length);

uint8_t*        USBD_NCM_AllocDatagram  (USBD_NCM_IfHandleType *itf,
                                         uint16_t length);
USBD_ReturnType USBD_NCM_SetDatagram    (USBD_NCM_IfHandleType *itf);

USBD_ReturnType USBD_NCM_PutDatagram    (USBD_NCM_IfHandleType *itf,
                                         uint8_t *data,
                                         uint16_t length);
/** @} */

/** @} */

/** @} */

#ifdef __cplusplus
}
#endif

#endif  /* __USBD_NCM_H */
