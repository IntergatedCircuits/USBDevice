/**
  ******************************************************************************
  * @file    usbd_pd_def.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   Universal Serial Bus Device Driver
  *          Peripheral Driver-specific constant and type definitions
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
#ifndef __USBD_PD_DEF_H_
#define __USBD_PD_DEF_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <usbd_config.h>
#include <xpd_common.h>

/** @addtogroup USBD
 * @{ */

/** @addtogroup USBD_Exported_Macros USBD Exported Macros
 * @{ */

#if defined(USBD_HS_SUPPORT) && !defined(USB_OTG_HS)
/* High speed support is only needed
 * if the HS core is intended for HS operation */
#undef  USBD_HS_SUPPORT
#define USBD_HS_SUPPORT                 0
#endif

/* Link Power Management support by peripheral */
#define USBD_LPM_SUPPORT                \
    (defined(USB_OTG_GLPMCFG_LPMEN) || defined(USB_LPMCSR_LMPEN))

/* The maximal number of available endpoints differs for all USB cores */
#if   defined(USB)
#define USBD_MAX_EP_COUNT         8 /* Theoretical maximum */
#elif defined(USB_OTG_HS) || defined(USB_OTG_GLPMCFG_LPMEN)
#define USBD_MAX_EP_COUNT         6
#else /* 1st gen USB_OTG_FS */
#define USBD_MAX_EP_COUNT         4
#endif

#if defined(USB) && !defined(USB_BCDR_DPPU)
#define USBD_PD_EP_FIELDS                                           \
    uint8_t             RegId;          /*!< Endpoint register ID */\
    uint16_t            PacketAddress;  /*!< PMA Address [0..1024] */

#define USBD_PD_CONFIG_FIELDS

#define USBD_PD_DEV_FIELDS                                          \
    struct {                                                        \
    XPD_HandleCallbackType DepInit;     /*!< Initialize module dependencies */\
    XPD_HandleCallbackType DepDeinit;   /*!< Restore module dependencies */\
    XPD_CtrlCallbackType    ConnectCtrl;/*!< Callback to set USB device bus line connection state */\
    XPD_HandleCallbackType Suspend;     /*!< Suspend request */     \
    XPD_HandleCallbackType Resume;      /*!< Resume request */      \
    XPD_HandleCallbackType SOF;         /*!< Start Of Frame */      \
    }Callbacks;                         /*   Handle Callbacks */    \
    union {                                                         \
        struct {                                                    \
            uint8_t EsofEn : 1;                                     \
            uint8_t Timer  : 7;                                     \
        };                                                          \
        uint8_t b;                                                  \
    }RemoteWakeup;

#elif defined(USB)
#define USBD_PD_EP_FIELDS                                           \
    uint8_t             RegId;          /*!< Endpoint register ID */\
    uint16_t            PacketAddress;  /*!< PMA Address [0..1024] */

#define USBD_PD_DEV_FIELDS                                          \
    struct {                                                        \
    XPD_HandleCallbackType DepInit;     /*!< Initialize module dependencies */\
    XPD_HandleCallbackType DepDeinit;   /*!< Restore module dependencies */\
    XPD_HandleCallbackType Suspend;     /*!< Suspend request */     \
    XPD_HandleCallbackType Resume;      /*!< Resume request */      \
    XPD_HandleCallbackType SOF;         /*!< Start Of Frame */      \
    }Callbacks;                         /*   Handle Callbacks */    \
    union {                                                         \
        struct {                                                    \
            uint8_t EsofEn : 1;                                     \
            uint8_t Timer  : 7;                                     \
        };                                                          \
        uint8_t b;                                                  \
    }RemoteWakeup;

#elif defined(USB_OTG_FS)
#define USBD_PD_EP_FIELDS                                          \
    uint16_t            FifoSize;       /*!< Data FIFO size */

#define USBD_PD_DEV_FIELDS                                         \
      USB_OTG_TypeDef * Inst;   /*!< The address of the peripheral instance used by the handle */\
      FunctionalState DMA;      /*!< DMA activation */
#endif

/** @} */

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __USBD_PD_DEF_H_ */
