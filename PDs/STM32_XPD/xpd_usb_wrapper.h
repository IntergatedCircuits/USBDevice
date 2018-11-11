/**
  ******************************************************************************
  * @file    xpd_usb_wrapper.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   STM32 eXtensible Peripheral Drivers Universal Serial Bus Module
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
#ifndef __XPD_USB_WRAPPER_H_
#define __XPD_USB_WRAPPER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <usbd_types.h>

/** @addtogroup USB
 * @{ */

/** @addtogroup USB_Imported_Types
 * @{ */

typedef USBD_HandleType         USB_HandleType;
typedef USBD_EpHandleType       USB_EndPointHandleType;
typedef USBD_ConfigurationType  USB_InitType;

/** @} */

#ifndef USB_vResetCallback
#define USB_vResetCallback              USBD_ResetCallback
#endif
#ifndef USB_vSetupCallback
#define USB_vSetupCallback              USBD_SetupCallback
#endif
#ifndef USB_vDataInCallback
#define USB_vDataInCallback             USBD_EpInCallback
#endif
#ifndef USB_vDataOutCallback
#define USB_vDataOutCallback            USBD_EpOutCallback
#endif

/* usbd <- PD */
void            USB_vResetCallback      (USBD_HandleType *dev,
                                         USB_SpeedType speed);

/* usbd_ctrl <- PD */
void            USB_vSetupCallback      (USBD_HandleType *dev);

/* usbd_ep <- PD */
void            USB_vDataInCallback     (USBD_HandleType *dev,
                                         USBD_EpHandleType *ep);
void            USB_vDataOutCallback    (USBD_HandleType *dev,
                                         USBD_EpHandleType *ep);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __XPD_USB_WRAPPER_H_ */
