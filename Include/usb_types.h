/**
  ******************************************************************************
  * @file    usb_types.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   Universal Serial Bus Device Driver
  *          Standard type definitions
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
#ifndef __USB_TYPES_H_
#define __USB_TYPES_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

/** @defgroup USB Universal Serial Bus
 *  @brief USB 2.0 specification compliant software
 * @{ */

/** @defgroup USB_Exported_Macros USB Exported Macros
 * @{ */

#if  defined ( __GNUC__ )
#ifndef __packed
#define __packed __attribute__((packed))
#endif /* __packed */
#ifndef __align
#define __align(X) __attribute__((aligned(X)))
#endif /* __align */
#elif defined ( __ICCARM__ )
#define __stringize(X) #X
#define __align(X)
#define __alignment(X) _Pragma(__stringize(data_alignment=X))
#endif
#ifndef __alignment
#define __alignment(X)
#endif

#ifndef PACKED
#if defined (__ICCARM__)
#define PACKED(X) __packed X
#else
#define PACKED(X) X __packed
#endif
#endif

/* The current implementation is based on USB release 2.00 */
#define USB_SPEC_BCD                    0x0200

/* USB standard features */
#define USB_FEATURE_EP_HALT             0
#define USB_FEATURE_REMOTE_WAKEUP       1
#define USB_FEATURE_TEST_MODE           2

/* USB endpoint MaxPacketSize depending on speed and type */
#define USB_EP_BULK_HS_MPS              512
#define USB_EP_BULK_FS_MPS              64

#define USB_EP_ISOC_HS_MPS              1024
#define USB_EP_ISOC_FS_MPS              1023

#define USB_EP_INTR_HS_MPS              1024
#define USB_EP_INTR_FS_MPS              64
#define USB_EP_INTR_LS_MPS              8

#define USB_EP_CTRL_HS_MPS              64
#define USB_EP_CTRL_FS_MPS              64
#define USB_EP_CTRL_LS_MPS              8

#define USB_EP0_HS_MAX_PACKET_SIZE      USB_EP_CTRL_HS_MPS
#define USB_EP0_FS_MAX_PACKET_SIZE      USB_EP_CTRL_FS_MPS
#define USB_EP0_LS_MAX_PACKET_SIZE      USB_EP_CTRL_LS_MPS

/** @} */

/** @defgroup USB_Exported_Types USB Exported Types
 * @{ */

/** @brief USB communication directions (considered from host point-of-view) */
typedef enum
{
    USB_DIRECTION_OUT   = 0, /*!< Host to device */
    USB_DIRECTION_IN    = 1, /*!< Device to host */
}USB_DirectionType;

/** @brief Standard USB speed levels */
typedef enum
{
    USB_SPEED_LOW   = 2, /*!< Low speed:  1.5 MBaud */
    USB_SPEED_FULL  = 0, /*!< Full speed:  12 MBaud */
    USB_SPEED_HIGH  = 1, /*!< High speed: 480 MBaud */
}USB_SpeedType;

/** @brief USB endpoint types */
typedef enum
{                                /*   Maximum packet size for       LS      FS      HS */
    USB_EP_TYPE_CONTROL     = 0, /*!< Control endpoint type    *//*  8      64      64 */
    USB_EP_TYPE_ISOCHRONOUS = 1, /*!< Isochronous endpoint type*//* NA    1023    1024 */
    USB_EP_TYPE_BULK        = 2, /*!< Bulk endpoint type       *//* NA      64     512 */
    USB_EP_TYPE_INTERRUPT   = 3  /*!< Interrupt endpoint type  *//*  8      64    1024 */
}USB_EndPointType;

/** @brief USB endpoint address definition */
typedef union
{
    struct {
        uint8_t           Number    : 4; /*!< The number (index) of the endpoint */
        uint8_t           : 3;
        USB_DirectionType Direction : 1; /*!< The endpoint's direction */
    };
    uint8_t b;                           /*!< The endpoint's address on a single byte */
}USB_EndpointAddressType;

/** @brief USB endpoint states */
typedef enum
{
    USB_EP_STATE_CLOSED = 0, /*!< The endpoint is closed -> NAK */
    USB_EP_STATE_IDLE   = 1, /*!< The endpoint is idle */
    USB_EP_STATE_STALL  = 2, /*!< The endpoint is halted -> STALL */
    USB_EP_STATE_SETUP  = 3, /*!< The endpoint is idle */
    USB_EP_STATE_DATA   = 4, /*!< The endpoint is performing data transfer */
    USB_EP_STATE_STATUS = 5, /*!< The endpoint is performing ZLP transfer */
}USB_EndPointStateType;

/** @brief Device Link Power Management (LPM) state */
typedef enum
{
    USB_LINK_STATE_OFF      = 3, /*!< Device disconnected from bus */
    USB_LINK_STATE_SUSPEND  = 2, /*!< Device suspended */
    USB_LINK_STATE_SLEEP    = 1, /*!< Device in L1 sleep mode */
    USB_LINK_STATE_ACTIVE   = 0, /*!< Device connected and active */
}USB_LinkStateType;

/** @brief USB power source selection */
typedef enum
{
    USBD_POWERED_BY_BUS  = 0, /*!< Device is USB bus powered */
    USBD_POWERED_BY_SELF = 1, /*!< Device is independently powered */
}USBD_PowerSourceType;

/** @brief USB setup request types */
typedef enum
{
    USB_REQ_TYPE_STANDARD   = 0, /*!< The request is specified by USB 2.0 standard */
    USB_REQ_TYPE_CLASS      = 1, /*!< The request is specified by a standard USB class */
    USB_REQ_TYPE_VENDOR     = 2, /*!< The request is specified by product vendor */
}USB_RequestType;

/** @brief USB setup request recipients */
typedef enum
{
    USB_REQ_RECIPIENT_DEVICE    = 0, /*!< The request is addressed to the device */
    USB_REQ_RECIPIENT_INTERFACE = 1, /*!< The request is addressed to an interface */
    USB_REQ_RECIPIENT_ENDPOINT  = 2, /*!< The request is addressed to an endpoint */
    USB_REQ_RECIPIENT_OTHER     = 3, /*!< The request is custom addressed */
}USB_ReqRecipientType;

/** @brief USB standard request types */
typedef enum
{
    USB_REQ_GET_STATUS          = 0x00, /*!< Get current status of features */
    USB_REQ_CLEAR_FEATURE       = 0x01, /*!< Clear the activation of a feature */
    USB_REQ_SET_FEATURE         = 0x03, /*!< Activation of a feature */
    USB_REQ_SET_ADDRESS         = 0x05, /*!< Set the bus address of the device */
    USB_REQ_GET_DESCRIPTOR      = 0x06, /*!< Get a descriptor from the device */
    USB_REQ_SET_DESCRIPTOR      = 0x07, /*!< Write a descriptor in the device */
    USB_REQ_GET_CONFIGURATION   = 0x08, /*!< Get the current device configuration index */
    USB_REQ_SET_CONFIGURATION   = 0x09, /*!< Set the new device configuration index */
    USB_REQ_GET_INTERFACE       = 0x0A, /*!< Get the current alternate setting of the interface */
    USB_REQ_SET_INTERFACE       = 0x0B, /*!< Set the new alternate setting of the interface */
    USB_REQ_SYNCH_FRAME         = 0x0C,
}USB_StdRequestType;

/** @brief USB standard descriptor types */
typedef enum
{
    USB_DESC_TYPE_DEVICE                = 0x01, /*!< Device descriptor */
    USB_DESC_TYPE_CONFIGURATION         = 0x02, /*!< Configuration descriptor */
    USB_DESC_TYPE_STRING                = 0x03, /*!< String descriptor */
    USB_DESC_TYPE_INTERFACE             = 0x04, /*!< Interface descriptor */
    USB_DESC_TYPE_ENDPOINT              = 0x05, /*!< Endpoint descriptor */
    USB_DESC_TYPE_DEVICE_QUALIFIER      = 0x06, /*!< Device qualifier descriptor */
    USB_DESC_TYPE_OTHER_SPEED_CONFIG    = 0x07, /*!< Device descriptor for other supported speed */
    USB_DESC_TYPE_IAD                   = 0x0B, /*!< Interface Association Descriptor */
    USB_DESC_TYPE_BOS                   = 0x0F, /*!< Binary device Object Store descriptor */
    USB_DESC_TYPE_DEVICE_CAPABILITY     = 0x10, /*!< Device capability descriptor (part of BOS) */
}USB_DescriptorType;

/** @brief USB device capability types */
typedef enum
{
    USB_DEVCAP_USB_2p0_EXT  = 0x02, /*!< USB 2.0 extension (for LPM) */
    USB_DEVCAP_PLATFORM     = 0x05, /*!< Microsoft OS 2.0 descriptors */
}USB_DeviceCapabilityType;

/** @brief USB setup request */
typedef PACKED(struct)
{
    union {
        struct {
            USB_ReqRecipientType Recipient : 5; /*!< The recipient of the request */
            USB_RequestType      Type      : 2; /*!< The request's type */
            USB_DirectionType    Direction : 1; /*!< The direction of the data stage (OUT when no data stage is present) */
        };
        uint8_t b;      /*!< The bRequestType as a byte */
    }RequestType;       /*!< Specifies the characteristics of the specific request */
    uint8_t   Request;  /*!< Identifies the particular request */
    uint16_t  Value;    /*!< The contents vary according to the request */
    uint16_t  Index;    /*!< The contents vary according to the request */
    uint16_t  Length;   /*!< Specifies the length of the data transferred during the data stage */
}USB_SetupRequestType;

/* All elements shall be stored MSB first in the descriptors */

/** @brief USB device descriptor structure */
typedef PACKED(struct)
{
    uint8_t  bLength;               /*!< Size of the Descriptor in Bytes (18 bytes) */
    uint8_t  bDescriptorType;       /*!< Device Descriptor (0x01) */
    uint16_t bcdUSB;                /*!< USB Specification Number which device complies to */
    uint8_t  bDeviceClass;          /*!< Class Code (Assigned by USB Org)
                                        If equal to Zero, each interface specifies it’s own class code
                                        If equal to 0xFF, the class code is vendor specified.
                                        Otherwise field is valid Class Code. */
    uint8_t  bDeviceSubClass;       /*!< Subclass Code (Assigned by USB Org) */
    uint8_t  bDeviceProtocol;       /*!< Protocol Code (Assigned by USB Org) */
    uint8_t  bMaxPacketSize;        /*!< Maximum Packet Size for Zero Endpoint. Valid Sizes are 8, 16, 32, 64 */
    uint16_t idVendor;              /*!< Vendor ID (Assigned by USB Org) */
    uint16_t idProduct;             /*!< Product ID (Assigned by Manufacturer) */
    uint16_t bcdDevice;             /*!< Device Release Number */
    uint8_t  iManufacturer;         /*!< Index of Manufacturer String Descriptor */
    uint8_t  iProduct;              /*!< Index of Product String Descriptor */
    uint8_t  iSerialNumber;         /*!< Index of Serial Number String Descriptor */
    uint8_t  bNumConfigurations;    /*!< Number of Possible Configurations */
}USB_DeviceDescType;

/** @brief USB configuration descriptor structure */
typedef PACKED(struct)
{
    uint8_t  bLength;               /*!< Size of Descriptor in Bytes */
    uint8_t  bDescriptorType;       /*!< Configuration Descriptor (0x02) */
    uint16_t wTotalLength;          /*!< Total length in bytes of data returned */
    uint8_t  bNumInterfaces;        /*!< Number of Interfaces */
    uint8_t  bConfigurationValue;   /*!< Value to use as an argument to select this configuration */
    uint8_t  iConfiguration;        /*!< Index of String Descriptor describing this configuration */
    uint8_t  bmAttributes;          /*!< 0b1[Self Powered][Remote Wakeup]00000 */
    uint8_t  bMaxPower;             /*!< Maximum Power Consumption in 2mA units */
}USB_ConfigDescType;

/** @brief USB Unicode language identifier structure (with a single supported element) */
typedef PACKED(struct)
{
    uint8_t  bLength;           /*!< Size of Descriptor in Bytes */
    uint8_t  bDescriptorType;   /*!< String Descriptor (0x03) */
    uint16_t wLANGID[1];        /*!< Supported Language Codes (e.g. 0x0409 English - United States) */
}USB_LangIdDescType;

/** @brief USB interface descriptor structure */
typedef PACKED(struct)
{
    uint8_t  bLength;               /*!< Size of Descriptor in Bytes (9 Bytes) */
    uint8_t  bDescriptorType;       /*!< Interface Descriptor (0x04) */
    uint8_t  bInterfaceNumber;      /*!< Number of Interface */
    uint8_t  bAlternateSetting;     /*!< Value used to select alternative setting */
    uint8_t  bNumEndpoints;         /*!< Number of Endpoints used for this interface */
    uint8_t  bInterfaceClass;       /*!< Class Code (Assigned by USB Org) */
    uint8_t  bInterfaceSubClass;    /*!< Subclass Code (Assigned by USB Org) */
    uint8_t  bInterfaceProtocol;    /*!< Protocol Code (Assigned by USB Org) */
    uint8_t  iInterface;            /*!< Index of String Descriptor Describing this interface */
}USB_InterfaceDescType;

/** @brief USB endpoint descriptor structure */
typedef PACKED(struct)
{
    uint8_t  bLength;           /*!< Size of Descriptor in Bytes (7 Bytes) */
    uint8_t  bDescriptorType;   /*!< Interface Descriptor (0x05) */
    uint8_t  bEndpointAddress;  /*!< Endpoint Address 0b[0=Out / 1=In]000[Endpoint Number] */
    uint8_t  bmAttributes;      /*!< Bits 0..1 Transfer Type
                                        00 = Control
                                        01 = Isochronous
                                        10 = Bulk
                                        11 = Interrupt
                                    Bits 2..7 are reserved. If Isochronous endpoint,
                                    Bits 3..2 = Synchronisation Type (Iso Mode)
                                        00 = No Synchonisation
                                        01 = Asynchronous
                                        10 = Adaptive
                                        11 = Synchronous
                                    Bits 5..4 = Usage Type (Iso Mode)
                                        00 = Data Endpoint
                                        01 = Feedback Endpoint
                                        10 = Explicit Feedback Data Endpoint
                                        11 = Reserved */
    uint16_t wMaxPacketSize;    /*!< Maximum Packet Size this endpoint is capable of sending or receiving */
    uint8_t  bInterval;         /*!< Interval for polling endpoint data transfers. Value in frame counts.
                                     Ignored for Bulk & Control Endpoints. Isochronous must equal 1 and
                                     field may range from 1 to 255 for interrupt endpoints. */
}USB_EndpointDescType;

/** @brief USB device qualifier descriptor structure */
typedef PACKED(struct)
{
    uint8_t  bLength;               /*!< Size of Descriptor in Bytes */
    uint8_t  bDescriptorType;       /*!< Device Qualifier Descriptor (0x06) */
    uint16_t bcdUSB;                /*!< USB Specification Number which device complies to */
    uint8_t  bDeviceClass;          /*!< Class Code (Assigned by USB Org)
                                        If equal to Zero, each interface specifies it’s own class code
                                        If equal to 0xFF, the class code is vendor specified.
                                        Otherwise field is valid Class Code. */
    uint8_t  bDeviceSubClass;       /*!< Subclass Code (Assigned by USB Org) */
    uint8_t  bDeviceProtocol;       /*!< Protocol Code (Assigned by USB Org) */
    uint8_t  bMaxPacketSize;        /*!< Maximum Packet Size for Zero Endpoint. Valid Sizes are 8, 16, 32, 64 */
    uint8_t  bNumConfigurations;    /*!< Number of Possible Configurations */
    uint8_t  bReserved;             /*!< Keep 0 */
}USB_DeviceQualifierDescType;

/** @brief Binary device Object Store descriptor (header only) */
typedef PACKED(struct)
{
    uint8_t  bLength;               /*!< Size of Descriptor in Bytes */
    uint8_t  bDescriptorType;       /*!< BOS Descriptor (0x0F) */
    uint16_t wTotalLength;          /*!< Total length in bytes of data returned */
    uint8_t  bNumDeviceCaps;        /*!< Number of device capabilities to follow */
}USB_BOSDescType;

/** @brief USB device capability descriptor structure */
typedef PACKED(struct)
{
    uint8_t  bLength;               /*!< Size of Descriptor in Bytes */
    uint8_t  bDescriptorType;       /*!< Device Capability Descriptor (0x10) */
    uint8_t  bDevCapabilityType;    /*!< Capability type: USB 2.0 EXTENSION (0x02) */
    uint32_t bmAttributes;          /*!< Bit 0 Reserved (set to 0)
                                         Bit 1 Link Power Management support
                                         Bit 2 BESL and alternate HIRD definitions support */
}USB_DevCapabilityDescType;

/** @brief USB Interface Association Descriptor structure */
typedef PACKED(struct)
{
    uint8_t  bLength;               /*!< Size of Descriptor in Bytes */
    uint8_t  bDescriptorType;       /*!< Interface Association Descriptor (0x0B) */
    uint8_t  bFirstInterface;       /*!< First associated interface */
    uint8_t  bInterfaceCount;       /*!< Number of contiguous associated interfaces */
    uint8_t  bFunctionClass;        /*!< Class Code (Assigned by USB Org) */
    uint8_t  bFunctionSubClass;     /*!< Subclass Code (Assigned by USB Org) */
    uint8_t  bFunctionProtocol;     /*!< Protocol Code (Assigned by USB Org) */
    uint8_t  iFunction;             /*!< Index of String Descriptor Describing this function */
}USB_IfAssocDescType;

/** @} */

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __USB_TYPES_H_ */
