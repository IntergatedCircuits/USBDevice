/**
  ******************************************************************************
  * @file    usbd_config.h
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
#ifndef __USBD_CONFIG_H_
#define __USBD_CONFIG_H_

#ifdef __cplusplus
extern "C"
{
#endif

/** @addtogroup USBD_Exported_Macros
 * @{ */

/** @brief Must be set according to the highest number of interfaces for a given USB Device. */
#define USBD_MAX_IF_COUNT           2

/** @brief Must be set higher than:
 * @arg the length of the entire USB device configuration descriptor
 * @arg the longest USB string (as Unicode string descriptor)
 * @arg the length of the longest class-specific control request */
#define USBD_EP0_BUFFER_SIZE        256

/** @brief Set to 1 if peripheral support and application demand
 * for High-Speed operation both exist. */
#define USBD_HS_SUPPORT             0

/** @brief When set to 0, no SerialNumber is readable by the host.
 * Otherwise the SerialNumber will be converted from USBD_SERIAL_BCD_SIZE
 * amount of raw bytes to string BCD format and sent to the host. */
#define USBD_SERIAL_BCD_SIZE        8

/* Any class-specific configuration may follow, e.g.
 *      USBD_HID_OUT_SUPPORT        1 */

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __USBD_CONFIG_H_ */
