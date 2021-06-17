/**
  ******************************************************************************
  * @file    usbd_ncm.c
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-06-02
  * @brief   USB CDC Network Control Model implementation
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
#include <usbd_ncm.h>
#include <usbd_utils.h>

#if (USBD_MAX_IF_COUNT < 2)
#error "A single NCM interface takes up 2 device interface slots!"
#endif

#define NCM_NOT_INTR_INTERVAL   20
#define NCM_NOT_PACKET_SIZE     8
#if (USBD_HS_SUPPORT == 1)
#define NCM_DATA_PACKET_SIZE    USB_EP_BULK_HS_MPS
#else
#define NCM_DATA_PACKET_SIZE    USB_EP_BULK_FS_MPS
#endif

#define NCM_MAX_SEGMENT_SIZE    1514

#define NCM_NET_ADDRESS_SIZE    6

#define NCM_APP(ITF)    ((USBD_NCM_AppType*)((ITF)->App))

typedef enum
{
    NTB_EMPTY,
    NTB_PROCESSING,
    NTB_TRANSFERRING,
    NTB_READY
}NTBStateType;


typedef union {
    uint8_t b[4];
    uint32_t dw;
}USBD_NCM_SignatureType;


/* NTB Header */
typedef union
{
    PACKED(struct) {
        uint32_t Signature;     /* "NCMH" */
        uint16_t HeaderLength;  /* Size in bytes of this NTH16 structure */
        uint16_t Sequence;      /* Sequence number (for debugging) */
        uint16_t BlockLength;   /* Size of this NTB in bytes */
        uint16_t NdpIndex;      /* Offset of the first NDP16 from byte zero of the NTB */
    }V16;
    PACKED(struct) {
        uint32_t Signature;     /* "ncmh" */
        uint16_t HeaderLength;  /* Size in bytes of this NTH32 structure */
        uint16_t Sequence;      /* Sequence number (for debugging) */
        uint32_t BlockLength;   /* Size of this NTB in bytes */
        uint32_t NdpIndex;      /* Offset of the first NDP32 from byte zero of the NTB */
    }V32;
}USBD_NCM_TransferHeaderType;


typedef PACKED(struct) {
    uint16_t Index; /* Byte index of the datagram in the NTB */
    uint16_t Length;/* Size in bytes of the datagram in the NTB */
}USBD_NCM_Datagram16;


typedef PACKED(struct) {
    uint32_t Index; /* Byte index of the datagram in the NTB */
    uint32_t Length;/* Size in bytes of the datagram in the NTB */
}USBD_NCM_Datagram32;


/* NTB Datagram Pointer Table */
typedef union
{
    PACKED(struct) {
        uint32_t Signature;     /* "NCM0" for no CRC, "NCM1" for CRC-32 */
        uint16_t Length;        /* Size in bytes of this NDP16 structure */
        uint16_t NextNdpIndex;  /* Offset of the next NDP16 from byte zero of the NTB */
        USBD_NCM_Datagram16 Datagram[1]; /* The last datagram index + header is always 0 */
    }V16;
    PACKED(struct) {
        uint32_t Signature;     /*"ncm0" for no CRC, "ncm1" for CRC-32 */
        uint16_t Length;        /* Size in bytes of this NDP32 structure */
        uint16_t Reserved6;     /* Keep 0 */
        uint32_t NextNdpIndex;  /* Offset of the next NDP32 from byte zero of the NTB */
        uint32_t Reserved12;    /* Keep 0 */
        USBD_NCM_Datagram32 Datagram[1]; /* The last datagram index + header is always 0 */
    }V32;
}USBD_NCM_DatagramPointerTableType;


/* NTB Parameters */
typedef PACKED(struct)
{
    uint16_t Length;                /* Size in bytes of this NTBT structure */
    uint16_t NtbFormatsSupported;   /* 1 if only 16bit, 3 if 32bit is supported as well */
    uint32_t NtbInMaxSize;          /* IN NTB Maximum Size in bytes */
    uint16_t NdpInDivisor;          /* Divisor used for IN NTB Datagram payload alignment */
    uint16_t NdpInPayloadRemainder; /* Remainder used to align input datagram payload within the NTB */
    uint16_t NdpInAlignment;        /* Datagram alignment */
    uint16_t reserved;              /* Keep 0 */
    uint32_t NtbOutMaxSize;
    uint16_t NdpOutDivisor;
    uint16_t NdpOutPayloadRemainder;
    uint16_t NdpOutAlignment;
    uint16_t NtbOutMaxDatagrams;    /* Maximum number of datagrams in a single OUT NTB */
}USBD_NCM_ParametersType;


/* NTB Parameters */
typedef PACKED(struct)
{
    uint32_t size;
}USBD_NCM_NTB_InputSize;


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
    /* Union Functional Descriptor */
    PACKED(struct) {
        uint8_t  bFunctionLength;
        uint8_t  bDescriptorType;
        uint8_t  bDescriptorSubtype;
        uint8_t  bMasterInterface;
        uint8_t  bSlaveInterface0;
    }UFD;
    /* Ethernet Networking Functional Descriptor */
    PACKED(struct) {
        uint8_t  bFunctionLength;
        uint8_t  bDescriptorType;
        uint8_t  bDescriptorSubtype;
        uint8_t  iMACAddress;
        uint32_t bmEthernetStatistics;
        uint16_t wMaxSegmentSize;
        uint16_t wNumberMCFilters;
        uint8_t  bNumberPowerFilters;
    }ENFD;
    /* NCM Functional Descriptor */
    PACKED(struct) {
        uint8_t  bFunctionLength;
        uint8_t  bDescriptorType;
        uint8_t  bDescriptorSubtype;
        uint16_t bcdNcmVersion;
        uint8_t  bmNetworkCapabilities;
    }NCMFD;
    /* Notification Endpoint Descriptor */
    USB_EndpointDescType NED;
    /* Data Interface Descriptors */
    USB_InterfaceDescType DID0;
    USB_InterfaceDescType DID;
    /* Endpoint descriptors are dynamically added */
}USBD_NCM_DescType;


static const USBD_NCM_SignatureType nth16_sign =
{ .b = {'N', 'C', 'M', 'H'} };

static const USBD_NCM_SignatureType ndp16_sign =
{ .b = {'N', 'C', 'M', '0'} };

static const USBD_NCM_DescType ncm_desc = {
    .IAD = { /* Interface Association Descriptor */
        .bLength            = sizeof(ncm_desc.IAD),
        .bDescriptorType    = USB_DESC_TYPE_IAD,
        .bFirstInterface    = 0,
        .bInterfaceCount    = 2,
        .bFunctionClass     = 0x02, /* bFunctionClass: Communication Interface Class */
        .bFunctionSubClass  = 0x0D, /* bFunctionSubClass: Network Control Model */
        .bFunctionProtocol  = 0x00, /* bFunctionProtocol: none */
        .iFunction          = USBD_ISTR_INTERFACES,
    },
    .CID = { /* Comm Interface Descriptor */
        .bLength            = sizeof(ncm_desc.CID),
        .bDescriptorType    = USB_DESC_TYPE_INTERFACE,
        .bInterfaceNumber   = 0,
        .bAlternateSetting  = 0,
        .bNumEndpoints      = 1,
        .bInterfaceClass    = 0x02, /* bInterfaceClass: Communication Interface Class */
        .bInterfaceSubClass = 0x0D, /* bInterfaceSubClass: Network Control Model */
        .bInterfaceProtocol = 0x00, /* bInterfaceProtocol: none */
        .iInterface         = USBD_ISTR_INTERFACES,
    },
    .HFD = { /* Header Functional Descriptor */
        .bLength            = sizeof(ncm_desc.HFD),
        .bDescriptorType    = 0x24, /* bDescriptorType: CS_INTERFACE */
        .bDescriptorSubtype = 0x00, /* bDescriptorSubtype: Header Func Desc */
        .bcdCDC             = 0x110,/* bcdCDC: spec release number v1.10 */
    },
    .UFD = { /* Union Functional Descriptor */
        .bFunctionLength    = sizeof(ncm_desc.UFD),
        .bDescriptorType    = 0x24, /* bDescriptorType: CS_INTERFACE */
        .bDescriptorSubtype = 0x06, /* bDescriptorSubtype: Union func desc */
        .bMasterInterface   = 0,
        .bSlaveInterface0   = 1,
    },
    .ENFD = { /* Ethernet Networking Functional Descriptor */
        .bFunctionLength        = sizeof(ncm_desc.ENFD),
        .bDescriptorType        = 0x24, /* bDescriptorType: CS_INTERFACE */
        .bDescriptorSubtype     = 0x0F, /* bDescriptorSubtype: Ethernet Networking func desc */
        .iMACAddress            = USBD_IIF_INDEX(0, 1),
        .bmEthernetStatistics   = 0,
        .wMaxSegmentSize        = NCM_MAX_SEGMENT_SIZE, /* The maximum supported segment size */
        .wNumberMCFilters       = 0,
        .bNumberPowerFilters    = 0,
    },
    .NCMFD = { /* NCM Functional Descriptor */
        .bFunctionLength        = sizeof(ncm_desc.NCMFD),
        .bDescriptorType        = 0x24, /* bDescriptorType: CS_INTERFACE */
        .bDescriptorSubtype     = 0x1A, /* bDescriptorSubtype: NCM func desc */
        .bcdNcmVersion          = 0x0100,
        .bmNetworkCapabilities  = 0, /* D1: GetNetAddress and SetNetAddress requests support */
    },
    .NED = { /* Notification Endpoint Descriptor */
        .bLength            = sizeof(ncm_desc.NED),
        .bDescriptorType    = USB_DESC_TYPE_ENDPOINT,
        .bEndpointAddress   = 0x82,
        .bmAttributes       = USB_EP_TYPE_INTERRUPT,
        .wMaxPacketSize     = NCM_NOT_PACKET_SIZE,
        .bInterval          = NCM_NOT_INTR_INTERVAL,
    },
    .DID0 = { /* Dummy data class interface descriptor */
        .bLength = sizeof(ncm_desc.DID0),
        .bDescriptorType = USB_DESC_TYPE_INTERFACE,
        .bInterfaceNumber   = 1,
        .bAlternateSetting  = 0,
        .bNumEndpoints      = 0,
        .bInterfaceClass    = 0x0A, /* bInterfaceClass: Data Interface Class */
        .bInterfaceSubClass = 0x00,
        .bInterfaceProtocol = 0x01, /* bInterfaceProtocol: Network Transfer Block */
        .iInterface         = 0,
    },
    .DID = { /* Data class interface descriptor */
        .bLength = sizeof(ncm_desc.DID),
        .bDescriptorType = USB_DESC_TYPE_INTERFACE,
        .bInterfaceNumber   = 1,
        .bAlternateSetting  = 1,
        .bNumEndpoints      = 2,
        .bInterfaceClass    = 0x0A, /* bInterfaceClass: Data Interface Class */
        .bInterfaceSubClass = 0x00,
        .bInterfaceProtocol = 0x01, /* bInterfaceProtocol: Network Transfer Block */
        .iInterface         = USBD_ISTR_INTERFACES,
    },
};

static uint16_t         ncm_getDesc     (USBD_NCM_IfHandleType *itf, uint8_t ifNum, uint8_t * dest);
static const char *     ncm_getString   (USBD_NCM_IfHandleType *itf, uint8_t intNum);
static void             ncm_init        (USBD_NCM_IfHandleType *itf);
static void             ncm_deinit      (USBD_NCM_IfHandleType *itf);
static USBD_ReturnType  ncm_setupStage  (USBD_NCM_IfHandleType *itf);
static void             ncm_dataStage   (USBD_NCM_IfHandleType *itf);
static void             ncm_outData     (USBD_NCM_IfHandleType *itf, USBD_EpHandleType *ep);
static void             ncm_inData      (USBD_NCM_IfHandleType *itf, USBD_EpHandleType *ep);
static void             ncm_sendNTB     (USBD_NCM_IfHandleType *itf, uint8_t page);

/* NCM interface class callbacks structure */
static const USBD_ClassType ncm_cbks = {
    .GetDescriptor  = (USBD_IfDescCbkType)  ncm_getDesc,
    .GetString      = (USBD_IfStrCbkType)   ncm_getString,
    .Init           = (USBD_IfCbkType)      ncm_init,
    .Deinit         = (USBD_IfCbkType)      ncm_deinit,
    .SetupStage     = (USBD_IfSetupCbkType) ncm_setupStage,
    .DataStage      = (USBD_IfCbkType)      ncm_dataStage,
    .OutData        = (USBD_IfEpCbkType)    ncm_outData,
    .InData         = (USBD_IfEpCbkType)    ncm_inData,
#if (USBD_MS_OS_DESC_VERSION > 0)
    .MsCompatibleId = "WINNCM",
#else
#warning "Without Microsoft OS descriptor support the UsbNcm function driver will have to be installed manually on Windows OS!"
#endif /* (USBD_MS_OS_DESC_VERSION > 0) */
};

/** @ingroup USBD_NCM
 * @defgroup USBD_NCM_Private_Functions NCM Private Functions
 * @{ */

/**
 * @brief Copies the interface descriptor to the destination buffer.
 * @param itf: reference of the NCM interface
 * @param ifNum: the index of the current interface in the device
 * @param dest: the destination buffer
 * @return Length of the copied descriptor
 */
static uint16_t ncm_getDesc(USBD_NCM_IfHandleType *itf, uint8_t ifNum, uint8_t * dest)
{
    USBD_NCM_DescType *desc = (USBD_NCM_DescType*)dest;
    uint16_t len = sizeof(ncm_desc);

    memcpy(dest, &ncm_desc, sizeof(ncm_desc));

#if (USBD_MAX_IF_COUNT > 2)
    /* Adjustment of interface indexes */
    desc->IAD.bFirstInterface  = ifNum;
    desc->IAD.iFunction = USBD_IIF_INDEX(ifNum, 0);

    desc->CID.bInterfaceNumber = ifNum;
    desc->UFD.bMasterInterface = ifNum;

    desc->DID0.bInterfaceNumber = ifNum + 1;
    desc->DID.bInterfaceNumber = ifNum + 1;
    desc->UFD.bSlaveInterface0 = ifNum + 1;

    desc->CID.iInterface = USBD_IIF_INDEX(ifNum, 0);
    desc->DID.iInterface = USBD_IIF_INDEX(ifNum, 0);
    desc->ENFD.iMACAddress = USBD_IIF_INDEX(ifNum, 1);
#endif /* (USBD_MAX_IF_COUNT > 2) */

    desc->NED.bEndpointAddress = itf->Config.NotEpNum;

    len += USBD_EpDesc(itf->Base.Device, itf->Config.OutEpNum, &dest[len]);
    len += USBD_EpDesc(itf->Base.Device, itf->Config.InEpNum, &dest[len]);

#if (USBD_HS_SUPPORT == 1)
    if (itf->Base.Device->Speed == USB_SPEED_FULL)
    {
        USB_EndpointDescType* ed = &dest[sizeof(ncm_desc)];
        ed[0].wMaxPacketSize = USB_EP_BULK_FS_MPS;
        ed[1].wMaxPacketSize = USB_EP_BULK_FS_MPS;
    }
#endif

    return len;
}

/**
 * @brief Returns the selected interface string (either the name or the MAC address).
 * @param itf: reference of the NCM interface
 * @param intNum: interface-internal string index
 * @return The referenced string
 */
static const char* ncm_getString(USBD_NCM_IfHandleType *itf, uint8_t intNum)
{
    const char* str;
    if (intNum == 0)
    {
        str = NCM_APP(itf)->Name;
    }
    else
    {
        /* Allocate string after the end of the string descriptor */
        str = (void*)itf->Base.Device->CtrlData + 2 +
                sizeof(*NCM_APP(itf)->NetAddress) * 4;
        Uint2Unicode((void*)NCM_APP(itf)->NetAddress, (void*)str,
                2 * sizeof(*NCM_APP(itf)->NetAddress));
    }
    return str;
}

/**
 * @brief Initializes the interface by opening its notification endpoint
 *        and initializing the attached application (only if alt selector == 1).
 * @param itf: reference of the NCM interface
 */
static void ncm_init(USBD_NCM_IfHandleType *itf)
{
    if (itf->Base.AltSelector == 1)
    {
        /* Open notification EP */
        USBD_EpOpen(itf->Base.Device, itf->Config.NotEpNum,
                USB_EP_TYPE_INTERRUPT, NCM_NOT_PACKET_SIZE);

        /* Initialize application */
        USBD_SAFE_CALLBACK(NCM_APP(itf)->Init, itf);
    }
    else
    {
        /* Set initial max IN NTB size */
        itf->In.MaxSize = sizeof(itf->In.Data[0]);

        /* TODO reset MAC address to default */
    }
}

/**
 * @brief Deinitializes the interface by closing its endpoints
 *        and deinitializing the attached application (only if alt selector == 1).
 * @param itf: reference of the NCM interface
 */
static void ncm_deinit(USBD_NCM_IfHandleType *itf)
{
    itf->Notify.Connection.Value = 0;

    if (itf->Base.AltSelector == 1)
    {
        USBD_HandleType *dev = itf->Base.Device;

        /* Close all EPs */
        USBD_EpClose(dev, itf->Config.InEpNum);
        USBD_EpClose(dev, itf->Config.OutEpNum);
        USBD_EpClose(dev, itf->Config.NotEpNum);

        /* Deinitialize application */
        USBD_SAFE_CALLBACK(NCM_APP(itf)->Deinit, itf);

#if (USBD_HS_SUPPORT == 1)
        /* Reset the endpoint MPS to the desired size */
        USBD_EpAddr2Ref(dev, itf->Config.InEpNum)->MaxPacketSize  = NCM_DATA_PACKET_SIZE;
        USBD_EpAddr2Ref(dev, itf->Config.OutEpNum)->MaxPacketSize = NCM_DATA_PACKET_SIZE;
#endif
    }
}

/**
 * @brief Performs the interface-specific setup request handling.
 * @param itf: reference of the NCM interface
 * @return OK if the setup request is accepted, INVALID otherwise
 */
static USBD_ReturnType ncm_setupStage(USBD_NCM_IfHandleType *itf)
{
    USBD_ReturnType retval = USBD_E_INVALID;
    USBD_HandleType *dev = itf->Base.Device;

    if (dev->Setup.RequestType.Type == USB_REQ_TYPE_CLASS)
    {
        switch (dev->Setup.Request)
        {
            /* Mandatory requests */
            case CDC_REQ_GET_NTB_PARAMETERS:
            {
                USBD_NCM_ParametersType* param  = (void*)dev->CtrlData;

                param->Length                   = sizeof(USBD_NCM_ParametersType);
                param->NtbFormatsSupported      = 1; /* only 16 bit NTB for now */
                param->NtbInMaxSize             = USBD_NCM_MAX_IN_SIZE;
                param->NdpInDivisor             = 4;
                param->NdpInAlignment           = 4;
                param->NdpInPayloadRemainder    = 0;
                param->NtbOutMaxSize            = USBD_NCM_MAX_OUT_SIZE;
                param->NdpOutDivisor            = 4;
                param->NdpOutAlignment          = 4;
                param->NdpOutPayloadRemainder   = 0;
                param->NtbOutMaxDatagrams       = 20; /* Number is arbitrary, any number is supported */
                param->reserved                 = 0;

                retval = USBD_CtrlSendData(dev, dev->CtrlData, sizeof(*param));
                break;
            }

            case CDC_REQ_GET_NTB_INPUT_SIZE:
            {
                USBD_NCM_NTB_InputSize *insize = (void*)dev->CtrlData;
                insize->size = itf->In.MaxSize;
                retval = USBD_CtrlSendData(dev, dev->CtrlData, sizeof(*insize));
                break;
            }

            case CDC_REQ_SET_NTB_INPUT_SIZE:
            {
                retval = USBD_CtrlReceiveData(dev, dev->CtrlData, sizeof(USBD_NCM_NTB_InputSize));
                break;
            }

#if 0
            /* Optional requests */
            case CDC_REQ_GET_NET_ADDRESS:
                if (dev->Setup.Length == NCM_NET_ADDRESS_SIZE)
                {
                    /* TODO */
                }
                break;

            case CDC_REQ_SET_NET_ADDRESS:
                /* Only accepted while data interface alt sel = 0
                 * doesn't change iMACAddress value */
                if ((dev->Setup.Length == NCM_NET_ADDRESS_SIZE) &&
                    (itf->Base.AltSelector == 0))
                {
                    retval = USBD_CtrlReceiveData(dev, dev->CtrlData, NCM_NET_ADDRESS_SIZE);
                }
                break;

            case CDC_REQ_GET_NTB_FORMAT:
                *((uint16_t*)dev->CtrlData) = 0;
                retval = USBD_CtrlSendData(dev, dev->CtrlData, 2);
                break;

            case CDC_REQ_SET_NTB_FORMAT:
                /* Value = 1 for 32 bit format */
                if (dev->Setup.Value == 0)
                {   retval = USBD_E_OK; }
                break;
#endif
            default:
                break;
        }
    }

    return retval;
}

/**
 * @brief Updates interface from data received through the control pipe.
 * @param itf: reference of the NCM interface
 */
static void ncm_dataStage(USBD_NCM_IfHandleType *itf)
{
    USBD_HandleType *dev = itf->Base.Device;

    if (dev->Setup.RequestType.Direction == USB_DIRECTION_OUT)
    {
        switch (dev->Setup.Request)
        {
            case CDC_REQ_SET_NTB_INPUT_SIZE:
            {
                USBD_NCM_NTB_InputSize *insize = (void*)dev->CtrlData;
                /* Sanity check */
                if (insize->size > (sizeof(((USBD_NCM_TransferHeaderType*)0)->V16) +
                                    sizeof(((USBD_NCM_DatagramPointerTableType*)0)->V16)))
                {
                    itf->In.MaxSize = insize->size;
                }
                break;
            }
#if 0
            case CDC_REQ_SET_NET_ADDRESS:
                /* TODO */
                break;
#endif
            default:
                break;
        }
    }
}

/**
 * @brief Starts another NTB OUT transfer if the other buffer page is empty
 *        and sets the ready flag for the current page for application consumption.
 * @param itf: reference of the NCM interface
 * @param ep: reference to the endpoint structure
 */
static void ncm_outData(USBD_NCM_IfHandleType *itf, USBD_EpHandleType *ep)
{
    /* Save transfer details */
    USBD_NCM_TransferHeaderType* nth = (void*)ep->Transfer.Data - ep->Transfer.Length;
    uint8_t rxPage = ((void*)itf->Out.Data[0] == (void*)nth) ? 0 : 1, page = rxPage;
    itf->Out.State[rxPage] = NTB_EMPTY;

    /* Verify header */
    if ((nth->V16.BlockLength  == ep->Transfer.Length) &&
        (nth->V16.HeaderLength == sizeof(nth->V16)) &&
        (nth->V16.Signature    == nth16_sign.dw))
    {
        uint16_t ndpIndex = nth->V16.NdpIndex;
        USBD_NCM_DatagramPointerTableType* pt = (void*)nth + ndpIndex;

        /* Verify all tables in advance */
        while ((ndpIndex >= sizeof(nth->V16)) &&
               (ndpIndex < nth->V16.BlockLength) &&
               (pt->V16.Signature == ndp16_sign.dw) &&
               (pt->V16.Length > sizeof(pt->V16)))
        {
            ndpIndex = pt->V16.NextNdpIndex;
            pt = (void*)nth + ndpIndex;

            /* Indicates last NDP */
            if (ndpIndex == 0)
            {
                itf->Out.State[rxPage] = NTB_READY;
                page = 1 - rxPage;

                /* If the previous buffer is processed, pass the newly received */
                if (itf->Out.State[page] == NTB_EMPTY)
                {
                    itf->Out.Page = rxPage;
                    itf->Out.PT = (void*)nth + nth->V16.NdpIndex;
                    itf->Out.Dx = 0;
                }
                break;
            }
        }
    }

    /* Receive to empty page */
    if ((itf->Out.State[page] == NTB_EMPTY) &&
        (itf->Notify.Connection.Value != 0))
    {
        /* Start receiving in other buffer */
        USBD_EpReceive(itf->Base.Device, itf->Config.OutEpNum,
                itf->Out.Data[page], sizeof(itf->Out.Data[0]));

        itf->Out.State[page] = NTB_TRANSFERRING;
    }

    /* Signal the availability of new datagram(s) */
    if ((NCM_APP(itf)->Received != NULL) &&
        (itf->Out.State[rxPage] == NTB_READY))
    {
        NCM_APP(itf)->Received(itf);
    }
}

/**
 * @brief Sets the missing NTB fields and sends it through the data IN endpoint.
 * @param itf: reference of the NCM interface
 * @param page: the IN buffer page to send
 */
static void ncm_sendNTB(USBD_NCM_IfHandleType *itf, uint8_t page)
{
    USBD_NCM_TransferHeaderType* nth = (void*)itf->In.Data[page];
    USBD_NCM_DatagramPointerTableType* pt = (void*)nth + itf->In.Index;
    struct {
        uint16_t length;
        uint16_t reserved;
    }*dp = (void*)nth + sizeof(itf->In.Data[0]) - sizeof(*dp);
    USBD_NCM_Datagram16 *dg = &pt->V16.Datagram[0];
    int i;

    /* Set table header */
    pt->V16.Signature       = ndp16_sign.dw;
    pt->V16.Length          = sizeof(pt->V16) + (itf->In.DgCount * sizeof(pt->V16.Datagram[0]));
    pt->V16.NextNdpIndex    = 0;

    /* Set header fields */
    nth->V16.Signature      = nth16_sign.dw;
    nth->V16.HeaderLength   = sizeof(nth->V16);
    nth->V16.BlockLength    = sizeof(nth->V16) + itf->In.Index + pt->V16.Length;
    nth->V16.NdpIndex       = itf->In.Index;

    /* 2 rounds are used so the dp list entries aren't overwritten */
    for (i = 0; i < itf->In.DgCount; i++)
    {
        pt->V16.Datagram[i].Length = dp[-i].length;
    }

    dg->Index = sizeof(nth->V16);
    for (i = 1; i < itf->In.DgCount; i++, dg++)
    {
        (dg + 1)->Index = (dg->Index + dg->Length + 3) & (~3);
    }

    /* A null element ends the table */
    dg++;
    dg->Index = 0;
    dg->Length = 0;

    /* This should never fail */
    USBD_EpSend(itf->Base.Device, itf->Config.InEpNum,
            itf->In.Data[page], nth->V16.BlockLength);

    /* Switch to the other page */
    itf->In.Page    = 1 - page;
    itf->In.DgCount = 0;
    itf->In.Index   = sizeof(nth->V16);
    itf->In.RemSize = itf->In.MaxSize - itf->In.Index - sizeof(pt->V16);

    itf->In.FillState = NTB_EMPTY;
    itf->In.SendState = NTB_TRANSFERRING;

    /* Increment sequence number by 2 (double buffering) */
    nth->V16.Sequence += 2;
}

/**
 * @brief Transmits the other IN NTB page if it's ready.
 * @param itf: reference of the NCM interface
 * @param ep: reference to the endpoint structure
 */
static void ncm_inData(USBD_NCM_IfHandleType *itf, USBD_EpHandleType *ep)
{
    /* NTB sending is considered at the end of both notification
     * and data EP transfer, this is done to ensure the connect
     * notification reaches the host before any NTBs */
    {
        /* Set the state of this buffer to empty */
        itf->In.SendState = NTB_EMPTY;

        /* Send the other page if it's ready */
        if (itf->In.FillState == NTB_READY)
        {
            void* nth = ep->Transfer.Data - ep->Transfer.Length;
            uint8_t page = ((void*)itf->In.Data[0] == nth) ? 1 : 0;
            ncm_sendNTB(itf, page);
        }
    }
}

/** @} */

/** @defgroup USBD_NCM_Exported_Functions NCM Exported Functions
 * @{ */

/**
 * @brief Mounts the NCM interface to the USB Device at the next two interface slots.
 * @note  The NCM class uses two device interface slots per software interface.
 * @note  The interface reference shall have its @ref USBD_NCM_IfHandleType::Config structure
 *        and @ref USBD_NCM_IfHandleType::App reference properly set before this function is called.
 * @param itf: reference of the NCM interface
 * @param dev: reference of the USB Device
 * @return OK if the mounting was successful,
 *         ERROR if it failed due to insufficient device interface slots
 */
USBD_ReturnType USBD_NCM_MountInterface(USBD_NCM_IfHandleType *itf, USBD_HandleType *dev)
{
    USBD_ReturnType retval = USBD_E_ERROR;

    /* Note: NCM uses 2 interfaces */
    if (dev->IfCount < (USBD_MAX_IF_COUNT - 1))
    {
        /* Binding interfaces */
        itf->Base.Device = dev;
        itf->Base.Class  = &ncm_cbks;
        itf->Base.AltCount = 2;
        itf->Base.AltSelector = 0;

        /* Configure static elements of notification messages */
        itf->Notify.SpeedChange.RequestType     = 0xA1;
        itf->Notify.SpeedChange.NotificationType= CDC_NOT_CONNECTION_SPEED_CHANGE;
        itf->Notify.SpeedChange.Index           = dev->IfCount;
        itf->Notify.SpeedChange.Length          = sizeof(itf->Notify.SpeedData);
        itf->Notify.Connection.RequestType      = 0xA1;
        itf->Notify.Connection.NotificationType = CDC_NOT_NETWORK_CONNECTION;
        itf->Notify.Connection.Index            = dev->IfCount;
        itf->Notify.Connection.Value            = 0;
        itf->Notify.Connection.Length           = 0;

        /* Set endpoint types and max packet sizes */
        {
            USBD_EpHandleType *ep;

            ep = USBD_EpAddr2Ref(dev, itf->Config.NotEpNum);
            ep->Type            = USB_EP_TYPE_INTERRUPT;
            ep->MaxPacketSize   = NCM_NOT_PACKET_SIZE;
            ep->IfNum           = dev->IfCount;

            ep = USBD_EpAddr2Ref(dev, itf->Config.InEpNum);
            ep->Type            = USB_EP_TYPE_BULK;
            ep->MaxPacketSize   = NCM_DATA_PACKET_SIZE;
            ep->IfNum           = dev->IfCount;

            ep = USBD_EpAddr2Ref(dev, itf->Config.OutEpNum);
            ep->Type            = USB_EP_TYPE_BULK;
            ep->MaxPacketSize   = NCM_DATA_PACKET_SIZE;
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
 * @brief Notifies the host of connected medium and its speed,
 *        and sets up the data interface for NTB transfers.
 * @param itf: reference of the NCM interface
 * @param bitrate: bitrate of the connected medium in bits per second
 * @return INVALID if the interface isn't ready or already connected,
 *         BUSY if the notification endpoint is processing a previous transfer,
 *         OK if successful
 */
USBD_ReturnType USBD_NCM_Connect(USBD_NCM_IfHandleType *itf, uint32_t bitrate)
{
    USBD_ReturnType retval = USBD_E_INVALID;

    if ((itf->Base.AltSelector == 1) && (itf->Notify.Connection.Value == 0))
    {
        USBD_HandleType *dev = itf->Base.Device;
        uint16_t mps;
#if (USBD_HS_SUPPORT == 1)
        if (dev->Speed == USB_SPEED_HIGH)
        {   mps = USB_EP_BULK_HS_MPS; }
        else
#endif
        {   mps = USB_EP_BULK_FS_MPS; }

        /* Open data interface EPs */
        USBD_EpOpen(dev, itf->Config.InEpNum , USB_EP_TYPE_BULK, mps);
        USBD_EpOpen(dev, itf->Config.OutEpNum, USB_EP_TYPE_BULK, mps);

        /* IN setup */
        ((USBD_NCM_TransferHeaderType*)itf->In.Data[0])->V16.Sequence = 0;
        ((USBD_NCM_TransferHeaderType*)itf->In.Data[1])->V16.Sequence = 1;
        itf->In.FillState = NTB_EMPTY;
        itf->In.SendState = NTB_READY;
        itf->In.Page    = 0;
        itf->In.DgCount = 0;
        itf->In.Index   = sizeof(((USBD_NCM_TransferHeaderType*)0)->V16);
        itf->In.RemSize = itf->In.MaxSize - itf->In.Index -
                          sizeof(((USBD_NCM_DatagramPointerTableType*)0)->V16);

        /* OUT setup and start reception */
        itf->Out.State[0] = itf->Out.State[1] = NTB_EMPTY;
        itf->Out.Page = 0;

        USBD_EpReceive(dev, itf->Config.OutEpNum,
                itf->Out.Data[1], sizeof(itf->Out.Data[0]));

        /* Send notification */
        itf->Notify.SpeedData.DLBitRate = bitrate;
        itf->Notify.SpeedData.ULBitRate = bitrate;
        itf->Notify.Connection.Value = 1;
        retval = USBD_EpSend(dev, itf->Config.NotEpNum,
                &itf->Notify, sizeof(itf->Notify));
    }

    return retval;
}

/**
 * @brief Notifies the host of medium disconnection and closes data endpoints.
 * @param itf: reference of the NCM interface
 * @return INVALID if the interface isn't ready or already disconnected,
 *         BUSY if the notification endpoint is processing a previous transfer,
 *         OK if successful
 */
USBD_ReturnType USBD_NCM_Disconnect(USBD_NCM_IfHandleType *itf)
{
    USBD_ReturnType retval = USBD_E_INVALID;

    if ((itf->Base.AltSelector == 1) && (itf->Notify.Connection.Value != 0))
    {
        USBD_HandleType *dev = itf->Base.Device;
        itf->Notify.Connection.Value = 0;
        itf->In.FillState = NTB_EMPTY;

        USBD_EpClose(dev, itf->Config.InEpNum);
        USBD_EpClose(dev, itf->Config.OutEpNum);

        retval = USBD_EpSend(dev, itf->Config.NotEpNum,
                &itf->Notify.Connection, sizeof(itf->Notify.Connection));
    }

    return retval;
}

/**
 * @brief Reads the next datagram from the received queue and initiates a NTB transfer.
 *        if one of the buffer pages is empty.
 * @param itf: reference of the NCM interface
 * @param length: the length of the datagram is assigned
 * @return The datagram's start addresss
 */
uint8_t* USBD_NCM_GetDatagram(USBD_NCM_IfHandleType *itf, uint16_t *length)
{
    USBD_NCM_TransferHeaderType* nth;
    USBD_NCM_DatagramPointerTableType* pt = itf->Out.PT;
    uint8_t* data = NULL, page = itf->Out.Page;
    *length = 0;

    /* If fresh NTB, just mark it */
    if (itf->Out.State[page] == NTB_READY)
    {
        itf->Out.State[page] = NTB_PROCESSING;
    }
    else
    {
        /* A datagram has already been popped from this NTB */
        if (itf->Out.State[page] == NTB_PROCESSING)
        {
            /* First advance to the next datagram */
            if (pt->V16.Datagram[itf->Out.Dx + 1].Index != 0)
            {
                itf->Out.Dx++;
            }
            else if (pt->V16.NextNdpIndex != 0)
            {
                nth = (void*)itf->Out.Data[page];
                pt = (void*)nth + pt->V16.NextNdpIndex;
                itf->Out.PT = pt;
                itf->Out.Dx = 0;
            }
            else
            {
                /* NTB is fully processed */
                itf->Out.State[page] = NTB_EMPTY;
            }
        }

        /* Switch to other NTB if available */
        if ((itf->Out.State[page] == NTB_EMPTY) &&
            (itf->Out.State[1 - page] == NTB_READY))
        {
            itf->Out.Page = 1 - page;
            itf->Out.State[itf->Out.Page] = NTB_PROCESSING;
            nth = (void*)itf->Out.Data[itf->Out.Page];
            pt = (void*)nth + nth->V16.NdpIndex;
            itf->Out.PT = pt;
            itf->Out.Dx = 0;

            /* Receive to empty NTB */
            if (itf->Notify.Connection.Value != 0)
            {
                USBD_EpReceive(itf->Base.Device, itf->Config.OutEpNum,
                        itf->Out.Data[page], sizeof(itf->Out.Data[0]));

                itf->Out.State[page] = NTB_TRANSFERRING;
            }
        }
    }

    /* Set the return parameters if there's datagram available */
    if (itf->Out.State[itf->Out.Page] == NTB_PROCESSING)
    {
        data = (uint8_t*)itf->Out.Data[itf->Out.Page]
                + pt->V16.Datagram[itf->Out.Dx].Index;
        *length = pt->V16.Datagram[itf->Out.Dx].Length;
    }

    return data;
}

/**
 * @brief Attempts to allocate a datagram in the transmit NTB buffer.
 *        When the buffer is filled, the @ref USBD_NCM_SetDatagram function
 *        must be called to complete the placement of the datagram.
 * @param itf: reference of the NCM interface
 * @param length: total size of the datagram
 * @return pointer to word-aligned buffer if available, NULL otherwise
 */
uint8_t* USBD_NCM_AllocDatagram(USBD_NCM_IfHandleType *itf, uint16_t length)
{
    uint8_t* data = NULL;

    if ((itf->Notify.Connection.Value != 0) &&
        (length <= NCM_MAX_SEGMENT_SIZE) &&
        (itf->In.FillState != NTB_PROCESSING))
    {
        /* Datagrams are inserted starting from the header
         * Datagram lengths are saved backwards starting from the end of the buffer
         * and get ordered only at the time of NTB transmission */
        uint16_t wlen = (length + 3) & (~3);
        uint16_t addlen = wlen + sizeof(USBD_NCM_Datagram16);
        uint8_t page;

        /* Set syncronization state */
        itf->In.FillState = NTB_PROCESSING;

        page = itf->In.Page;

        /* If the current page has enough free space */
        if (addlen <= itf->In.RemSize)
        {
            USBD_NCM_TransferHeaderType* nth = (void*)itf->In.Data[page];
            struct {
                uint16_t length;
                uint16_t reserved;
            }*dp = (void*)nth + sizeof(itf->In.Data[0])
                    - (sizeof(*dp) * (++itf->In.DgCount));

            data = (void*)nth + itf->In.Index;

            dp->length = length;
            itf->In.Index += wlen;
            itf->In.RemSize -= addlen;
        }
    }

    return data;
}

/**
 * @brief Called after @ref USBD_NCM_AllocDatagram and after the datagram is
 *        copied to the buffer. Completes the operation and attempts an NTB IN transfer.
 * @param itf: reference of the NCM interface
 * @return OK if called in the right sequence, INVALID otherwise
 */
USBD_ReturnType USBD_NCM_SetDatagram(USBD_NCM_IfHandleType *itf)
{
    USBD_ReturnType retval = USBD_E_INVALID;

    if (itf->In.FillState == NTB_PROCESSING)
    {
        uint8_t page = itf->In.Page;

        /* Clear syncronization state */
        if (itf->In.SendState != NTB_EMPTY)
        {
            itf->In.FillState = NTB_READY;
        }
        else /* If the other page is empty, we can transmit this one */
        {
            ncm_sendNTB(itf, page);
        }

        retval = USBD_E_OK;
    }
    return retval;
}

/**
 * @brief Writes a datagram to the current block and initiates a NTB transfer
 *        if the current IN pipe is idle.
 * @param itf: reference of the NCM interface
 * @param data: pointer to the data to send
 * @param length: length of the data
 * @return INVALID if the datagram size is higher than the limit or if disconnected,
 *         OK if successful
 */
USBD_ReturnType USBD_NCM_PutDatagram(USBD_NCM_IfHandleType *itf, uint8_t *data, uint16_t length)
{
    USBD_ReturnType retval = USBD_E_INVALID;
    uint8_t* dst = USBD_NCM_AllocDatagram(itf, length);

    if (dst != NULL)
    {
        memcpy(dst, data, length);

        retval = USBD_NCM_SetDatagram(itf);
    }

    return retval;
}

/** @} */
