/**
  ******************************************************************************
  * @file    usbd_pd_if.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   Documentation-only
  *          Peripheral Driver-specific interface function declarations
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

#include <usbd_types.h>

/** @ingroup USBD
 * @defgroup USBD_PD_Interface USB Peripheral Driver Interface
 * @brief Documentation of the Peripheral Driver accessing interface
 * @{ */

/**
 * @brief Initializes the USB peripheral based on the settings in
 *        USBD_DescriptionType::Config
 * @param dev: USB Device handle reference
 * @param conf: the configuration field of the Device Description
 */
extern void USBD_PD_Init(USBD_HandleType * dev, const USBD_ConfigurationType * conf);

/**
 * @brief Shuts down the USB peripheral completely.
 * @param dev: USB Device handle reference
 */
extern void USBD_PD_Deinit(USBD_HandleType * dev);

/**
 * @brief Starts the operation of the peripheral as a device,
 *        places the peripheral in ATTACHED state.
 * @param dev: USB Device handle reference
 */
extern void USBD_PD_Start(USBD_HandleType * dev);

/**
 * @brief Stops the operation of the peripheral as a device,
 *        places the peripheral in DETACHED state.
 * @param dev: USB Device handle reference
 */
extern void USBD_PD_Stop(USBD_HandleType * dev);

/**
 * @brief Starts remote wakeup signaling on the USB bus.
 * @param dev: USB Device handle reference
 */
extern void USBD_PD_SetRemoteWakeup(USBD_HandleType * dev);

/**
 * @brief Stops SUSPEND state remote wakeup signaling on the USB bus.
 * @param dev: USB Device handle reference
 */
extern void USBD_PD_ClearRemoteWakeup(USBD_HandleType * dev);

/**
 * @brief Sets the USB device's address in the peripheral.
 * @param dev: USB Device handle reference
 * @param addr: the new device address to set
 */
extern void USBD_PD_SetAddress(USBD_HandleType * dev, uint8_t addr);

/**
 * @brief Opens the default control endpoint (EP0) of the device.
 * @param dev: USB Device handle reference
 */
extern void USBD_PD_CtrlEpOpen(USBD_HandleType * dev);

/**
 * @brief Opens a device endpoint.
 * @param dev: USB Device handle reference
 * @param addr: endpoint address
 * @param type: endpoint type
 * @param mps: maximum packet size
 */
extern void USBD_PD_EpOpen      (USBD_HandleType * dev, uint8_t addr,
                                 USB_EndPointType type,uint16_t mps);

/**
 * @brief Closes a device endpoint.
 * @param dev: USB Device handle reference
 * @param addr: endpoint address
 */
extern void USBD_PD_EpClose     (USBD_HandleType * dev, uint8_t addr);

/**
 * @brief Sends a data stream through a device endpoint.
 * @param dev: USB Device handle reference
 * @param addr: endpoint address
 * @param data: pointer to the data to send
 * @param len: total length of the data
 */
extern void USBD_PD_EpSend      (USBD_HandleType * dev, uint8_t addr,
                                 const uint8_t* data, uint16_t len);

/**
 * @brief Receives data through a device endpoint.
 * @param dev: USB Device handle reference
 * @param addr: endpoint address
 * @param data: pointer to the data buffer
 * @param len: maximum length of the data
 */
extern void USBD_PD_EpReceive   (USBD_HandleType * dev, uint8_t addr,
                                 uint8_t* data, uint16_t len);

/**
 * @brief Sets a device endpoint to STALL transfers.
 * @param dev: USB Device handle reference
 * @param addr: endpoint address
 */
extern void USBD_PD_EpSetStall  (USBD_HandleType * dev, uint8_t addr);

/**
 * @brief Clears the STALL condition of a device endpoint.
 * @param dev: USB Device handle reference
 * @param addr: endpoint address
 */
extern void USBD_PD_EpClearStall(USBD_HandleType * dev, uint8_t addr);

/**
 * @brief Empties any buffered data from a device endpoint.
 * @param dev: USB Device handle reference
 * @param addr: endpoint address
 */
extern void USBD_PD_EpFlush     (USBD_HandleType * dev, uint8_t addr);

/** @} */

#endif /* __USBD_PD_IF_H_ */
