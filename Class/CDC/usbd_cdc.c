/**
  ******************************************************************************
  * @file    usbd_cdc.c
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   USB CDC Abstract Control Model implementation
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
#include <private/usbd_internal.h>
#include <usbd_cdc.h>

#if (USBD_MAX_IF_COUNT < 2)
#error "A single CDC interface takes up 2 device interface slots!"
#endif

#if (USBD_CDC_NOTEP_USED == 1)
#define CDC_NOT_INTR_INTERVAL                       20
#else
#define CDC_NOT_INTR_INTERVAL                       0xFF
#endif
#define CDC_NOT_PACKET_SIZE                         8
#if (USBD_HS_SUPPORT == 1)
#define CDC_DATA_PACKET_SIZE                        USB_EP_BULK_HS_MPS
#else
#define CDC_DATA_PACKET_SIZE                        USB_EP_BULK_FS_MPS
#endif

#define CDC_APP(ITF)    ((USBD_CDC_AppType*)((ITF)->App))

typedef PACKED(struct)
{
    /* Interface Association Descriptor */
    USB_IfAssocDescType IAD;
    /* Communication Interface Descriptor */
    USB_InterfaceDescType CID;
    /* Header Functional Descriptor */
    PACKED(struct) {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint8_t bDescriptorSubtype;
        uint16_t bcdCDC;
    }HFD;
    /* Call Management Functional Descriptor */
    PACKED(struct) {
        uint8_t  bFunctionLength;
        uint8_t  bDescriptorType;
        uint8_t  bDescriptorSubtype;
        uint8_t  bmCapabilities;
        uint8_t  bDataInterface;
    }CMFD;
    /* ACM Functional Descriptor */
    PACKED(struct) {
        uint8_t  bFunctionLength;
        uint8_t  bDescriptorType;
        uint8_t  bDescriptorSubtype;
        uint8_t  bmCapabilities;
    }ACMFD;
    /* Union Functional Descriptor */
    PACKED(struct) {
        uint8_t  bFunctionLength;
        uint8_t  bDescriptorType;
        uint8_t  bDescriptorSubtype;
        uint8_t  bMasterInterface;
        uint8_t  bSlaveInterface0;
    }UFD;
#if (USBD_CDC_NOTEP_USED == 1)
    /* Notification Endpoint Descriptor */
    USB_EndpointDescType NED;
#endif
    /* Data Interface Descriptor */
    USB_InterfaceDescType DID;
    /* Endpoint descriptors are dynamically added */
}USBD_CDC_DescType;

static const USBD_CDC_DescType cdc_desc = {
    .IAD = { /* Interface Association Descriptor */
        .bLength            = sizeof(cdc_desc.IAD),
        .bDescriptorType    = USB_DESC_TYPE_IAD,
        .bFirstInterface    = 0,
        .bInterfaceCount    = 2,
        .bFunctionClass     = 0x02, /* bFunctionClass: Communication Interface Class */
        .bFunctionSubClass  = 0x02, /* bFunctionSubClass: Abstract Control Model */
        .bFunctionProtocol  = 0x01, /* bFunctionProtocol: Common AT commands */
        .iFunction          = USBD_ISTR_INTERFACES,
    },
    .CID = { /* Comm Interface Descriptor */
        .bLength            = sizeof(cdc_desc.CID),
        .bDescriptorType    = USB_DESC_TYPE_INTERFACE,
        .bInterfaceNumber   = 0,
        .bAlternateSetting  = 0,
#if (USBD_CDC_NOTEP_USED == 1)
        .bNumEndpoints      = 1,
#else
        .bNumEndpoints      = 0,
#endif
        .bInterfaceClass    = 0x02, /* bInterfaceClass: Communication Interface Class */
        .bInterfaceSubClass = 0x02, /* bInterfaceSubClass: Abstract Control Model */
        .bInterfaceProtocol = 0x01, /* bInterfaceProtocol: Common AT commands */
        .iInterface         = USBD_ISTR_INTERFACES,
    },
    .HFD = { /* Header Functional Descriptor */
        .bLength            = sizeof(cdc_desc.HFD),
        .bDescriptorType    = 0x24, /* bDescriptorType: CS_INTERFACE */
        .bDescriptorSubtype = 0x00, /* bDescriptorSubtype: Header Func Desc */
        .bcdCDC             = 0x110,/* bcdCDC: spec release number v1.10 */
    },
    .CMFD = { /* Call Management Functional Descriptor */
        .bFunctionLength    = sizeof(cdc_desc.CMFD),
        .bDescriptorType    = 0x24, /* bDescriptorType: CS_INTERFACE */
        .bDescriptorSubtype = 0x01, /* bDescriptorSubtype: Call Management Func Desc */
        .bmCapabilities     = 0x00, /* bmCapabilities */
        .bDataInterface     = 1,
    },
    .ACMFD = { /* ACM Functional Descriptor */
        .bFunctionLength    = sizeof(cdc_desc.ACMFD),
        .bDescriptorType    = 0x24, /* bDescriptorType: CS_INTERFACE */
        .bDescriptorSubtype = 0x02, /* bDescriptorSubtype: Abstract Control Management desc */
        .bmCapabilities     = 0x02, /* bmCapabilities:
                                        D3: NETWORK_CONNECTION notification support
                                        D2: SEND_BREAK request support
                                        D1: SET_LINE_CODING, SET_CONTROL_LINE_STATE, GET_LINE_CODING requests
                                            and SERIAL_STATE notification support
                                        D0: SET_COMM_FEATURE, CLEAR_COMM_FEATURE, and GET_COMM_FEATURE request support */
    },
    .UFD = { /* Union Functional Descriptor */
        .bFunctionLength    = sizeof(cdc_desc.UFD),
        .bDescriptorType    = 0x24, /* bDescriptorType: CS_INTERFACE */
        .bDescriptorSubtype = 0x06, /* bDescriptorSubtype: Union func desc */
        .bMasterInterface   = 0,
        .bSlaveInterface0   = 1,
    },
#if (USBD_CDC_NOTEP_USED == 1)
    .NED = { /* Notification Endpoint Descriptor */
        .bLength            = sizeof(cdc_desc.NED),
        .bDescriptorType    = USB_DESC_TYPE_ENDPOINT,
        .bEndpointAddress   = 0x82,
        .bmAttributes       = USB_EP_TYPE_INTERRUPT,
        .wMaxPacketSize     = CDC_NOT_PACKET_SIZE,
        .bInterval          = CDC_NOT_INTR_INTERVAL,
    },
#endif
    .DID = { /* Data class interface descriptor */
        .bLength = sizeof(cdc_desc.DID),
        .bDescriptorType = USB_DESC_TYPE_INTERFACE,
        .bInterfaceNumber   = 1,
        .bAlternateSetting  = 0,
        .bNumEndpoints      = 2,
        .bInterfaceClass    = 0x0A, /* bInterfaceClass: Data Interface Class */
        .bInterfaceSubClass = 0x00,
        .bInterfaceProtocol = 0x00,
        .iInterface         = USBD_ISTR_INTERFACES,
    },
};

static uint16_t         cdc_getDesc     (USBD_CDC_IfHandleType *itf, uint8_t ifNum, uint8_t * dest);
static const char *     cdc_getString   (USBD_CDC_IfHandleType *itf, uint8_t intNum);
static void             cdc_init        (USBD_CDC_IfHandleType *itf);
static void             cdc_deinit      (USBD_CDC_IfHandleType *itf);
static USBD_ReturnType  cdc_setupStage  (USBD_CDC_IfHandleType *itf);
static void             cdc_dataStage   (USBD_CDC_IfHandleType *itf);
static void             cdc_outData     (USBD_CDC_IfHandleType *itf, USBD_EpHandleType *ep);
static void             cdc_inData      (USBD_CDC_IfHandleType *itf, USBD_EpHandleType *ep);

/* CDC interface class callbacks structure */
static const USBD_ClassType cdc_cbks = {
    .GetDescriptor  = (USBD_IfDescCbkType)  cdc_getDesc,
    .GetString      = (USBD_IfStrCbkType)   cdc_getString,
    .Deinit         = (USBD_IfCbkType)      cdc_deinit,
    .SetupStage     = (USBD_IfSetupCbkType) cdc_setupStage,
    .DataStage      = (USBD_IfCbkType)      cdc_dataStage,
    .OutData        = (USBD_IfEpCbkType)    cdc_outData,
    .InData         = (USBD_IfEpCbkType)    cdc_inData,
};

/** @ingroup USBD_CDC
 * @defgroup USBD_CDC_Private_Functions CDC Private Functions
 * @{ */

/**
 * @brief Copies the interface descriptor to the destination buffer.
 * @param itf: reference of the CDC interface
 * @param ifNum: the index of the current interface in the device
 * @param dest: the destination buffer
 * @return Length of the copied descriptor
 */
static uint16_t cdc_getDesc(USBD_CDC_IfHandleType *itf, uint8_t ifNum, uint8_t * dest)
{
    USBD_CDC_DescType *desc = (USBD_CDC_DescType*)dest;
    uint16_t len = sizeof(cdc_desc);

    memcpy(dest, &cdc_desc, sizeof(cdc_desc));

#if (USBD_MAX_IF_COUNT > 2)
    /* Adjustment of interface indexes */
    desc->IAD.bFirstInterface  = ifNum;
    desc->IAD.iFunction  = USBD_IIF_INDEX(ifNum, 0);

    desc->CID.bInterfaceNumber = ifNum;
    desc->UFD.bMasterInterface = ifNum;

    desc->DID.bInterfaceNumber = ifNum + 1;
    desc->CMFD.bDataInterface  = ifNum + 1;
    desc->UFD.bSlaveInterface0 = ifNum + 1;

    desc->CID.iInterface = USBD_IIF_INDEX(ifNum, 0);
    desc->DID.iInterface = USBD_IIF_INDEX(ifNum, 0);
#endif /* (USBD_MAX_IF_COUNT > 2) */

#if (USBD_CDC_BREAK_SUPPORT == 1)
    if (CDC_APP(itf)->Break != NULL)
    {   desc->ACMFD.bmCapabilities |= 4; }
#endif /* USBD_CDC_BREAK_SUPPORT */

    if (itf->Config.Protocol != 0)
    {
        desc->IAD.bFunctionProtocol  = itf->Config.Protocol;
        desc->CID.bInterfaceProtocol = itf->Config.Protocol;
    }

#if (USBD_CDC_NOTEP_USED == 1)
    desc->NED.bEndpointAddress = itf->Config.NotEpNum;
#endif

    len += USBD_EpDesc(itf->Base.Device, itf->Config.OutEpNum, &dest[len]);
    len += USBD_EpDesc(itf->Base.Device, itf->Config.InEpNum, &dest[len]);

#if (USBD_HS_SUPPORT == 1)
    if (itf->Base.Device->Speed == USB_SPEED_FULL)
    {
        USB_EndpointDescType* ed = &dest[sizeof(cdc_desc)];
        ed[0].wMaxPacketSize = USB_EP_BULK_FS_MPS;
        ed[1].wMaxPacketSize = USB_EP_BULK_FS_MPS;
    }
#endif

    return len;
}

/**
 * @brief Returns the selected interface string.
 * @param itf: reference of the CDC interface
 * @param intNum: interface-internal string index
 * @return The referenced string
 */
static const char* cdc_getString(USBD_CDC_IfHandleType *itf, uint8_t intNum)
{
    return itf->App->Name;
}

/**
 * @brief Initializes the interface by opening its endpoints
 *        and initializing the attached application.
 * @param itf: reference of the CDC interface
 */
static void cdc_init(USBD_CDC_IfHandleType *itf)
{
    USBD_HandleType *dev = itf->Base.Device;
    uint16_t mps;

#if (USBD_HS_SUPPORT == 1)
    if (dev->Speed == USB_SPEED_HIGH)
    {
        mps = USB_EP_BULK_HS_MPS;
    }
    else
#endif
    {
        mps = USB_EP_BULK_FS_MPS;
    }

    /* Open EPs */
    USBD_EpOpen(dev, itf->Config.InEpNum , USB_EP_TYPE_BULK, mps);
    USBD_EpOpen(dev, itf->Config.OutEpNum, USB_EP_TYPE_BULK, mps);
#if (USBD_CDC_NOTEP_USED == 1)
    if ((itf->Config.NotEpNum & 0xF) < USBD_MAX_EP_COUNT)
    {
        USBD_EpOpen(dev, itf->Config.NotEpNum, USB_EP_TYPE_INTERRUPT, CDC_NOT_PACKET_SIZE);
    }
#endif

    /* Initialize application */
    USBD_SAFE_CALLBACK(CDC_APP(itf)->Open, itf, &itf->LineCoding);
}

/**
 * @brief Deinitializes the interface by closing its endpoints
 *        and deinitializing the attached application.
 * @param itf: reference of the CDC interface
 */
static void cdc_deinit(USBD_CDC_IfHandleType *itf)
{
    if (itf->LineCoding.DataBits != 0)
    {
        USBD_HandleType *dev = itf->Base.Device;

        /* Close EPs */
        USBD_EpClose(dev, itf->Config.InEpNum);
        USBD_EpClose(dev, itf->Config.OutEpNum);
#if (USBD_CDC_NOTEP_USED == 1)
        if ((itf->Config.NotEpNum & 0xF) < USBD_MAX_EP_COUNT)
        {
            USBD_EpClose(dev, itf->Config.NotEpNum);
        }
#endif

        /* Deinitialize application */
        USBD_SAFE_CALLBACK(CDC_APP(itf)->Close, itf);

#if (USBD_HS_SUPPORT == 1)
        /* Reset the endpoint MPS to the desired size */
        USBD_EpAddr2Ref(dev, itf->Config.InEpNum)->MaxPacketSize  = CDC_DATA_PACKET_SIZE;
        USBD_EpAddr2Ref(dev, itf->Config.OutEpNum)->MaxPacketSize = CDC_DATA_PACKET_SIZE;
#endif
        itf->LineCoding.DataBits = 0;
    }
}

/**
 * @brief Performs the interface-specific setup request handling.
 * @param itf: reference of the CDC interface
 * @return OK if the setup request is accepted, INVALID otherwise
 */
static USBD_ReturnType cdc_setupStage(USBD_CDC_IfHandleType *itf)
{
    USBD_ReturnType retval = USBD_E_INVALID;
    USBD_HandleType *dev = itf->Base.Device;

    switch (dev->Setup.RequestType.Type)
    {
        case USB_REQ_TYPE_CLASS:
        {
            switch (dev->Setup.Request)
            {
                case CDC_REQ_SET_LINE_CODING:
                    /* Reset the data interface */
                    cdc_deinit(itf);

                    retval = USBD_CtrlReceiveData(dev,
                            &itf->LineCoding, sizeof(itf->LineCoding));
                    break;

                case CDC_REQ_GET_LINE_CODING:
                    retval = USBD_CtrlSendData(dev,
                            &itf->LineCoding, sizeof(itf->LineCoding));
                    break;

                case CDC_REQ_SET_CONTROL_LINE_STATE:
#if (USBD_CDC_CONTROL_LINE_USED == 1)
                    if (CDC_APP(itf)->SetCtrlLine != NULL)
                    {
                        union {
                            struct {
                                uint16_t DTR : 1; /* Data Terminal Ready */
                                uint16_t RTS : 1; /* Request To Send */
                                uint16_t : 14;
                            }b;
                            uint16_t w;
                        } *ctrl = (void*)&dev->Setup.Value;

                        CDC_APP(itf)->SetCtrlLine(itf, ctrl->b.DTR, ctrl->b.RTS);
                    }
#endif /* USBD_CDC_CONTROL_LINE_USED */
                    retval = USBD_E_OK;
                    break;

#if (USBD_CDC_BREAK_SUPPORT == 1)
                case CDC_REQ_SEND_BREAK:
                    /* Simply pass the request with wValue */
                    if (CDC_APP(itf)->Break != NULL)
                    {
                        CDC_APP(itf)->Break(itf, dev->Setup.Value);
                        retval = USBD_E_OK;
                    }
                    break;
#endif /* USBD_CDC_BREAK_SUPPORT */

                default:
                    break;
            }
            break;
        }

        default:
            break;
    }

    return retval;
}

/**
 * @brief Passes the received control endpoint data to the application.
 * @param itf: reference of the CDC interface
 */
static void cdc_dataStage(USBD_CDC_IfHandleType *itf)
{
    USBD_HandleType *dev = itf->Base.Device;

    {
        if ((dev->Setup.Request == CDC_REQ_SET_LINE_CODING) &&
            (itf->LineCoding.DataBits != 0))
        {
            cdc_init(itf);
        }
    }
}

/**
 * @brief Notifies the application of a completed OUT transfer.
 * @param itf: reference of the CDC interface
 * @param ep: reference to the endpoint structure
 */
static void cdc_outData(USBD_CDC_IfHandleType *itf, USBD_EpHandleType *ep)
{
    USBD_SAFE_CALLBACK(CDC_APP(itf)->Received, itf,
            ep->Transfer.Data - ep->Transfer.Length, ep->Transfer.Length);
}

/**
 * @brief Notifies the application of a completed IN transfer.
 * @param itf: reference of the CDC interface
 * @param ep: reference to the endpoint structure
 */
static void cdc_inData(USBD_CDC_IfHandleType *itf, USBD_EpHandleType *ep)
{
#if (USBD_CDC_NOTEP_USED == 1)
    if (ep == USBD_EpAddr2Ref(itf->Base.Device, itf->Config.InEpNum))
#endif
    {
        uint16_t len = ep->Transfer.Length;

        if (len == 0)
        {
            /* if ZLP is finished, substitute original length */
            len = itf->TransmitLength;
            itf->TransmitLength = 0;
        }
        else if ((len & (ep->MaxPacketSize - 1)) == 0)
        {
            /* if length mod MPS == 0, split the transfer by sending ZLP */
            itf->TransmitLength = len;
            USBD_CDC_Transmit(itf, ep->Transfer.Data, 0);
        }

        /* callback when the endpoint isn't busy sending ZLP */
        if (ep->State != USB_EP_STATE_DATA)
        {
            USBD_SAFE_CALLBACK(CDC_APP(itf)->Transmitted, itf, ep->Transfer.Data - len, len);
        }
    }
}

/** @} */

/** @defgroup USBD_CDC_Exported_Functions CDC Exported Functions
 * @{ */

/**
 * @brief Mounts the CDC interface to the USB Device at the next two interface slots.
 * @note  The CDC class uses two device interface slots per software interface.
 * @note  The interface reference shall have its @ref USBD_CDC_IfHandleType::Config structure
 *        and @ref USBD_CDC_IfHandleType::App reference properly set before this function is called.
 * @param itf: reference of the CDC interface
 * @param dev: reference of the USB Device
 * @return OK if the mounting was successful,
 *         ERROR if it failed due to insufficient device interface slots
 */
USBD_ReturnType USBD_CDC_MountInterface(USBD_CDC_IfHandleType *itf, USBD_HandleType *dev)
{
    USBD_ReturnType retval = USBD_E_ERROR;

    /* Note: CDC uses 2 interfaces */
    if (dev->IfCount < (USBD_MAX_IF_COUNT - 1))
    {
        /* Binding interfaces */
        itf->Base.Device = dev;
        itf->Base.Class  = &cdc_cbks;
        itf->Base.AltCount = 1;
        itf->Base.AltSelector = 0;
        itf->TransmitLength = 0;

        {
            USBD_EpHandleType *ep;

#if (USBD_CDC_NOTEP_USED == 1)
            if ((itf->Config.NotEpNum & 0xF) < USBD_MAX_EP_COUNT)
            {
                ep = USBD_EpAddr2Ref(dev, itf->Config.NotEpNum);
                ep->Type            = USB_EP_TYPE_INTERRUPT;
                ep->MaxPacketSize   = CDC_NOT_PACKET_SIZE;
                ep->IfNum           = dev->IfCount;
            }
#endif

            ep = USBD_EpAddr2Ref(dev, itf->Config.InEpNum);
            ep->Type            = USB_EP_TYPE_BULK;
            ep->MaxPacketSize   = CDC_DATA_PACKET_SIZE;
            ep->IfNum           = dev->IfCount;

            ep = USBD_EpAddr2Ref(dev, itf->Config.OutEpNum);
            ep->Type            = USB_EP_TYPE_BULK;
            ep->MaxPacketSize   = CDC_DATA_PACKET_SIZE;
            ep->IfNum           = dev->IfCount;
        }

        dev->IF[dev->IfCount] = (USBD_IfHandleType*)itf;
        dev->IfCount++;

        dev->IF[dev->IfCount] = (USBD_IfHandleType*)itf;
        dev->IfCount++;

        retval = USBD_E_OK;
    }

    return retval;
}

/**
 * @brief Transmits data through the CDC IN endpoint.
 * @param itf: reference of the CDC interface
 * @param data: pointer to the data to send
 * @param length: length of the data
 * @return BUSY if the previous transfer is still ongoing, OK if successful
 */
USBD_ReturnType USBD_CDC_Transmit(USBD_CDC_IfHandleType *itf, uint8_t *data, uint16_t length)
{
    return USBD_EpSend(itf->Base.Device, itf->Config.InEpNum, data, length);
}

/**
 * @brief Receives data through the CDC OUT endpoint.
 * @param itf: reference of the CDC interface
 * @param data: pointer to the data to receive
 * @param length: length of the data
 * @return BUSY if the previous transfer is still ongoing, OK if successful
 */
USBD_ReturnType USBD_CDC_Receive(USBD_CDC_IfHandleType *itf, uint8_t *data, uint16_t length)
{
    return USBD_EpReceive(itf->Base.Device, itf->Config.OutEpNum, data, length);
}

#if (USBD_CDC_NOTEP_USED == 1)
/**
 * @brief Sends a device notification to the host.
 * @param itf: reference of the CDC interface
 * @param notice: pointer to the notification message to send
 * @return BUSY if the previous transfer is still ongoing, OK if successful
 */
USBD_ReturnType USBD_CDC_Notify(USBD_CDC_IfHandleType *itf, USBD_CDC_NotifyMessageType *notice)
{
    USBD_ReturnType retval = USBD_E_ERROR;

    if (itf->Config.NotEpNum < USBD_MAX_EP_COUNT)
    {
        uint16_t length = sizeof(notice->Header) + notice->Header.Length;
        retval = USBD_EpSend(itf->Base.Device, itf->Config.NotEpNum, notice, length);
    }

    return retval;
}
#endif

/** @} */
