/**
  ******************************************************************************
  * @file    usbd_utils.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   Universal Serial Bus Device Driver
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
#ifndef __USBD_UTILS_H_
#define __USBD_UTILS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

extern void Uint2Unicode(const uint8_t *data, uint8_t *unicode, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* __USBD_UTILS_H_ */
