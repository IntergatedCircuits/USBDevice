/**
  ******************************************************************************
  * @file    usbd_internal.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   Universal Serial Bus Device Driver
  *          Services accessible by (class) interfaces
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
#ifndef __USBD_INTERNAL_H_
#define __USBD_INTERNAL_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <usbd.h>
#include <usbd_pd_if.h>

/* strlen(), memcpy() */
#include <string.h>

/** @ingroup USBD
 * @addtogroup USBD_Internal_Functions
 * @{ */

USBD_ReturnType USBD_CtrlSendData       (USBD_HandleType *dev,
                                         void *data,
                                         uint16_t len);

USBD_ReturnType USBD_CtrlReceiveData    (USBD_HandleType *dev,
                                         void *data, uint16_t len);

uint16_t        USBD_EpDesc             (USBD_HandleType *dev,
                                         uint8_t epAddr,
                                         uint8_t *data);

uint8_t         USBD_EpHsInterval       (uint32_t interval_ms);

USBD_ReturnType USBD_EpSend             (USBD_HandleType *dev,
                                         uint8_t epAddr,
                                         void *data,
                                         uint16_t len);

USBD_ReturnType USBD_EpReceive          (USBD_HandleType *dev,
                                         uint8_t epAddr,
                                         void *data,
                                         uint16_t len);

/**
 * @brief Converts the USBD endpoint address to its reference.
 * @param dev: USB Device handle reference
 * @param epAddr: endpoint address
 * @return The endpoint's reference
 */
static inline
USBD_EpHandleType* USBD_EpAddr2Ref      (USBD_HandleType *dev,
                                         uint8_t epAddr)
{
    return (epAddr > 0x7F) ? &dev->EP.IN[epAddr & 0xF] : &dev->EP.OUT[epAddr];
}

/**
 * @brief Converts the USBD endpoint reference to its address.
 * @param dev: USB Device handle reference
 * @param ep: USB endpoint handle reference
 * @return The endpoint's address
 */
static inline uint8_t USBD_EpRef2Addr   (USBD_HandleType *dev,
                                         USBD_EpHandleType *ep)
{
    uint8_t epAddr = (((uint32_t)ep - (uint32_t)&dev->EP.IN[0])
            / sizeof(USBD_EpHandleType));
    return (epAddr < USBD_MAX_EP_COUNT) ?
            (0x80 | epAddr) :                   /* IN endpoint */
            (epAddr - USBD_MAX_EP_COUNT); /* OUT endpoint */
}

/**
 * @brief Opens the device endpoint.
 * @param dev: USB Device handle reference
 * @param epAddr: endpoint address
 * @param type: endpoint type
 * @param mps: endpoint maximal packet size
 */
static inline void USBD_EpOpen          (USBD_HandleType *dev,
                                         uint8_t epAddr,
                                         USB_EndPointType type,
                                         uint16_t mps)
{
    USBD_PD_EpOpen(dev, epAddr, type, mps);
    USBD_EpAddr2Ref(dev, epAddr)->State = USB_EP_STATE_IDLE;
}

/**
 * @brief Closes the device endpoint.
 * @param dev: USB Device handle reference
 * @param epAddr: endpoint address
 */
static inline void USBD_EpClose         (USBD_HandleType *dev,
                                         uint8_t epAddr)
{
    USBD_PD_EpClose(dev, epAddr);
    USBD_EpAddr2Ref(dev, epAddr)->State = USB_EP_STATE_CLOSED;
}

/**
 * @brief Flushes the buffered data from the endpoint.
 * @param dev: USB Device handle reference
 * @param epAddr: endpoint address
 */
static inline void USBD_EpFlush         (USBD_HandleType *dev,
                                         uint8_t epAddr)
{
    USBD_PD_EpFlush(dev, epAddr);
    USBD_EpAddr2Ref(dev, epAddr)->State = USB_EP_STATE_IDLE;
}

/**
 * @brief Sets the stall (NAK) status on the endpoint.
 * @param dev: USB Device handle reference
 * @param epAddr: endpoint address
 */
static inline void USBD_EpSetStall      (USBD_HandleType *dev,
                                         uint8_t epAddr)
{
    USBD_PD_EpSetStall(dev, epAddr);
    USBD_EpAddr2Ref(dev, epAddr)->State = USB_EP_STATE_STALL;
}

/**
 * @brief Clears the stall (NAK) status on the endpoint.
 * @param dev: USB Device handle reference
 * @param epAddr: endpoint address
 */
static inline void USBD_EpClearStall    (USBD_HandleType *dev,
                                         uint8_t epAddr)
{
    USBD_PD_EpClearStall(dev, epAddr);
    USBD_EpAddr2Ref(dev, epAddr)->State = USB_EP_STATE_IDLE;
}

/** @} */

/**
 * @brief  Safe function pointer caller that
 *         checks it against NULL and calls it with parameters.
 * @param  CALLBACK: the function pointer
 * @param  ...: the required parameters of the function
 */
#define USBD_SAFE_CALLBACK(CALLBACK, ...)        \
    do{ if ((CALLBACK) != NULL) (void) CALLBACK(__VA_ARGS__); }while(0)


/** @brief Reserved string descriptor index */
#define USBD_IIF_INVALID                0

/**
 * @brief  Returns an interface string index based on the interface index
 *         and an internal index.
 * @param  IFNUM: the interface index in the device
 * @param  INTNUM: the internal index
 */
#define USBD_IIF_INDEX(IFNUM, INTNUM)   \
    (USBD_ISTR_INTERFACES + (IFNUM) + ((INTNUM) << 4))


#ifdef __htonl
#define htonl(_x)                       __htonl(_x)
#endif
#ifdef __htons
#define htons(_x)                       __htons(_x)
#endif
#define ntohl(_x)                       htonl(_x)
#define ntohs(_x)                       htons(_x)


#ifdef __cplusplus
}
#endif

#endif /* __USBD_INTERNAL_H_ */
