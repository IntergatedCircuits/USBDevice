/**
  ******************************************************************************
  * @file    usbd_desc.c
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   Universal Serial Bus Device Driver
  *          USB descriptors provision
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
#include <private/usbd_private.h>
#include <usbd_utils.h>

/** @ingroup USBD_Private
 * @defgroup USBD_Private_Functions_Desc USBD Descriptors Provision
 * @{ */

__alignment(USBD_DATA_ALIGNMENT)
/** @brief USB Standard Language Identifier Descriptor */
static const USB_LangIdDescType usbd_langIdDesc __align(USBD_DATA_ALIGNMENT) = {
    .bLength         = sizeof(usbd_langIdDesc),
    .bDescriptorType = USB_DESC_TYPE_STRING,
    .wLANGID         = {USBD_LANGID_STRING,
                        },
};

/** @brief USB Standard Composite Device Descriptor */
static const USB_DeviceDescType usbd_deviceDesc = {
    .bLength            = sizeof(USB_DeviceDescType),
    .bDescriptorType    = USB_DESC_TYPE_DEVICE,
    .bcdUSB             = USBD_SPEC_BCD,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize     = USBD_EP0_MAX_PACKET_SIZE,
    .idVendor           = 0xFFFF,
    .idProduct          = 0xFFFF,
    .bcdDevice          = 0xFFFF,
    .iManufacturer      = USBD_ISTR_VENDOR,
    .iProduct           = USBD_ISTR_PRODUCT,
#if (USBD_SERIAL_BCD_SIZE > 0)
    .iSerialNumber      = USBD_ISTR_SERIAL,
#endif /* Defaults to 0 */
    .bNumConfigurations = USBD_MAX_CONFIGURATION_COUNT
};

#if (USBD_HS_SUPPORT == 1)
/** @brief USB Standard Device Qualifier Descriptor */
static const USB_DeviceQualifierDescType usbd_devQualDesc __align(USBD_DATA_ALIGNMENT) =
{
    .bLength            = sizeof(USB_DeviceQualifierDescType),
    .bDescriptorType    = USB_DESC_TYPE_DEVICE_QUALIFIER,
    .bcdUSB             = USBD_SPEC_BCD,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize     = USBD_EP0_MAX_PACKET_SIZE,
    .bNumConfigurations = USBD_MAX_CONFIGURATION_COUNT,
};
#endif

/** @brief USB Binary device Object Store (BOS) Descriptor */
#if (USBD_LPM_SUPPORT == 1)
static const struct {
    USB_BOSDescType bos;                /*!< BOS base */
    USB_DevCapabilityDescType devCap;   /*!< Device capabilities */
}__packed usbd_bosDesc __align(USBD_DATA_ALIGNMENT) =
{
    .bos = {
        .bLength            = sizeof(USB_BOSDescType),
        .bDescriptorType    = USB_DESC_TYPE_BOS,
        .wTotalLength       = sizeof(usbd_bosDesc),
        .bNumDeviceCaps     = 1,
    },
    .devCap = {
        .bLength            = sizeof(USB_DevCapabilityDescType),
        .bDescriptorType    = USB_DESC_TYPE_DEVICE_CAPABILITY,
        .bDevCapabilityType = USB_DEVCAP_USB_2p0_EXT,
        .bmAttributes       = 0,
    }
};
#endif

/**
 * @brief This function provides the USB device descriptor.
 * @param dev: USB Device handle reference
 * @param data: the target container for the device descriptor
 * @return The length of the descriptor
 */
static uint16_t USBD_DeviceDesc(USBD_HandleType *dev, uint8_t *data)
{
    USB_DeviceDescType *desc = (USB_DeviceDescType*)data;

    memcpy(data, &usbd_deviceDesc, sizeof(USB_DeviceDescType));

    desc->bMaxPacketSize = dev->EP.OUT[0].MaxPacketSize;
    desc->idVendor       = dev->Desc->Vendor.ID;
    desc->idProduct      = dev->Desc->Product.ID;
    desc->bcdDevice      = dev->Desc->Product.Version.bcd;

    return sizeof(USB_DeviceDescType);
}

/**
 * @brief This function assembles the USB configuration descriptor
 *        using the interface descriptors.
 * @param dev: USB Device handle reference
 * @param data: the target container for the configuration descriptor
 * @return The length of the descriptor
 */
static uint16_t USBD_ConfigDesc(USBD_HandleType *dev, uint8_t *data)
{
    USB_ConfigDescType *desc = (USB_ConfigDescType*)data;
    uint16_t wTotalLength = sizeof(USB_ConfigDescType);
    uint8_t ifNum;
    USBD_IfHandleType *itf = NULL;

    /* Get the individual interface descriptors */
    for (ifNum = 0; ifNum < dev->IfCount; ifNum++)
    {
        /* Associated interfaces return the entire descriptor */
        if (dev->IF[ifNum] == itf) { continue; }

        itf = dev->IF[ifNum];
        wTotalLength += USBD_IfClass_GetDesc(itf, ifNum, &data[wTotalLength]);
    }

    /* Get the configuration descriptor */
    desc->bLength               = sizeof(USB_ConfigDescType);
    desc->bDescriptorType       = USB_DESC_TYPE_CONFIGURATION;
    desc->wTotalLength          = wTotalLength;
    desc->bNumInterfaces        = dev->IfCount;
    desc->bConfigurationValue   = 1;
    desc->iConfiguration        = USBD_ISTR_CONFIG;
    desc->bmAttributes          = 0x80 | dev->Desc->Config.b;
    desc->bMaxPower             = dev->Desc->Config.MaxCurrent_mA / 2;

    return wTotalLength;
}

/**
 * @brief This function converts an ASCII string into a string descriptor.
 * @param str: the input ASCII string
 * @param data: the target container for the string descriptor
 * @return The length of the descriptor
 */
static uint16_t USBD_GetStringDesc(const char *str, uint8_t *data)
{
    uint16_t *dst = (uint16_t*)&data[2];
    data[0] = 2;
    data[1] = USB_DESC_TYPE_STRING;

    /* If ASCII, convert to Unicode */
    if (str[1] != 0)
    {
        uint8_t  *src = (uint8_t*)str;
        while (*src != 0)
        {
            *dst++ = (uint16_t)*src++;
            data[0] += sizeof(uint16_t);
        }
    }
    else /* If Unicode already, just copy */
    {
        uint16_t *src = (uint16_t*)str;
        while (*src != 0)
        {
            *dst++ = *src++;
            data[0] += sizeof(uint16_t);
        }
    }
    return data[0];
}

/**
 * @brief This function collects and transfers the requested descriptor through EP0.
 * @param dev: USB Device handle reference
 * @return OK if the descriptor is provided, INVALID if not supported
 */
USBD_ReturnType USBD_GetDescriptor(USBD_HandleType *dev)
{
    USBD_ReturnType retval = USBD_E_INVALID;

    uint16_t len = 0;
    uint8_t *data = dev->CtrlData;

    /* High byte identifies descriptor type */
    switch (dev->Setup.Value >> 8)
    {
        case USB_DESC_TYPE_DEVICE:
        {
            len = USBD_DeviceDesc(dev, data);
            break;
        }

        case USB_DESC_TYPE_CONFIGURATION:
        {
            len = USBD_ConfigDesc(dev, data);
            break;
        }

        case USB_DESC_TYPE_STRING:
        {
            /* Low byte is the descriptor iIndex */
            switch (dev->Setup.Value & 0xFF)
            {
                /* Zero index returns the list of supported Unicode
                 * language identifiers */
                case USBD_ISTR_LANGID:
                    data = (uint8_t*)&usbd_langIdDesc;
                    len  = sizeof(usbd_langIdDesc);
                    break;

                /* Otherwise Setup.Index == LangID of requested string */

                case USBD_ISTR_VENDOR:
                    len = USBD_GetStringDesc(dev->Desc->Vendor.Name, data);
                    break;

                case USBD_ISTR_PRODUCT:
                    len = USBD_GetStringDesc(dev->Desc->Product.Name, data);
                    break;

                case USBD_ISTR_CONFIG:
                    len = USBD_GetStringDesc(dev->Desc->Config.Name, data);
                    break;

#if (USBD_SERIAL_BCD_SIZE > 0)
                case USBD_ISTR_SERIAL:
                    data[0] = len = 2 + USBD_SERIAL_BCD_SIZE * 2;
                    data[1] = USB_DESC_TYPE_STRING;
                    Uint2Unicode((const uint8_t*)dev->Desc->SerialNumber,
                            &data[2], USBD_SERIAL_BCD_SIZE);
                    break;
#endif

                default:
                {
                    const char* str = USBD_IfString(dev);

                    if (str != NULL)
                    {
                        len = USBD_GetStringDesc(str, data);
                    }
                    break;
                }
            }
            break;
        }

#if (USBD_HS_SUPPORT == 1)
        case USB_DESC_TYPE_DEVICE_QUALIFIER:
        {
            if (dev->Speed == USB_SPEED_HIGH)
            {
                data = (uint8_t*)&usbd_devQualDesc;
                len  = sizeof(usbd_devQualDesc);
            }
            break;
        }

        case USB_DESC_TYPE_OTHER_SPEED_CONFIG:
        {
            if (dev->Speed == USB_SPEED_HIGH)
            {
                /* Workaround: temporarily set speed to full,
                 * so the configuration is assembled for full speed case */
                dev->Speed = USB_SPEED_FULL;
                len = USBD_ConfigDesc(dev, data);
                dev->Speed = USB_SPEED_HIGH;
            }
            break;
        }
#endif

#if (USBD_LPM_SUPPORT == 1)
        case USB_DESC_TYPE_BOS:
        {
            len = sizeof(usbd_bosDesc);

            /* Check if Link Power Management is used */
            if (dev->Desc->Config.LPM != 0)
            {
                memcpy(data, &usbd_bosDesc, sizeof(usbd_bosDesc));

                /* Modify bmAttributes:
                 * bit1: LPM protocol support
                 * bit2: BESL and alternate HIRD definitions supported */
                data[8] |= 6;
            }
            else
            {
                /* Return the default BOS */
                data = (uint8_t*)&usbd_bosDesc;
            }
            break;
        }
#endif

        default:
            break;
    }

    /* Transfer the non-null descriptor */
    if (len > 0)
    {
        retval = USBD_CtrlSendData(dev, data, len);
    }

    return retval;
}

/** @} */

/** @addtogroup USBD_Internal_Functions
 * @{ */

/**
 * @brief This function returns the input endpoint's descriptor and length.
 * @param dev: USB Device handle reference
 * @param epAddr: endpoint address
 * @param data: the target container for the endpoint descriptor
 * @return The length of the descriptor
 */
uint16_t USBD_EpDesc(USBD_HandleType *dev, uint8_t epAddr, uint8_t *data)
{
    USBD_EpHandleType *ep = USBD_EpAddr2Ref(dev, epAddr);
    USB_EndpointDescType *desc = (USB_EndpointDescType*)data;

    desc->bLength           = sizeof(USB_EndpointDescType);
    desc->bDescriptorType   = USB_DESC_TYPE_ENDPOINT;
    desc->bEndpointAddress  = epAddr;
    desc->bmAttributes      = ep->Type;
    desc->wMaxPacketSize    = ep->MaxPacketSize;
    desc->bInterval         = 1;

    return sizeof(USB_EndpointDescType);
}

/**
 * @brief Converts milliseconds to HS descriptor bInterval format with approximation.
 * @param interval_ms: the EP polling interval in ms
 * @return The closest bInterval field value
 */
uint8_t USBD_EpHsInterval(uint32_t interval_ms)
{
    uint32_t i, interval_125us = (interval_ms * 1000) / 125;
    for (i = 3; i < 16; i++)
    {
        if (interval_125us < ((uint32_t)2 << i))
        {
            i++;
            break;
        }
    }
    return (uint8_t)i;
}

/** @} */
