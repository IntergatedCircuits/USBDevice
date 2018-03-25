/**
  ******************************************************************************
  * @file    usbd_cdc.c
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   USB Communications Device Class implementation
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
#include <usbd_internal.h>
#include <usbd_cdc.h>

#if (USBD_MAX_IF_COUNT < 2)
#error "A single CDC interface takes up 2 device interface slots!"
#endif

#if (USBD_CDC_NOTEP_USED == 1)
#define CDC_NOT_INTR_INTERVAL                       1
#else
#define CDC_NOT_INTR_INTERVAL                       0xFF
#endif
#define CDC_NOT_PACKET_SIZE                         8
#if (USBD_HS_SUPPORT == 1)
#define CDC_DATA_PACKET_SIZE                        USB_EP_BULK_HS_MPS
#else
#define CDC_DATA_PACKET_SIZE                        USB_EP_BULK_FS_MPS
#endif

#if (USBD_CDC_ALTSETTINGS != 0)
#define CDC_APP(ITF)    ((USBD_CDC_AppType*)(&(ITF)->App[(ITF)->Base.AltSelector]))
#else
#define CDC_APP(ITF)    ((USBD_CDC_AppType*)((ITF)->App))
#endif

typedef struct
{
    /* Interface Association Descriptor */
    USB_IfAssocDescType IAD;
    /* Communication Interface Descriptor */
    USB_InterfaceDescType CID;
    /* Header Functional Descriptor */
    struct {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint8_t bDescriptorSubtype;
        uint16_t bcdCDC;
    }__packed HFD;
    /* Call Management Functional Descriptor */
    struct {
        uint8_t  bFunctionLength;
        uint8_t  bDescriptorType;
        uint8_t  bDescriptorSubtype;
        uint8_t  bmCapabilities;
        uint8_t  bDataInterface;
    }__packed CMFD;
    /* ACM Functional Descriptor */
    struct {
        uint8_t  bFunctionLength;
        uint8_t  bDescriptorType;
        uint8_t  bDescriptorSubtype;
        uint8_t  bmCapabilities;
    }__packed ACMFD;
    /* Union Functional Descriptor */
    struct {
        uint8_t  bFunctionLength;
        uint8_t  bDescriptorType;
        uint8_t  bDescriptorSubtype;
        uint8_t  bMasterInterface;
        uint8_t  bSlaveInterface0;
    }__packed UFD;
    /* Notification Endpoint Descriptor */
    USB_EndpointDescType NED;
    /* Data Interface Descriptor */
    USB_InterfaceDescType DID;
    /* Endpoint descriptors are dynamically added */
}__packed USBD_CDC_DescType;

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
        .bNumEndpoints      = 1,
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
        .bmCapabilities     = 0x00, /* bmCapabilities: D0+D1 */
        .bDataInterface     = 1,
    },
    .ACMFD = { /* ACM Functional Descriptor */
        .bFunctionLength    = sizeof(cdc_desc.ACMFD),
        .bDescriptorType    = 0x24, /* bDescriptorType: CS_INTERFACE */
        .bDescriptorSubtype = 0x02, /* bDescriptorSubtype: Abstract Control Management desc */
        .bmCapabilities     = 0x02, /* bmCapabilities */
    },
    .UFD = { /* Union Functional Descriptor */
        .bFunctionLength    = sizeof(cdc_desc.UFD),
        .bDescriptorType    = 0x24, /* bDescriptorType: CS_INTERFACE */
        .bDescriptorSubtype = 0x06, /* bDescriptorSubtype: Union func desc */
        .bMasterInterface   = 0,
        .bSlaveInterface0   = 1,
    },
    .NED = { /* Notification Endpoint Descriptor */
        .bLength            = sizeof(cdc_desc.NED),
        .bDescriptorType    = USB_DESC_TYPE_ENDPOINT,
        .bEndpointAddress   = 0x82,
        .bmAttributes       = USB_EP_TYPE_INTERRUPT,
        .wMaxPacketSize     = CDC_NOT_PACKET_SIZE,
        .bInterval          = CDC_NOT_INTR_INTERVAL,
    },
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

#if (USBD_CDC_ALTSETTINGS != 0)
static uint16_t         cdc_getAltsDesc (USBD_CDC_IfHandleType *itf, uint8_t ifNum, uint8_t * dest);
#endif
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
#if (USBD_CDC_ALTSETTINGS != 0)
    .GetDescriptor  = (USBD_IfDescCbkType)  cdc_getAltsDesc,
#else
    .GetDescriptor  = (USBD_IfDescCbkType)  cdc_getDesc,
#endif
    .GetString      = (USBD_IfStrCbkType)   cdc_getString,
    .Init           = (USBD_IfCbkType)      cdc_init,
    .Deinit         = (USBD_IfCbkType)      cdc_deinit,
    .SetupStage     = (USBD_IfSetupCbkType) cdc_setupStage,
    .DataStage      = (USBD_IfCbkType)      cdc_dataStage,
    .OutData        = (USBD_IfEpCbkType)    cdc_outData,
    .InData         = (USBD_IfEpCbkType)    cdc_inData,
};

/** @ingroup USBD_CDC
 * @defgroup USBD_CDC_Private_Functions CDC Private Functions
 * @{ */

#if (USBD_CDC_ALTSETTINGS != 0)
/**
 * @brief Copies the interface descriptor to the destination buffer.
 * @param itf: reference of the CDC interface
 * @param ifNum: the index of the current interface in the device
 * @param dest: the destination buffer
 * @return Length of the copied descriptor
 */
static uint16_t cdc_getAltsDesc(USBD_CDC_IfHandleType *itf, uint8_t ifNum, uint8_t * dest)
{
    uint8_t as;
    uint16_t len = 0;

    /* Copy the descriptors many times, add alternate setting indexes */
    for (as = 0; as < itf->Base.AltCount; as++)
    {
        USBD_CDC_DescType *desc = (USBD_CDC_DescType*)&dest[len];
        len += cdc_getDesc(itf, ifNum, &dest[len]);

        desc->CID.bAlternateSetting = as;
        desc->DID.bAlternateSetting = as;

        desc->IAD.iFunction  = USBD_IIF_INDEX(ifNum, as);
        desc->CID.iInterface = USBD_IIF_INDEX(ifNum, as);
        desc->DID.iInterface = USBD_IIF_INDEX(ifNum, as);
    }
    return len;
}
#endif /* (USBD_CDC_ALTSETTINGS != 0) */

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

    if (itf->Config.Protocol != 0)
    {
        desc->CID.bInterfaceProtocol = itf->Config.Protocol;
    }

    desc->NED.bEndpointAddress = itf->Config.NotEpNum;

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
#if (USBD_CDC_ALTSETTINGS != 0)
    if (intNum < itf->Base.AltCount)
    {
        return itf->App[intNum].Name;
    }
    else
    {
        return NULL;
    }
#else
    return itf->App->Name;
#endif
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
    if (itf->Base.Device->Speed == USB_SPEED_HIGH)
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
    USBD_EpOpen(dev, itf->Config.NotEpNum, USB_EP_TYPE_INTERRUPT, CDC_NOT_PACKET_SIZE);
#endif

    /* Initialize application */
    USBD_SAFE_CALLBACK(CDC_APP(itf)->Init, );
}

/**
 * @brief Deinitializes the interface by closing its endpoints
 *        and deinitializing the attached application.
 * @param itf: reference of the CDC interface
 */
static void cdc_deinit(USBD_CDC_IfHandleType *itf)
{
    USBD_HandleType *dev = itf->Base.Device;

    /* Close EPs */
    USBD_EpClose(dev, itf->Config.InEpNum);
    USBD_EpClose(dev, itf->Config.OutEpNum);
#if (USBD_CDC_NOTEP_USED == 1)
    USBD_EpClose(dev, itf->Config.NotEpNum);
#endif

    /* Deinitialize application */
    USBD_SAFE_CALLBACK(CDC_APP(itf)->Deinit, );

#if (USBD_HS_SUPPORT == 1)
    /* Reset the endpoint MPS to the desired size */
    dev->EP.IN [itf->Config.InEpNum  & 0xF].MaxPacketSize = 
    dev->EP.OUT[itf->Config.OutEpNum      ].MaxPacketSize = CDC_DATA_PACKET_SIZE;
#endif
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
            /* Data stage is upcoming */
            if (dev->Setup.Length > 0)
            {
                if (dev->Setup.RequestType.Direction == USB_DIRECTION_IN)
                {
                    /* Get the data to send */
                    CDC_APP(itf)->Control(&dev->Setup, dev->CtrlData);

                    retval = USBD_CtrlSendData(dev, dev->CtrlData, dev->Setup.Length);
                }
                else
                {
                    /* Receive Control data first */
                    retval = USBD_CtrlReceiveData(dev, dev->CtrlData);
                }
            }
            else
            {
                /* Simply pass the request with wValue */
                CDC_APP(itf)->Control(&dev->Setup, (uint8_t*) &dev->Setup.Value);

                /* Accept all class requests */
                retval = USBD_E_OK;
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

    if (dev->Setup.RequestType.Direction == USB_DIRECTION_OUT)
    {
        /* Hand over received data to App */
        CDC_APP(itf)->Control(&dev->Setup, dev->CtrlData);
    }
}

/**
 * @brief Notifies the application of a completed OUT transfer.
 * @param itf: reference of the CDC interface
 * @param ep: reference to the endpoint structure
 */
static void cdc_outData(USBD_CDC_IfHandleType *itf, USBD_EpHandleType *ep)
{
    USBD_SAFE_CALLBACK(CDC_APP(itf)->Received,
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
    if (ep == &itf->Base.Device->EP.IN[itf->Config.InEpNum & 0xF])
#endif
    {
        USBD_SAFE_CALLBACK(CDC_APP(itf)->Transmitted,
                ep->Transfer.Data - ep->Transfer.Length, ep->Transfer.Length);
    }
}

/** @} */

/** @defgroup USBD_CDC_Exported_Functions CDC Exported Functions
 * @{ */

/**
 * @brief Mounts the CDC interface to the USB Device at the next two interface slots.
 * @note  The CDC class uses two device interface slots per software interface.
 * @note  The interface reference shall have its Config structure and App reference
 *        properly set before this function is called.
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

        {
            USBD_EpHandleType *ep;

#if (USBD_CDC_NOTEP_USED == 1)
            ep = &dev->EP.IN [CDC_APP(itf)->NotEpNum & 0xF];
            ep->Type            = USB_EP_TYPE_INTERRUPT;
            ep->MaxPacketSize   = CDC_NOT_PACKET_SIZE;
            ep->IfNum           = dev->IfCount;
#endif

            ep = &dev->EP.IN [itf->Config.InEpNum  & 0xF];
            ep->Type            = USB_EP_TYPE_BULK;
            ep->MaxPacketSize   = CDC_DATA_PACKET_SIZE;
            ep->IfNum           = dev->IfCount;

            ep = &dev->EP.OUT[itf->Config.OutEpNum];
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

/** @} */
