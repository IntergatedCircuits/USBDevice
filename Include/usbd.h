/**
  ******************************************************************************
  * @file    usbd.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   Universal Serial Bus Device Driver
  *          Public functions header
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
#ifndef __USBD_H_
#define __USBD_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <usbd_types.h>

/** @ingroup USBD
 * @addtogroup USBD_Exported_Functions
 * @{ */
void            USBD_Init               (USBD_HandleType *dev,
                                         const USBD_DescriptionType *desc);
void            USBD_Deinit             (USBD_HandleType *dev);
void            USBD_UnmountInterfaces  (USBD_HandleType *dev);

void            USBD_Connect            (USBD_HandleType *dev);
void            USBD_Disconnect         (USBD_HandleType *dev);

USBD_ReturnType USBD_SetRemoteWakeup    (USBD_HandleType *dev);
USBD_ReturnType USBD_ClearRemoteWakeup  (USBD_HandleType *dev);
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __USBD_H_ */
