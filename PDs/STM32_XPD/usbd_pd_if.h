/**
  ******************************************************************************
  * @file    usbd_pd_if.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   Universal Serial Bus Device Driver
  *          Peripheral Driver-specific interface function definitions
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
#ifndef __USBD_PD_IF_H_
#define __USBD_PD_IF_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <xpd_common.h>

#if   defined(USB)
#include <xpd_usb.h>

/** @addtogroup USBD_Exported_Macros
 * @{ */

#define USBD_PD_Init                    USB_vInit
#define USBD_PD_Deinit                  USB_vDeinit
#define USBD_PD_Start                   USB_vStart_IT
#define USBD_PD_Stop                    USB_vStop_IT
#define USBD_PD_SetRemoteWakeup         USB_vSetRemoteWakeup
#define USBD_PD_ClearRemoteWakeup       USB_vClearRemoteWakeup
#define USBD_PD_SetAddress              USB_vSetAddress
#define USBD_PD_CtrlEpOpen              USB_vCtrlEpOpen
#define USBD_PD_EpOpen                  USB_vEpOpen
#define USBD_PD_EpClose                 USB_vEpClose
#define USBD_PD_EpSend                  USB_vEpSend
#define USBD_PD_EpReceive               USB_vEpReceive
#define USBD_PD_EpSetStall              USB_vEpSetStall
#define USBD_PD_EpClearStall            USB_vEpClearStall
#define USBD_PD_EpFlush(HANDLE,EPNUM)   ((void)0)

/** @} */

#elif defined(USB_OTG_FS)
#include <xpd_usb_otg.h>

/** @addtogroup USBD_Exported_Macros
 * @{ */

#define USBD_PD_Init                    USB_vDevInit
#define USBD_PD_Deinit                  USB_vDevDeinit
#define USBD_PD_Start                   USB_vDevStart_IT
#define USBD_PD_Stop                    USB_vDevStop_IT
#define USBD_PD_SetRemoteWakeup         USB_vSetRemoteWakeup
#define USBD_PD_ClearRemoteWakeup       USB_vClearRemoteWakeup
#define USBD_PD_SetAddress              USB_vSetAddress
#define USBD_PD_CtrlEpOpen              USB_vCtrlEpOpen
#define USBD_PD_EpOpen                  USB_vEpOpen
#define USBD_PD_EpClose                 USB_vEpClose
#define USBD_PD_EpSend                  USB_vEpSend
#define USBD_PD_EpReceive               USB_vEpReceive
#define USBD_PD_EpSetStall              USB_vEpSetStall
#define USBD_PD_EpClearStall            USB_vEpClearStall
#define USBD_PD_EpFlush                 USB_vEpFlush

#ifndef __htonl
#define __htonl(_x)                     ((uint32_t)__REV(_x))
#endif
#ifndef __htons
#define __htons(_x)                     ((uint16_t)__REVSH(_x))
#endif

/** @} */

#endif

#ifdef __cplusplus
}
#endif

#endif /* __USBD_PD_IF_H_ */
