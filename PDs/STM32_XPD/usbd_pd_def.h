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

/** @addtogroup USBD_Exported_Macros
 * @{ */

#if defined(USBD_HS_SUPPORT) && !defined(USB_OTG_HS)
/* High speed support is only needed
 * if the HS core is intended for HS operation */
#undef  USBD_HS_SUPPORT
#define USBD_HS_SUPPORT                 0
#endif

/* Link Power Management support by peripheral */
#define USBD_LPM_SUPPORT                \
    (defined(USB_OTG_GLPMCFG_LPMEN) || defined(USB_LPMCSR_LPMEN))

/* USB cores set the address after the SetAddress request is completed,
 * while USB_OTG cores need the new address as soon as available */
#define USBD_SET_ADDRESS_IMMEDIATE      \
    (defined(USB_OTG_FS))

/* The maximal number of available endpoints differs for all USB cores */
#if   defined(USB)
#define USBD_MAX_EP_COUNT               8 /* Theoretical maximum */
#elif defined(USB_OTG_HS)
#define USBD_MAX_EP_COUNT               USB_OTG_HS_MAX_IN_ENDPOINTS
#elif defined(USB_OTG_FS) && defined(USB_OTG_FS_MAX_IN_ENDPOINTS)
#define USBD_MAX_EP_COUNT               USB_OTG_FS_MAX_IN_ENDPOINTS
#elif defined(USB_OTG_FS)
#define USBD_MAX_EP_COUNT               6
#endif

/* Peripheral Driver extension fields */
#if defined(USB)
#define USBD_PD_EP_FIELDS                                           \
    uint8_t             RegId;          /*!< Endpoint register ID */

#if defined(USB_BCDR_DPPU)
#define USBD_PD_DEV_FIELDS                                          \
    struct {                                                        \
    XPD_HandleCallbackType DepInit;     /*!< Initialize module dependencies */\
    XPD_HandleCallbackType DepDeinit;   /*!< Restore module dependencies */\
    XPD_HandleCallbackType Suspend;     /*!< Suspend request */     \
    XPD_HandleCallbackType Resume;      /*!< Resume request */      \
    XPD_HandleCallbackType SOF;         /*!< Start Of Frame */      \
    }Callbacks;                         /*   Handle Callbacks */
#else
#define USBD_PD_DEV_FIELDS                                          \
    struct {                                                        \
    XPD_HandleCallbackType DepInit;     /*!< Initialize module dependencies */\
    XPD_HandleCallbackType DepDeinit;   /*!< Restore module dependencies */\
    XPD_CtrlCallbackType   ConnectCtrl; /*!< Callback to set USB device bus line connection state */\
    XPD_HandleCallbackType Suspend;     /*!< Suspend request */     \
    XPD_HandleCallbackType Resume;      /*!< Resume request */      \
    XPD_HandleCallbackType SOF;         /*!< Start Of Frame */      \
    }Callbacks;                         /*   Handle Callbacks */
#endif /* USB_BCDR_DPPU */

/* Packet memory is 16 bit wide */
#define USBD_DATA_ALIGNMENT             2

#elif defined(USB_OTG_FS)

/** @brief USB peripheral PHYsical layer selection */
typedef enum {
    USB_PHY_EMBEDDED_FS = 0, /*!< Full-Speed PHY embedded in chip */
    USB_PHY_ULPI        = 1, /*!< ULPI interface to external High-Speed PHY */
    USB_PHY_EMBEDDED_HS = 2, /*!< High-Speed PHY embedded in chip */
}USB_PHYType;

#if defined(USB_OTG_HS)

#define USBD_PD_CONFIG_FIELDS                                       \
    USB_PHYType     PHY;      /*!< USB PHYsical layer selection */  \
    FunctionalState DMA;      /*!< DMA activation */

/* DMA requires word aligned data */
#define USBD_DATA_ALIGNMENT             4

#else

#define USBD_PD_CONFIG_FIELDS                                       \
    USB_PHYType     PHY;      /*!< USB PHYsical layer selection */

#define USBD_DATA_ALIGNMENT             1

#endif /* USB_OTG_HS */

#define USBD_PD_DEV_FIELDS                                          \
    USB_OTG_TypeDef * Inst;   /*!< The address of the peripheral */ \
    struct {                                                        \
    XPD_HandleCallbackType DepInit;     /*!< Initialize module dependencies */\
    XPD_HandleCallbackType DepDeinit;   /*!< Restore module dependencies */\
    XPD_HandleCallbackType Suspend;     /*!< Suspend request */     \
    XPD_HandleCallbackType Resume;      /*!< Resume request */      \
    XPD_HandleCallbackType SOF;         /*!< Start Of Frame */      \
    }Callbacks;                         /*   Handle Callbacks */

#endif

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __USBD_PD_DEF_H_ */
