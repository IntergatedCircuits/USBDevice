/**
  ******************************************************************************
  * @file    usbd_private.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   Universal Serial Bus Device Driver
  *          Private cross-domain functions header
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
#ifndef __USBD_PRIVATE_H_
#define __USBD_PRIVATE_H_

#include <usbd_internal.h>

/* {function definition} <- {call site} */

/* usbd <- usbd_ctrl */
USBD_ReturnType USBD_DevRequest         (USBD_HandleType *dev);

/* usbd_ctrl <- usbd_ep */
void            USBD_CtrlInCallback     (USBD_HandleType *dev);
void            USBD_CtrlOutCallback    (USBD_HandleType *dev);

/* usbd_if <- usbd_ctrl */
USBD_ReturnType USBD_IfRequest          (USBD_HandleType *dev);

/* usbd_if <- usbd */
void            USBD_IfConfig           (USBD_HandleType *dev,
                                         uint8_t cfgNum);

/* usbd_if <- usbd_desc */
const char*     USBD_IfString           (USBD_HandleType *dev);

/* usbd_ep <- usbd_ctrl */
USBD_ReturnType USBD_EpRequest          (USBD_HandleType *dev);

/* usbd_desc <- usbd */
USBD_ReturnType USBD_GetDescriptor      (USBD_HandleType *dev);

#endif /* __USBD_PRIVATE_H_ */
