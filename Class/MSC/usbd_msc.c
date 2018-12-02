/**
  ******************************************************************************
  * @file    usbd_msc.c
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-03-25
  * @brief   Universal Serial Bus Mass Storage Class
  *          Bulk-Only Transfer implementation
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
#include <private/usbd_msc_private.h>

#define MSC_SUBCLASS_SCSI_NREP      0x00
#define MSC_SUBCLASS_RBC            0x01
#define MSC_SUBCLASS_MMC5           0x02
#define MSC_SUBCLASS_UFI            0x04
#define MSC_SUBCLASS_SCSI_TRANSP    0x06
#define MSC_SUBCLASS_LSDFS          0x07
#define MSC_SUBCLASS_IEEE1667       0x08
#define MSC_SUBCLASS_VENDOR         0xFF

#define MSC_PROT_CBI_CCI            0x00
#define MSC_PROT_CBI                0x01
#define MSC_PROT_BBB                0x50
#define MSC_PROT_UAS                0x62
#define MSC_PROT_VENDOR             0xFF

#if (USBD_HS_SUPPORT == 1)
#define MSC_DATA_PACKET_SIZE        USB_EP_BULK_HS_MPS
#else
#define MSC_DATA_PACKET_SIZE        USB_EP_BULK_FS_MPS
#endif

typedef union {
    uint8_t b[4];
    uint32_t dw;
}USBD_MSC_SignatureType;

static const USBD_MSC_SignatureType cbw_sign =
{ .b = {'U', 'S', 'B', 'C'} };

static const USBD_MSC_SignatureType csw_sign =
{ .b = {'U', 'S', 'B', 'S'} };

static const USB_InterfaceDescType msc_desc = {
    /* MSC Interface Descriptor */
    .bLength            = sizeof(msc_desc),
    .bDescriptorType    = USB_DESC_TYPE_INTERFACE,
    .bInterfaceNumber   = 0,
    .bAlternateSetting  = 0,
    .bNumEndpoints      = 2,
    .bInterfaceClass    = 0x08,                     /* bInterfaceClass: Mass Storage Class */
    .bInterfaceSubClass = MSC_SUBCLASS_SCSI_TRANSP, /* bInterfaceSubClass: SCSI transparent */
    .bInterfaceProtocol = MSC_PROT_BBB,             /* bInterfaceProtocol: Bulk-Only (BBB) */
    .iInterface         = USBD_ISTR_INTERFACES,
};


static uint16_t         msc_getDesc     (USBD_MSC_IfHandleType *itf, uint8_t ifNum, uint8_t * dest);
static const char *     msc_getString   (USBD_MSC_IfHandleType *itf, uint8_t intNum);
static void             msc_init        (USBD_MSC_IfHandleType *itf);
static void             msc_deinit      (USBD_MSC_IfHandleType *itf);
static USBD_ReturnType  msc_setupStage  (USBD_MSC_IfHandleType *itf);
static void             msc_outData     (USBD_MSC_IfHandleType *itf, USBD_EpHandleType *ep);
static void             msc_inData      (USBD_MSC_IfHandleType *itf, USBD_EpHandleType *ep);

/* MSC interface class callbacks structure */
static const USBD_ClassType msc_cbks = {
    .GetDescriptor  = (USBD_IfDescCbkType)  msc_getDesc,
    .GetString      = (USBD_IfStrCbkType)   msc_getString,
    .Init           = (USBD_IfCbkType)      msc_init,
    .Deinit         = (USBD_IfCbkType)      msc_deinit,
    .SetupStage     = (USBD_IfSetupCbkType) msc_setupStage,
    .OutData        = (USBD_IfEpCbkType)    msc_outData,
    .InData         = (USBD_IfEpCbkType)    msc_inData,
};

/** @ingroup USBD_MSC
 * @defgroup USBD_MSC_Private_Functions MSC Private Functions
 * @{ */

/**
 * @brief Copies the interface descriptor to the destination buffer.
 * @param itf: reference of the MSC interface
 * @param ifNum: the index of the current interface in the device
 * @param dest: the destination buffer
 * @return Length of the copied descriptor
 */
static uint16_t msc_getDesc(USBD_MSC_IfHandleType *itf, uint8_t ifNum, uint8_t * dest)
{
    USB_InterfaceDescType *desc = (USB_InterfaceDescType*)dest;
    uint16_t len = sizeof(msc_desc);

    memcpy(dest, &msc_desc, sizeof(msc_desc));

#if (USBD_MAX_IF_COUNT > 1)
    /* Adjustment of interface indexes */
    desc->bInterfaceNumber = ifNum;

    desc->iInterface = USBD_IIF_INDEX(ifNum, 0);
#endif /* (USBD_MAX_IF_COUNT > 1) */

    len += USBD_EpDesc(itf->Base.Device, itf->Config.OutEpNum, &dest[len]);
    len += USBD_EpDesc(itf->Base.Device, itf->Config.InEpNum, &dest[len]);

#if (USBD_HS_SUPPORT == 1)
    if (itf->Base.Device->Speed == USB_SPEED_FULL)
    {
        USB_EndpointDescType* ed = &dest[sizeof(msc_desc)];
        ed[0].wMaxPacketSize = USB_EP_BULK_FS_MPS;
        ed[1].wMaxPacketSize = USB_EP_BULK_FS_MPS;
    }
#endif

    return len;
}

/**
 * @brief Returns the selected interface string.
 * @param itf: reference of the MSC interface
 * @param intNum: interface-internal string index
 * @return The referenced string
 */
static const char* msc_getString(USBD_MSC_IfHandleType *itf, uint8_t intNum)
{
    /* TODO Temporary solution to reduce number of strings */
    return MSC_GetLU(itf,0)->Inquiry->ProductId;
}

/**
 * @brief Initiate the reception of the CBW through the OUT pipe.
 * @param itf: reference of the MSC interface
 */
static void msc_receiveCBW(USBD_MSC_IfHandleType *itf)
{
    itf->State = MSC_STATE_COMMAND_OUT;

    USBD_EpReceive(itf->Base.Device, itf->Config.OutEpNum,
            &itf->CBW, sizeof(itf->CBW));
}

/**
 * @brief Send the Command Status Wrapper to the host.
 * @param itf: reference of the MSC interface
 */
static void msc_sendCSW(USBD_MSC_IfHandleType *itf)
{
    USBD_EpSend(itf->Base.Device, itf->Config.InEpNum,
            &itf->CSW, sizeof(itf->CSW));

    msc_receiveCBW(itf);
}

/**
 * @brief Initializes the interface by opening its endpoints.
 * @param itf: reference of the MSC interface
 */
static void msc_init(USBD_MSC_IfHandleType *itf)
{
    USBD_HandleType *dev = itf->Base.Device;
    uint16_t mps;
    uint8_t lun;

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

    /* Initialize BOT layer */
    itf->Status = MSC_STATUS_NORMAL;
    itf->CSW.dSignature = csw_sign.dw;

    msc_receiveCBW(itf);

    for (lun = 0; lun <= itf->Config.MaxLUN; lun++)
    {
        USBD_SAFE_CALLBACK(MSC_GetLU(itf, lun)->Init, lun);
    }

}

/**
 * @brief Deinitializes the interface by closing its endpoints.
 * @param itf: reference of the MSC interface
 */
static void msc_deinit(USBD_MSC_IfHandleType *itf)
{
    USBD_HandleType *dev = itf->Base.Device;
    uint8_t lun;

    /* Close EPs */
    USBD_EpClose(dev, itf->Config.InEpNum);
    USBD_EpClose(dev, itf->Config.OutEpNum);

#if (USBD_HS_SUPPORT == 1)
    /* Reset the endpoint MPS to the desired size */
    USBD_EpAddr2Ref(dev, itf->Config.InEpNum)->MaxPacketSize  = MSC_DATA_PACKET_SIZE;
    USBD_EpAddr2Ref(dev, itf->Config.OutEpNum)->MaxPacketSize = MSC_DATA_PACKET_SIZE;
#endif

    for (lun = 0; lun <= itf->Config.MaxLUN; lun++)
    {
        USBD_SAFE_CALLBACK(MSC_GetLU(itf, lun)->Deinit, lun);
    }
}

/**
 * @brief Performs the class type setup request handling.
 * @param itf: reference of the MSC interface
 * @return OK if the setup request is accepted, INVALID otherwise
 */
static USBD_ReturnType msc_setupStage(USBD_MSC_IfHandleType *itf)
{
    USBD_ReturnType retval = USBD_E_INVALID;
    USBD_HandleType *dev = itf->Base.Device;

    switch (dev->Setup.RequestType.Type)
    {
        case USB_REQ_TYPE_CLASS:
        {
            switch (dev->Setup.Request)
            {
                case MSC_BOT_GET_MAX_LUN:
                {
                    dev->CtrlData[0] = itf->Config.MaxLUN;
                    retval = USBD_CtrlSendData(dev, dev->CtrlData, 1);
                    break;
                }
                case MSC_BOT_RESET:
                {
                    itf->Status = MSC_STATUS_RECOVERY;
                    retval = USBD_E_OK;
                    break;
                }
                default:
                    break;
            }
        }
        default:
            break;
    }
    return retval;
}

/**
 * @brief Advances an ongoing read operation or sends a CSW for failure/completion.
 * @param itf: reference of the MSC interface
 * @param ep: reference to the IN endpoint structure
 */
static void msc_inData(USBD_MSC_IfHandleType *itf, USBD_EpHandleType *ep)
{
    USBD_HandleType *dev = itf->Base.Device;

    switch (itf->State)
    {
        /* Keep sending the read data */
        case MSC_STATE_DATA_IN:
        {
            /* Continue with READ10 command */
            SCSI_ProcessRead(itf);

            if (itf->CSW.bStatus != MSC_CSW_CMD_PASSED)
            {
                itf->State = MSC_STATE_STALL;
                USBD_EpSetStall(dev, itf->Config.InEpNum);
            }
            break;
        }

        /* Single or last transfer is complete, send CSW */
        case MSC_STATE_STATUS_IN:
        /* EP ClearStall */
        case MSC_STATE_STALL:
        {
            if (itf->Status == MSC_STATUS_NORMAL)
            {
                msc_sendCSW(itf);
            }
            break;
        }

        default: /* msc_sendCSW completed */
            break;
    }
}

/**
 * @brief This function manages the processing of bulk endpoint received data.
 * @param itf: reference of the MSC interface
 * @param ep: reference to the IN endpoint structure
 */
static void msc_outData(USBD_MSC_IfHandleType *itf, USBD_EpHandleType *ep)
{
    USBD_HandleType *dev = itf->Base.Device;

    switch (itf->State)
    {
        /* Command Transport */
        case MSC_STATE_COMMAND_OUT:
        {
            /* CSW initial setup */
            itf->CSW.dTag = itf->CBW.dTag;
            itf->CSW.dDataResidue = itf->CBW.dDataLength;
            itf->CSW.bStatus = MSC_CSW_CMD_PASSED;

            /* Check received CBW validity */
            if ((ep->Transfer.Length == sizeof(itf->CBW)) &&
                (itf->CBW.dSignature == cbw_sign.dw) &&
                (itf->CBW.bLUN <= itf->Config.MaxLUN) &&
                (itf->CBW.bCBLength > 0) &&
                (itf->CBW.bCBLength <= sizeof(itf->CBW.CB)))
            {
                SCSI_ProcessCommand(itf);

                /* Send command status */
                if (itf->CBW.dDataLength == 0)
                {
                    msc_sendCSW(itf);
                }
                /* Treat rejected command */
                else if (itf->CSW.bStatus != MSC_CSW_CMD_PASSED)
                {
                    itf->State = MSC_STATE_STALL;

                    /* Terminate transfer by STALLing the proper endpoint */
                    if (itf->CBW.bmFlags == 0)
                    {
                        USBD_EpSetStall(dev, itf->Config.OutEpNum);
                    }
                    else
                    {
                        USBD_EpSetStall(dev, itf->Config.InEpNum);
                    }
                }
            }
            else
            {
                /* CBW invalid, STALL both endpoints until recovery */
                SCSI_PutSenseCode(itf, SCSI_SKEY_ILLEGAL_REQUEST,
                        SCSI_ASC_INVALID_CDB);

                itf->State = MSC_STATE_STALL;
                itf->Status = MSC_STATUS_ERROR;

                USBD_EpSetStall(dev, itf->Config.OutEpNum);
                USBD_EpSetStall(dev, itf->Config.InEpNum);
            }
            break;
        }

        /* Data Transport */
        case MSC_STATE_DATA_OUT:
        {
            /* Continue with WRITE10 command */
            SCSI_ProcessWrite(itf);

            if (itf->CSW.bStatus != MSC_CSW_CMD_PASSED)
            {
                itf->State = MSC_STATE_STALL;
                USBD_EpSetStall(dev, itf->Config.OutEpNum);
            }
            /* Write completed, send status */
            else if (itf->CSW.dDataResidue == 0)
            {
                msc_sendCSW(itf);
            }
            break;
        }

        /* EP ClearStall */
        default:
        {
            if (itf->Status == MSC_STATUS_NORMAL)
            {
                msc_sendCSW(itf);
            }
            else if (itf->Status == MSC_STATUS_RECOVERY)
            {
                msc_receiveCBW(itf);
                itf->Status = MSC_STATUS_NORMAL;
            }
            break;
        }
    }
}

/**
 * @brief Returns the selected logical unit's handler.
 * @param itf: reference of the MSC interface
 * @param lun: number of the logical unit to use
 * @return The logical unit's handler reference
 */
const USBD_MSC_LUType* MSC_GetLU(USBD_MSC_IfHandleType *itf, uint8_t lun)
{
    return &itf->LUs[lun];
}

/** @} */

/** @defgroup USBD_MSC_Exported_Functions MSC Exported Functions
 * @{ */

/**
 * @brief Mounts the MSC interface to the USB Device at the next two interface slots.
 * @note  The interface reference shall have its @ref USBD_MSC_IfHandleType::Config structure
 *        and @ref USBD_MSC_IfHandleType::LUs reference properly set before this function is called.
 * @param itf: reference of the MSC interface
 * @param dev: reference of the USB Device
 * @return OK if the mounting was successful,
 *         ERROR if it failed due to insufficient device interface slots
 */
USBD_ReturnType USBD_MSC_MountInterface(USBD_MSC_IfHandleType *itf, USBD_HandleType *dev)
{
    USBD_ReturnType retval = USBD_E_ERROR;

    if (dev->IfCount < USBD_MAX_IF_COUNT)
    {
        /* Binding interfaces */
        itf->Base.Device = dev;
        itf->Base.Class  = &msc_cbks;
        itf->Base.AltCount = 1;
        itf->Base.AltSelector = 0;

        {
            USBD_EpHandleType *ep;

            ep = USBD_EpAddr2Ref(dev, itf->Config.InEpNum);
            ep->Type            = USB_EP_TYPE_BULK;
            ep->MaxPacketSize   = MSC_DATA_PACKET_SIZE;
            ep->IfNum           = dev->IfCount;

            ep = USBD_EpAddr2Ref(dev, itf->Config.OutEpNum);
            ep->Type            = USB_EP_TYPE_BULK;
            ep->MaxPacketSize   = MSC_DATA_PACKET_SIZE;
            ep->IfNum           = dev->IfCount;
        }

        dev->IF[dev->IfCount] = (USBD_IfHandleType*)itf;
        dev->IfCount++;

        retval = USBD_E_OK;
    }

    return retval;
}

/** @} */
