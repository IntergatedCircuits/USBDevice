/**
  ******************************************************************************
  * @file    usbd_cdc.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   USB Communications Device Class
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
#ifndef __USBD_CDC_H
#define __USBD_CDC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <usbd_types.h>

/** @ingroup USBD
 * @addtogroup USBD_Class USBD Classes
 * @{ */

/** @defgroup USBD_CDC Communications Device Class (CDC)
 * @{ */

/** @defgroup USBD_CDC_Exported_Types CDC Exported Types
 * @{ */

/** @brief Standard CDC control requests */
typedef enum
{
    CDC_REQ_SEND_ENCAPSULATED_COMMAND   = 0x00, /*!< Host command in the supported control protocol format */
    CDC_REQ_GET_ENCAPSULATED_RESPONSE   = 0x01, /*!< Device response in the supported control protocol format */
    CDC_REQ_SET_COMM_FEATURE            = 0x02, /*!< Change a feature (indexed by wIndex) */
    CDC_REQ_GET_COMM_FEATURE            = 0x03, /*!< Read a feature (indexed by wIndex) */
    CDC_REQ_CLEAR_COMM_FEATURE          = 0x04, /*!< Reset a feature (indexed by wIndex) to default state */
    CDC_REQ_SET_LINE_CODING             = 0x20, /*!< Set asynchronous line-character formatting properties */
    CDC_REQ_GET_LINE_CODING             = 0x21, /*!< Read the current line coding */
    CDC_REQ_SET_CONTROL_LINE_STATE      = 0x22, /*!< Generate RS-232 control signals
                                                     (wValue = 0b[RTS activation][DTR activation]) */
    CDC_REQ_SEND_BREAK                  = 0x23, /*!< Generates an RS-232 style break
                                                     (wValue = break length in ms,
                                                     if = 0xFFFF it is continuous until
                                                     another request with 0 is received) */

    /* CDC class request for NCM */
    CDC_REQ_SET_ENET_MULTICAST_FILTERS  = 0x40, /*!< Controls the receipt of Ethernet frames
                                                     that are received with “multicast” destination addresses */
    CDC_REQ_SET_ENET_PWR_MGMT_PFILTER   = 0x41, /*!<  */
    CDC_REQ_GET_ENET_PWR_MGMT_PFILTER   = 0x42, /*!< Retrieves the status of the above power management pattern filter setting */
    CDC_REQ_SET_ENET_PACKET_FILTER      = 0x43, /*!< Controls the types of Ethernet frames that are to be received via the function */
    CDC_REQ_GET_ENET_STATISTIC          = 0x44, /*!< Retrieves Ethernet statistics such as
                                                     frames transmitted, frames received, and bad frames received */
    CDC_REQ_GET_NTB_PARAMETERS          = 0x80, /*!< Requests the function to report parameters that characterize the NCB */
    CDC_REQ_GET_NET_ADDRESS             = 0x81, /*!< Requests the current EUI-48 network address */
    CDC_REQ_SET_NET_ADDRESS             = 0x82, /*!< Changes the current EUI-48 network address */
    CDC_REQ_GET_NTB_FORMAT              = 0x83, /*!< Get current NTB Format */
    CDC_REQ_SET_NTB_FORMAT              = 0x84, /*!< Select 16 or 32 bit Network Transfer Blocks */
    CDC_REQ_GET_NTB_INPUT_SIZE          = 0x85, /*!< Get the current value of maximum NTB input size */
    CDC_REQ_SET_NTB_INPUT_SIZE          = 0x86, /*!< Selects the maximum size of NTBs to be transmitted
                                                     by the function over the bulk IN pipe */
    CDC_REQ_GET_MAX_DATAGRAM_SIZE       = 0x87, /*!< Requests the current maximum datagram size */
    CDC_REQ_SET_MAX_DATAGRAM_SIZE       = 0x88, /*!< Sets the maximum datagram size to a value other than the default */
    CDC_REQ_GET_CRC_MODE                = 0x89, /*!< Requests the current CRC mode */
    CDC_REQ_SET_CRC_MODE                = 0x90, /*!< Sets the current CRC mode */
}USBD_CDC_RequestType;


/** @brief CDC line coding configuration structure */
typedef PACKED(struct)
{
    uint32_t DTERate;       /*!< Data terminal rate, in bits per second */
    uint8_t  CharFormat;    /*!< Stop bits:
                                    @arg 0 -> 1 Stop bit
                                    @arg 1 -> 1.5 Stop bits
                                    @arg 2 -> 2 Stop bits */
    uint8_t  ParityType;    /*!< Parity:
                                    @arg 0 -> None
                                    @arg 1 -> Odd
                                    @arg 2 -> Even
                                    @arg 3 -> Mark
                                    @arg 4 -> Space */
    uint8_t  DataBits;      /*!< Data bits: (5, 6, 7, 8 or 16) */
}USBD_CDC_LineCodingType;


/** @brief Standard CDC notifications */
typedef enum
{
    CDC_NOT_NETWORK_CONNECTION      = 0x00, /*!< Notify the host about network connection status (Value = 1 / 0) */
    CDC_NOT_RESPONSE_AVAILABLE      = 0x01, /*!< Notify the host that an encapsulated response is available */
    CDC_NOT_SERIAL_STATE            = 0x29, /*!< Asynchronous notification of UART status */
    CDC_NOT_CONNECTION_SPEED_CHANGE = 0x2A, /*!< Change in the uplink / downlink bit rate of the connection */
}USBD_CDC_NotificationType;


/** @brief CDC notification header structure */
typedef PACKED(struct)
{
    uint8_t  RequestType;       /*!< Always set to 0xA1 */
    uint8_t  NotificationType;  /*!< @ref USBD_CDC_NotificationType */
    uint16_t Value;             /*!< The data if it fits */
    uint16_t Index;             /*!< Communication interface index in the USB configuration */
    uint16_t Length;            /*!< Length of optional additional data */
}USBD_CDC_NotifyHeaderType;


/** @brief CDC speed change notification data structure */
typedef PACKED(struct)
{
    uint32_t DLBitRate; /* contains the downlink bit rate (IN pipe) */
    uint32_t ULBitRate; /* contains the uplink bit rate (OUT pipe) */
}USBD_CDC_SpeedChangeType;


/** @brief CDC serial state notification data structure */
typedef PACKED(union)
{
    struct {
        uint16_t RxCarrier : 1; /*!< State of receiver carrier detection mechanism (RS-232 signal DCD) */
        uint16_t TxCarrier : 1; /*!< State of transmission carrier (RS-232 signal DSR.) */
        uint16_t Break : 1;     /*!< State of break detection mechanism */
        uint16_t RingSignal : 1;/*!< State of ring signal detection */
        uint16_t Framing : 1;   /*!< A framing error has occurred */
        uint16_t Parity : 1;    /*!< A parity error has occurred */
        uint16_t OverRun : 1;   /*!< Received data has been discarded due to overrun */
        uint16_t : 9;
    };
    uint16_t w;
}USBD_CDC_SerialStateType;


/** @brief Common CDC notification data structure */
typedef struct
{
    USBD_CDC_NotifyHeaderType Header;
    union {
                                                /*!< CDC_NOT_NETWORK_CONNECTION */
                                                /*!< CDC_NOT_RESPONSE_AVAILABLE */
        USBD_CDC_SerialStateType SerialState;   /*!< CDC_NOT_SERIAL_STATE */
        USBD_CDC_SpeedChangeType SpeedChange;   /*!< CDC_NOT_CONNECTION_SPEED_CHANGE */
    };
}USBD_CDC_NotifyMessageType;


/** @brief CDC application structure */
typedef struct
{
    const char* Name;   /*!< String description of the application */

    void (*Open)        (void* itf,
                         USBD_CDC_LineCodingType* coding);  /*!< Open port */

    void (*Close)       (void* itf);        /*!< Close port */

    void (*Received)    (void* itf,
                         uint8_t* data,
                         uint16_t length);  /*!< Received data available */

    void (*Transmitted) (void* itf,
                         uint8_t* data,
                         uint16_t length);  /*!< Transmission of data completed */

#if (USBD_CDC_CONTROL_LINE_USED == 1)
    void (*SetCtrlLine) (void* itf,
                         uint8_t dtr,
                         uint8_t rts);      /*!< Control DTR and RTS signals */
#endif /* USBD_CDC_CONTROL_LINE_USED */

#if (USBD_CDC_BREAK_SUPPORT == 1)
    void (*Break)       (void* itf,
                         uint16_t len_ms);  /*!< Send BREAK signal of given time on Tx line */
#endif /* USBD_CDC_BREAK_SUPPORT */
}USBD_CDC_AppType;


/** @brief CDC interface configuration */
typedef struct
{
    uint8_t Protocol;   /*!< Protocol used for Control requests */
    uint8_t OutEpNum;   /*!< OUT endpoint address */
    uint8_t InEpNum;    /*!< IN endpoint address */
    uint8_t NotEpNum;   /*!< Notification endpoint address
                             @note This address should be distinct and valid even if
                             the endpoint is not used. */
}USBD_CDC_ConfigType;


/** @brief CDC class interface structure */
typedef struct
{
    USBD_IfHandleType Base;             /*!< Class-independent interface base */
    const USBD_CDC_AppType* App;        /*!< CDC application reference */
    USBD_CDC_ConfigType Config;         /*!< CDC interface configuration */

    USBD_CDC_LineCodingType LineCoding; /*!< CDC line coding */
    USBD_PADDING_1();
    uint16_t TransmitLength;            /*!< Backup transmitted length for splitting transfers */
}USBD_CDC_IfHandleType;

/** @} */

/** @addtogroup USBD_CDC_Exported_Functions
 * @{ */
USBD_ReturnType USBD_CDC_MountInterface (USBD_CDC_IfHandleType *itf,
                                         USBD_HandleType *dev);

USBD_ReturnType USBD_CDC_Transmit       (USBD_CDC_IfHandleType *itf,
                                         uint8_t *data,
                                         uint16_t length);

USBD_ReturnType USBD_CDC_Receive        (USBD_CDC_IfHandleType *itf,
                                         uint8_t *data,
                                         uint16_t length);

#if (USBD_CDC_NOTEP_USED == 1)
USBD_ReturnType USBD_CDC_Notify         (USBD_CDC_IfHandleType *itf,
                                         USBD_CDC_NotifyMessageType *notice);
#endif

/** @} */

/** @} */

/** @} */

#ifdef __cplusplus
}
#endif

#endif  /* __USBD_CDC_H */
