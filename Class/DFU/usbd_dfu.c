/**
  ******************************************************************************
  * @file    usbd_dfu.c
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-03-11
  * @brief   USB Device Firmware Upgrade Class implementation
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
#include <usbd_dfu.h>

#include <string.h>

#if (USBD_DFU_ALTSETTINGS != 0)
#define DFU_APP(ITF)    ((USBD_DFU_AppType*)(&(ITF)->App[(ITF)->Base.AltSelector]))
#else
#define DFU_APP(ITF)    ((USBD_DFU_AppType*)((ITF)->App))
#endif

#define DFU_CLASS_REQ_COUNT (sizeof(dfu_validStates)/sizeof(dfu_validStates[0]))

#define DFU_ATTR_WILL_DETACH            0x08
#define DFU_ATTR_MANIFESTATION_TOLERANT 0x04
#define DFU_ATTR_CAN_UPLOAD             0x02
#define DFU_ATTR_CAN_DNLOAD             0x01

#ifndef USBD_DFU_ST_EXTENSION
#define USBD_DFU_ST_EXTENSION           0
#endif
/* DFU STMicroelectronics Extension (DFUSE) commands */
#define DFUSE_CMD_GETCOMMANDS           0x00
#define DFUSE_CMD_SETADDRESSPOINTER     0x21
#define DFUSE_CMD_ERASE                 0x41
#define DFUSE_CMD_READ_UNPROTECT        0x92

#define DFUSE_GETADDRESS(ITF, DESC)     \
    ((uint8_t*)((ITF)->Address + (((ITF)->BlockNum - 2) * (DESC)->DFUFD.wTransferSize)))

typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bmAttributes; /*
                            [bitCanDnload]
                            [bitCanUpload]
                            [bitManifestationTolerant] means no USB reset is needed
                            [bitWillDetach] means Disconnect-Connect for DFU_DETACH
                            4b reserved */
    uint16_t wDetachTimeOut;
    uint16_t wTransferSize;
    uint16_t bcdDFUVersion;
}__packed USBD_DFU_FuncDescType;


typedef struct
{
    /* DFU Interface Descriptor */
    USB_InterfaceDescType DFU;
    /* DFU Functional Descriptor */
    USBD_DFU_FuncDescType DFUFD;
}__packed USBD_DFU_DescType;


static const USBD_DFU_DescType dfu_desc = {
    .DFU = { /* DFU Interface Descriptor */
        .bLength            = sizeof(dfu_desc.DFU),
        .bDescriptorType    = USB_DESC_TYPE_INTERFACE,
        .bInterfaceNumber   = 0,
        .bAlternateSetting  = 0,
        .bNumEndpoints      = 0,
        .bInterfaceClass    = 0xFE, /* bInterfaceClass: Application Specific Class Code */
        .bInterfaceSubClass = 0x01, /* bInterfaceSubClass: Device Firmware Upgrade Code */
        .bInterfaceProtocol = 0x02, /* bInterfaceProtocol: 1=runtime, 2=boot mode */
        .iInterface         = USBD_ISTR_INTERFACES,
    },
    .DFUFD = { /* DFU Functional Descriptor */
        .bLength            = sizeof(dfu_desc.DFUFD),
        .bDescriptorType    = DFU_DESC_TYPE_FUNCTIONAL,
        .bmAttributes       =
#if (USBD_DFU_MANIFEST_TOLERANT != 0)
                              DFU_ATTR_MANIFESTATION_TOLERANT |
#endif
                              DFU_ATTR_WILL_DETACH,
        .wDetachTimeOut     = 100,  /* Wait time [ms] between DFU_DETACH and USB reset */
        .wTransferSize      = USBD_EP0_BUFFER_SIZE,
#if (USBD_DFU_ST_EXTENSION != 0)
        .bcdDFUVersion      = 0x011A,
#else
        .bcdDFUVersion      = 0x0101,
#endif
    },
};

#if (USBD_DFU_ST_EXTENSION != 0)
__alignment(USBD_DATA_ALIGNMENT)
/* List of supported DFU SE commands */
static const uint8_t dfuse_cmds[] __align(USBD_DATA_ALIGNMENT) = {
    DFUSE_CMD_GETCOMMANDS,
    DFUSE_CMD_SETADDRESSPOINTER,
    DFUSE_CMD_ERASE,
};
#endif

/** @brief Bitmask stores which request (key) is valid in which DFU states (value) */
static const uint16_t dfu_validStates[] = {
        /* DFU_REQ_DETACH */
        (1 << DFU_STATE_APP_IDLE),
        /* DFU_REQ_DNLOAD */
        (1 << DFU_STATE_IDLE) | (1 << DFU_STATE_DNLOAD_IDLE),
        /* DFU_REQ_UPLOAD */
        (1 << DFU_STATE_IDLE) | (1 << DFU_STATE_UPLOAD_IDLE),
        /* DFU_REQ_GETSTATUS */
        (1 << DFU_STATE_APP_IDLE)    | (1 << DFU_STATE_APP_DETACH) |
        (1 << DFU_STATE_IDLE)        | (1 << DFU_STATE_DNLOAD_SYNC) |
        (1 << DFU_STATE_DNLOAD_IDLE) | (1 << DFU_STATE_MANIFEST_SYNC) |
        (1 << DFU_STATE_UPLOAD_IDLE) | (1 << DFU_STATE_ERROR),
        /* DFU_REQ_CLRSTATUS */
        (1 << DFU_STATE_ERROR),
        /* DFU_REQ_GETSTATE */
        (1 << DFU_STATE_APP_IDLE)    | (1 << DFU_STATE_APP_DETACH) |
        (1 << DFU_STATE_IDLE)        | (1 << DFU_STATE_DNLOAD_SYNC) |
        (1 << DFU_STATE_DNLOAD_IDLE) | (1 << DFU_STATE_MANIFEST_SYNC) |
        (1 << DFU_STATE_UPLOAD_IDLE) | (1 << DFU_STATE_ERROR),
        /* DFU_REQ_ABORT */
        (1 << DFU_STATE_IDLE)        | (1 << DFU_STATE_DNLOAD_SYNC) |
        (1 << DFU_STATE_DNLOAD_IDLE) | (1 << DFU_STATE_MANIFEST_SYNC) |
        (1 << DFU_STATE_UPLOAD_IDLE),
};

static USBD_ReturnType dfu_detach       (USBD_DFU_IfHandleType *itf);
static USBD_ReturnType dfu_download     (USBD_DFU_IfHandleType *itf);
static USBD_ReturnType dfu_upload       (USBD_DFU_IfHandleType *itf);
static USBD_ReturnType dfu_getStatus    (USBD_DFU_IfHandleType *itf);
static USBD_ReturnType dfu_clearStatus  (USBD_DFU_IfHandleType *itf);
static USBD_ReturnType dfu_getState     (USBD_DFU_IfHandleType *itf);
static USBD_ReturnType dfu_abort        (USBD_DFU_IfHandleType *itf);

#if (USBD_DFU_ALTSETTINGS != 0)
static uint16_t         dfu_getAltsDesc (USBD_DFU_IfHandleType *itf, uint8_t ifNum, uint8_t * dest);
#else
static uint16_t         dfu_getDesc     (USBD_DFU_IfHandleType *itf, uint8_t ifNum, uint8_t * dest);
#endif
static const char *     dfu_getString   (USBD_DFU_IfHandleType *itf, uint8_t intNum);
static void             dfu_init        (USBD_DFU_IfHandleType *itf);
static void             dfu_deinit      (USBD_DFU_IfHandleType *itf);
static USBD_ReturnType  dfu_setupStage  (USBD_DFU_IfHandleType *itf);
static void             dfu_dataStage   (USBD_DFU_IfHandleType *itf);


/* DFU interface class callbacks structure */
static const USBD_ClassType dfu_cbks = {
#if (USBD_DFU_ALTSETTINGS != 0)
    .GetDescriptor  = (USBD_IfDescCbkType)  dfu_getAltsDesc,
#else
    .GetDescriptor  = (USBD_IfDescCbkType)  dfu_getDesc,
#endif
    .GetString      = (USBD_IfStrCbkType)   dfu_getString,
    .Init           = (USBD_IfCbkType)      dfu_init,
    .Deinit         = (USBD_IfCbkType)      dfu_deinit,
    .SetupStage     = (USBD_IfSetupCbkType) dfu_setupStage,
    .DataStage      = (USBD_IfCbkType)      dfu_dataStage,
};

static USBD_ReturnType (*const dfu_reqFns[])(USBD_DFU_IfHandleType *itf) = {
    dfu_detach,
    dfu_download,
    dfu_upload,
    dfu_getStatus,
    dfu_clearStatus,
    dfu_getState,
    dfu_abort,
};

static uint16_t         rodfu_getDesc   (USBD_DFU_IfHandleType *itf, uint8_t ifNum, uint8_t * dest);
static USBD_ReturnType  rodfu_setupStage(USBD_DFU_IfHandleType *itf);

/* Reboot-only DFU interface class callbacks structure */
static const USBD_ClassType rodfu_cbks = {
    .GetDescriptor  = (USBD_IfDescCbkType)  rodfu_getDesc,
    .GetString      = (USBD_IfStrCbkType)   dfu_getString,
    .SetupStage     = (USBD_IfSetupCbkType) rodfu_setupStage,
};

/** @ingroup USBD_DFU
 * @defgroup USBD_DFU_Private_Functions DFU Private Functions
 * @{ */

/**
 * @brief Copies the interface descriptor to the destination buffer.
 * @param itf: reference of the DFU interface
 * @param ifNum: the index of the current interface in the device
 * @param dest: the destination buffer
 * @return Length of the copied descriptor
 */
static uint16_t rodfu_getDesc(USBD_DFU_IfHandleType *itf, uint8_t ifNum, uint8_t * dest)
{
    USBD_DFU_DescType *desc = (USBD_DFU_DescType*)dest;
    uint16_t len = sizeof(dfu_desc);

    memcpy(dest, &dfu_desc, sizeof(dfu_desc));
    desc->DFUFD.wDetachTimeOut = itf->Config.DetachTimeout_ms;

    /* Adjustment of interface indexes */
    desc->DFU.bInterfaceNumber = ifNum;

    desc->DFU.iInterface = USBD_IIF_INDEX(ifNum, 0);
    return len;
}

#if (USBD_DFU_ALTSETTINGS != 0)
/**
 * @brief Copies the interface descriptor to the destination buffer.
 * @param itf: reference of the DFU interface
 * @param ifNum: the index of the current interface in the device
 * @param dest: the destination buffer
 * @return Length of the copied descriptor
 */
static uint16_t dfu_getAltsDesc(USBD_DFU_IfHandleType *itf, uint8_t ifNum, uint8_t * dest)
{
    uint8_t as;
    uint16_t len = 0;
    USBD_DFU_FuncDescType *desc = (USBD_DFU_FuncDescType*)(dest +
            (itf->Base.AltCount * sizeof(dfu_desc.DFU)));

    /* Copy the DFU func desc after the interface descriptors */
    memcpy(desc, &dfu_desc.DFUFD, sizeof(dfu_desc.DFUFD));
    desc->wDetachTimeOut = itf->Config.DetachTimeout_ms;

    /* Copy the descriptors many times, add alternate setting indexes */
    for (as = 0; as < itf->Base.AltCount; as++)
    {
        USB_InterfaceDescType *ifdesc = (USB_InterfaceDescType*)&dest[len];
        memcpy(ifdesc, &dfu_desc.DFU, sizeof(dfu_desc.DFU));
        len += sizeof(dfu_desc.DFU);

        /* Set attributes */
        if ((itf->App[as].Erase != NULL) && (itf->App[as].Write != NULL))
        {
            desc->bmAttributes |= DFU_ATTR_CAN_DNLOAD;
        }
        if (itf->App[as].Read != NULL)
        {
            desc->bmAttributes |= DFU_ATTR_CAN_UPLOAD;
        }

        /* Adjustment of interface indexes */
        ifdesc->bInterfaceNumber = ifNum;
        ifdesc->bAlternateSetting = as;

        ifdesc->iInterface = USBD_IIF_INDEX(ifNum, as);
    }
    len += sizeof(dfu_desc.DFUFD);

    return len;
}
#else
/**
 * @brief Copies the interface descriptor to the destination buffer.
 * @param itf: reference of the DFU interface
 * @param ifNum: the index of the current interface in the device
 * @param dest: the destination buffer
 * @return Length of the copied descriptor
 */
static uint16_t dfu_getDesc(USBD_DFU_IfHandleType *itf, uint8_t ifNum, uint8_t * dest)
{
    USBD_DFU_DescType *desc = (USBD_DFU_DescType*)dest;
    uint16_t len = rodfu_getDesc(itf, ifNum, dest);

    /* Set attributes */
    if ((DFU_APP(itf)->Erase != NULL) && (DFU_APP(itf)->Write != NULL))
    {
        desc->DFUFD.bmAttributes |= DFU_ATTR_CAN_DNLOAD;
    }
    if (DFU_APP(itf)->Read != NULL)
    {
        desc->DFUFD.bmAttributes |= DFU_ATTR_CAN_UPLOAD;
    }

    return len;
}
#endif /* (USBD_DFU_ALTSETTINGS != 0) */

/**
 * @brief Returns the selected interface string.
 * @param itf: reference of the DFU interface
 * @param intNum: interface-internal string index
 * @return The referenced string
 */
static const char* dfu_getString(USBD_DFU_IfHandleType *itf, uint8_t intNum)
{
#if (USBD_DFU_ALTSETTINGS != 0)
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
 * @brief Initializes the interface by resetting the internal variables
 *        and initializing the attached application.
 * @param itf: reference of the DFU interface
 */
static void dfu_init(USBD_DFU_IfHandleType *itf)
{
    /* Internal variables initialization */
    itf->DevStatus.__reserved  = 0;
    itf->DevStatus.iString     = 0;
    itf->DevStatus.PollTimeout = 0;
    itf->Tag[0] = itf->Tag[1] = 0;

    /* DFU mode only */
    if (itf->DevStatus.State >= DFU_STATE_IDLE)
    {
        itf->Address = (uint8_t*)DFU_APP(itf)->Firmware.Address;
        itf->BlockNum = 0;
        itf->BlockLength = 0;

        /* Initialize media */
        USBD_SAFE_CALLBACK(DFU_APP(itf)->Init, );
    }
}

/**
 * @brief Removes other USB interfaces from the device if in APP_DETACH state.
 *        Otherwise deinitializes the interface's attached application.
 * @param itf: reference of the DFU interface
 */
static void dfu_deinit(USBD_DFU_IfHandleType *itf)
{
    /* In DFU mode */
    if (itf->DevStatus.State >= DFU_STATE_IDLE)
    {
        /* Deinitialize media */
        USBD_SAFE_CALLBACK(DFU_APP(itf)->Deinit, );
    }
}

/**
 * @brief Performs the interface-specific setup request handling.
 * @param itf: reference of the DFU interface
 * @return OK if the setup request is accepted, INVALID otherwise
 */
static USBD_ReturnType rodfu_setupStage(USBD_DFU_IfHandleType *itf)
{
    USBD_ReturnType retval = USBD_E_INVALID;
    USBD_HandleType *dev = itf->Base.Device;

    switch (dev->Setup.RequestType.Type)
    {
        case USB_REQ_TYPE_STANDARD:
        {
            /* DFU specific descriptors can be requested */
            if (dev->Setup.Request == USB_REQ_GET_DESCRIPTOR)
            {
                switch (dev->Setup.Value >> 8)
                {
                    /* Return DFU func. descriptor */
                    case DFU_DESC_TYPE_FUNCTIONAL:
                    {
#if USBD_DATA_ALIGNMENT > 1
                        void* data = dev->CtrlData;
                        memcpy(dev->CtrlData, &dfu_desc.DFUFD, sizeof(dfu_desc.DFUFD));
#else
                        void* data = &dfu_desc.DFUFD;
#endif
                        retval = USBD_CtrlSendData(dev, data, sizeof(dfu_desc.DFUFD));
                        break;
                    }
                    default:
                        break;
                }
            }
            break;
        }

        case USB_REQ_TYPE_CLASS:
        {
            switch (dev->Setup.Request)
            {
                case DFU_REQ_DETACH:
                    retval = dfu_detach(itf);
                    break;
                case DFU_REQ_GETSTATUS:
                    return USBD_CtrlSendData(dev,
                            &itf->DevStatus, sizeof(itf->DevStatus));
                    break;
                case DFU_REQ_GETSTATE:
                    retval = dfu_getState(itf);
                    break;
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
 * @brief Performs the interface-specific setup request handling.
 * @param itf: reference of the DFU interface
 * @return OK if the setup request is accepted, INVALID otherwise
 */
static USBD_ReturnType dfu_setupStage(USBD_DFU_IfHandleType *itf)
{
    USBD_ReturnType retval = USBD_E_INVALID;
    USBD_HandleType *dev = itf->Base.Device;

    switch (dev->Setup.RequestType.Type)
    {
        case USB_REQ_TYPE_STANDARD:
        {
            /* DFU specific descriptors can be requested */
            if (dev->Setup.Request == USB_REQ_GET_DESCRIPTOR)
            {
                switch (dev->Setup.Value >> 8)
                {
                    /* Return DFU func. descriptor */
                    case DFU_DESC_TYPE_FUNCTIONAL:
                    {
                        uint16_t len = dfu_cbks.GetDescriptor((void*)itf, 0, dev->CtrlData);
                        memcpy(dev->CtrlData, &dev->CtrlData[len - sizeof(dfu_desc.DFUFD)],
                                sizeof(dfu_desc.DFUFD));
                        retval = USBD_CtrlSendData(dev, dev->CtrlData,
                                sizeof(dfu_desc.DFUFD));
                        break;
                    }
                    default:
                        break;
                }
            }
            break;
        }

        case USB_REQ_TYPE_CLASS:
        {
            uint8_t reqId = dev->Setup.Request;
            uint16_t stateMask = 1 << itf->DevStatus.State;

            /* Call the operation indexed by bRequest
             * only if it can be executed in the current state */
            if ((reqId < DFU_CLASS_REQ_COUNT) &&
                ((dfu_validStates[reqId] & stateMask) != 0) &&
                (USBD_E_OK == dfu_reqFns[reqId](itf)))
            {
                retval = USBD_E_OK;
            }
            /* Don't leave APP mode */
            else if (itf->DevStatus.State < DFU_STATE_IDLE)
            {
                itf->DevStatus.State = DFU_STATE_APP_IDLE;
            }
            else /* Enter error state on unexpected request */
            {
                itf->DevStatus.State = DFU_STATE_ERROR;
                itf->DevStatus.Status = DFU_ERROR_STALLEDPKT;
            }
            break;
        }

        default:
            break;
    }
    return retval;
}

/**
 * @brief Handles the DFU DETACH request, which is followed by
 *        USB reset and device reconfiguration to DFU mode.
 *        When configured, performs attach-detach sequence.
 * @param itf: reference of the DFU interface
 * @return OK
 */
static USBD_ReturnType dfu_detach(USBD_DFU_IfHandleType *itf)
{
    itf->DevStatus.State = DFU_STATE_APP_DETACH;

    /* Check the detach capability in the DFU functional descriptor */
    if ((dfu_desc.DFUFD.bmAttributes & DFU_ATTR_WILL_DETACH) != 0)
    {
        /* Shutting down USB detaches it from the host,
         * attach will be done in DFU mode */
        USBD_Deinit(itf->Base.Device);

        /* Set tag to DFU mode */
        itf->Tag[0] =  DFU_MODE_TAG;
        itf->Tag[1] = ~DFU_MODE_TAG;

        /* Reset the system into DFU mode */
        USBD_SAFE_CALLBACK(itf->Config.Reboot, );
    }

    return USBD_E_OK;
}

/**
 * @brief Saves context data for block downloading.
 * @param itf: reference of the DFU interface
 * @return INVALID if the request is rejected, OK otherwise
 */
static USBD_ReturnType dfu_download(USBD_DFU_IfHandleType *itf)
{
    USBD_ReturnType retval = USBD_E_INVALID;
    USBD_HandleType *dev = itf->Base.Device;

    if (dev->Setup.Length > 0)
    {
        /* Check for download support */
        if ((DFU_APP(itf)->Erase != NULL) && (DFU_APP(itf)->Write != NULL))
        {
#if (USBD_DFU_ST_EXTENSION == 0)
            if (itf->DevStatus.State == DFU_STATE_IDLE)
            {
                /* Initialize address at first block */
                itf->Address = (uint8_t*)DFU_APP(itf)->Firmware.Address;
                itf->BlockNum = 0xFFFF;
            }

            /* Checks for valid sequence and overall length */
            if (   ( dev->Setup.Value == ((itf->BlockNum + 1) & 0xFFFF))
                && (((uint32_t)itf->Address + dev->Setup.Length) <
                    (DFU_APP(itf)->Firmware.Address + DFU_APP(itf)->Firmware.TotalSize)))
#endif /* (USBD_DFU_ST_EXTENSION == 0) */
            {
                /* Update the global length and block number */
                itf->BlockNum    = dev->Setup.Value;
                itf->BlockLength = dev->Setup.Length;

                /* Update the state machine */
                itf->DevStatus.State = DFU_STATE_DNLOAD_SYNC;

                /* Prepare the reception of the buffer over EP0 */
                retval = USBD_CtrlReceiveData(dev, dev->CtrlData);
            }
        }
    }
    /* Deviation from spec: allow 0 length Dnload in IDLE state to
     * put device in manifestation mode ->
     * It is possible to return to application mode without effective update */
    else
    {
        itf->BlockLength = 1;
        itf->DevStatus.State = DFU_STATE_MANIFEST_SYNC;
        retval = USBD_E_OK;
    }
    return retval;
}

/**
 * @brief Reads and sends a firmware block to the host.
 * @param itf: reference of the DFU interface
 * @return INVALID if the request is rejected, OK otherwise
 */
static USBD_ReturnType dfu_upload(USBD_DFU_IfHandleType *itf)
{
    USBD_ReturnType retval = USBD_E_INVALID;
    USBD_HandleType *dev = itf->Base.Device;

    /* Send data to host if supported */
    if ((dev->Setup.Length > 0) && (DFU_APP(itf)->Read != NULL))
    {
        uint8_t *data = dev->CtrlData;
#if (USBD_DFU_ST_EXTENSION != 0)
        itf->BlockNum = dev->Setup.Value;

        /* Get commands */
        if (itf->BlockNum == 0)
        {
            itf->DevStatus.State = (dev->Setup.Length > sizeof(dfuse_cmds)) ?
                    DFU_STATE_IDLE : DFU_STATE_UPLOAD_IDLE;

            /* Return with supported commands */
            retval = USBD_CtrlSendData(dev, dfuse_cmds, sizeof(dfuse_cmds));
        }
        /* Read memory */
        else if (itf->BlockNum > 1)
        {
            itf->DevStatus.State = DFU_STATE_UPLOAD_IDLE;

            DFU_APP(itf)->Read(
                    DFUSE_GETADDRESS(itf, &dfu_desc),
                    data,
                    dev->Setup.Length);

            retval = USBD_CtrlSendData(dev, data, dev->Setup.Length);
        }
#else
        /* The host sends DFU_UPLOAD requests to the device until it
         * responds with a short frame as an end of file (EOF) indicator.
         * The device is responsible for selecting the address range
         * to upload and formatting the firmware image appropriately. */

        if (itf->DevStatus.State == DFU_STATE_IDLE)
        {
            /* Initialize address at first block */
            itf->Address = (uint8_t*)DFU_APP(itf)->Firmware.Address;
            itf->BlockNum = 0xFFFF;
        }

        /* Check for correct sequence */
        if (dev->Setup.Value == ((itf->BlockNum + 1) & 0xFFFF))
        {
            uint16_t len;
            uint32_t progress = (uint32_t)itf->Address - DFU_APP(itf)->Firmware.Address;

            /* Shorten the block size if it's the end of the firmware memory,
             * return to IDLE */
            if ((progress + dev->Setup.Length) > DFU_APP(itf)->Firmware.TotalSize)
            {
                len = DFU_APP(itf)->Firmware.TotalSize - progress;
                itf->DevStatus.State = DFU_STATE_IDLE;
            }
            else
            {
                len = dev->Setup.Length;
                itf->DevStatus.State = DFU_STATE_UPLOAD_IDLE;
            }

            DFU_APP(itf)->Read(itf->Address, data, len);

            /* Increment address for next block upload */
            itf->Address  += len;
            itf->BlockNum  = dev->Setup.Value;

            retval = USBD_CtrlSendData(dev, data, len);
        }
#endif /* (USBD_DFU_ST_EXTENSION != 0) */
    }
    else /* No data stage */
    {
        itf->DevStatus.State = DFU_STATE_IDLE;
        retval = USBD_E_OK;
    }
    return retval;
}

/**
 * @brief Updates and sends the DFU Status through the control pipe.
 * @param itf: reference of the DFU interface
 * @return OK
 */
static USBD_ReturnType dfu_getStatus(USBD_DFU_IfHandleType *itf)
{
    USBD_HandleType *dev = itf->Base.Device;
    USBD_DFU_StateType nextState = itf->DevStatus.State;

    /* Provide timeout values before starting download / manifestation */
    if ((itf->DevStatus.State == DFU_STATE_DNLOAD_SYNC) ||
        (itf->DevStatus.State == DFU_STATE_MANIFEST_SYNC))
    {
        if (itf->BlockLength > 0)
        {
            if (DFU_APP(itf)->GetTimeout_ms != NULL)
            {
                /* Read the poll timeout */
                itf->DevStatus.PollTimeout = DFU_APP(itf)->GetTimeout_ms(
                        itf->Address,
                        itf->BlockLength);
            }

            /* DNLOAD_SYNC   -> DNLOAD_BUSY
             * MANIFEST_SYNC -> MANIFEST */
            nextState += 1;
        }
        else if (itf->DevStatus.State == DFU_STATE_DNLOAD_SYNC)
        {
            /* Download has been completed */
            itf->DevStatus.State = nextState = DFU_STATE_DNLOAD_IDLE;
        }
        else
        {
            /* Manifestation has been completed */
            itf->DevStatus.State = nextState = DFU_STATE_IDLE;
        }
    }

    /* Send the status data over EP0 */
    USBD_CtrlSendData(dev, &itf->DevStatus, sizeof(itf->DevStatus));

    itf->DevStatus.State = nextState;
    return USBD_E_OK;
}

/**
 * @brief Clears the error status from the DFU interface.
 * @param itf: reference of the DFU interface
 * @return OK
 */
static USBD_ReturnType dfu_clearStatus(USBD_DFU_IfHandleType *itf)
{
    itf->DevStatus.State = DFU_STATE_IDLE;
    itf->DevStatus.Status = DFU_ERROR_NONE;
    itf->DevStatus.PollTimeout = 0;
    return USBD_E_OK;
}

/**
 * @brief Sends the DFU state through the control pipe.
 * @param itf: reference of the DFU interface
 * @return OK
 */
static USBD_ReturnType dfu_getState(USBD_DFU_IfHandleType *itf)
{
    return USBD_CtrlSendData(itf->Base.Device,
            &itf->DevStatus.State, sizeof(itf->DevStatus.State));
}

/**
 * @brief Returns the DFU to dfuIDLE state.
 * @param itf: reference of the DFU interface
 * @return OK
 */
static USBD_ReturnType dfu_abort(USBD_DFU_IfHandleType *itf)
{
    itf->DevStatus.State  = DFU_STATE_IDLE;
    itf->DevStatus.Status = DFU_ERROR_NONE;
    itf->DevStatus.PollTimeout = 0;
    itf->BlockNum    = 0;
    itf->BlockLength = 0;
    return USBD_E_OK;
}

/**
 * @brief Performs time-consuming memory operations after a successful GetStatus transfer.
 * @param itf: reference of the DFU interface
 */
static void dfu_dataStage(USBD_DFU_IfHandleType *itf)
{
    USBD_HandleType *dev = itf->Base.Device;

    /* Perform after GetStatus request */
    if ((dev->Setup.RequestType.Type == USB_REQ_TYPE_CLASS) &&
        (dev->Setup.Request == DFU_REQ_GETSTATUS))
    {
        switch (itf->DevStatus.State)
        {
            case DFU_STATE_DNLOAD_BUSY:
            {
                /* New state if no errors occur */
                itf->DevStatus.State = DFU_STATE_DNLOAD_SYNC;
#if (USBD_DFU_ST_EXTENSION != 0)
                /* Regular Download Command */
                if (itf->BlockNum > 1)
                {
                    itf->DevStatus.Status = DFU_APP(itf)->Write(
                            DFUSE_GETADDRESS(itf, &dfu_desc),
                            dev->CtrlData,
                            itf->BlockLength);
                }
                /* Execute special command */
                else if (itf->BlockNum == 0)
                {
                    uint8_t cmd = dev->CtrlData[0], *data = dev->CtrlData + 1;

                    switch (cmd)
                    {
                        /* Change the address pointer for more optimal flashing */
                        case DFUSE_CMD_SETADDRESSPOINTER:
                            if (itf->BlockLength == 5)
                            {
                                itf->Address = (uint8_t*)(*(__packed uint32_t *)data);
                            }
                            break;

                        /* Erase a flash block */
                        case DFUSE_CMD_ERASE:
                            if (itf->BlockLength == 5)
                            {
                                itf->Address = (uint8_t*)(*(__packed uint32_t *)data);

                                itf->DevStatus.Status = DFU_APP(itf)->Erase(
                                        itf->Address);
                            }
                            break;

                        case DFUSE_CMD_READ_UNPROTECT:
                        case DFUSE_CMD_GETCOMMANDS:
                            break;
                        /* Not supported */
                        default:
                            itf->DevStatus.Status = DFU_ERROR_STALLEDPKT;
                            break;
                    }
                }
#else
                /* Erase firmware on the first block */
                if (itf->Address == (uint8_t*)DFU_APP(itf)->Firmware.Address)
                {
                    itf->DevStatus.Status = DFU_APP(itf)->Erase(
                            itf->Address);
                }
                /* Write after erase */
                if (itf->DevStatus.Status == DFU_ERROR_NONE)
                {
                    itf->DevStatus.Status = DFU_APP(itf)->Write(
                            itf->Address,
                            dev->CtrlData,
                            itf->BlockLength);

                    itf->Address += itf->BlockLength;
                }
#endif /* (USBD_DFU_ST_EXTENSION != 0) */
                itf->BlockLength = 0;
                itf->DevStatus.PollTimeout = 0;
                break;
            }

            case DFU_STATE_MANIFEST:
            {
                /* Perform manifestation */
                if (DFU_APP(itf)->Manifest != NULL)
                {
                    itf->DevStatus.Status = DFU_APP(itf)->Manifest();
                }

                if (itf->DevStatus.Status == DFU_ERROR_NONE)
                {
#if (USBD_DFU_MANIFEST_TOLERANT != 0)
                    itf->DevStatus.State = DFU_STATE_MANIFEST_SYNC;
                    itf->BlockLength = 0;
                    itf->DevStatus.PollTimeout = 0;
#else
                    itf->DevStatus.State = DFU_STATE_MANIFEST_WAIT_RESET;

                    /* Disconnect the USB device */
                    USBD_Deinit(itf->Base.Device);

                    /* Generate system reset to allow jumping to the user code */
                    USBD_SAFE_CALLBACK(itf->Config.Reboot, );
#endif
                }
                break;
            }

            default:
                break;
        }
    }

    /* If any operation encountered error, enter error state */
    if (itf->DevStatus.Status != DFU_ERROR_NONE)
    {
        itf->DevStatus.State = DFU_STATE_ERROR;
    }
}

/** @} */

/** @defgroup USBD_DFU_Exported_Functions_Boot DFU Exported Functions for Bootloader
 * @{ */

/**
 * @brief Sets the necessary fields of the DFU interface.
 * @note  This function shall be called in the DFU bootloader even if it jumps
 *        to the application immediately.
 * @param itf: reference of the DFU interface
 * @param pReboot: reference of the system rebooting function
 * @param app: reference of the program memory handler
 * @param appCount: array size of app parameter (only used when USBD_DFU_ALTSETTINGS != 0)
 */
void USBD_DFU_BootInit(USBD_DFU_IfHandleType *itf,
                       USBD_DFU_RebootCbkType pReboot,
                       const USBD_DFU_AppType *app, uint8_t appCount)
{
    itf->Base.Class  = &dfu_cbks;
#if (USBD_DFU_ALTSETTINGS != 0)
    itf->Base.AltCount = appCount;
#else
    itf->Base.AltCount = 1;
#endif
    itf->Base.AltSelector = 0;
    itf->App = app;
    itf->Config.Reboot = pReboot;

    /* If DFU state is entered due to detach request, enter IDLE state
     * Otherwise assume application firmware is missing */
    if (USBD_DFU_IsRequested(itf))
    {
        itf->DevStatus.State  = DFU_STATE_IDLE;
        itf->DevStatus.Status = DFU_ERROR_NONE;
    }
    else
    {
        itf->DevStatus.State  = DFU_STATE_ERROR;
        itf->DevStatus.Status = DFU_ERROR_FIRMWARE;
    }
}

/**
 * @brief Determines if DFU operation has been requested before the reset.
 * @param itf: reference of the DFU interface
 * @return TRUE if DFU mode is requested, FALSE otherwise
 */
int USBD_DFU_IsRequested(USBD_DFU_IfHandleType *itf)
{
    return ((itf->Tag[0] == DFU_MODE_TAG) && (itf->Tag[1] == (~DFU_MODE_TAG)));
}

/** @} */

/** @defgroup USBD_DFU_Exported_Functions_App DFU Exported Functions for Application
 * @{ */

/**
 * @brief Sets the necessary fields of the DFU interface.
 * @note  This function shall be called in the main application code.
 * @param itf: reference of the DFU interface
 * @param detachTimeout_ms: The necessary amount of time to safely shut down
 *        the application before entering DFU mode
 */
void USBD_DFU_AppInit(USBD_DFU_IfHandleType *itf, uint16_t detachTimeout_ms)
{
    itf->Config.DetachTimeout_ms = detachTimeout_ms;

    /* Setting state in application */
    itf->DevStatus.State  = DFU_STATE_APP_IDLE;
    itf->DevStatus.Status = DFU_ERROR_NONE;
}

/**
 * @brief Mounts a reboot-only version of the DFU interface to the USB Device
 *        at the next interface slot. Used for devices which have DFU implementation
 *        in ROM.
 * @note  The interface reference shall have its Config structure
 *        properly set before this function is called.
 * @param itf: reference of the DFU interface
 * @param dev: reference of the USB Device
 * @return OK if the mounting was successful,
 *         ERROR if it failed due to insufficient device interface slots
 */
USBD_ReturnType USBD_DFU_MountRebootOnly(USBD_DFU_IfHandleType *itf, USBD_HandleType *dev)
{
    USBD_ReturnType retval = USBD_E_ERROR;

    if (dev->IfCount < USBD_MAX_IF_COUNT)
    {
        /* Binding interfaces */
        itf->Base.Device = dev;
        itf->Base.Class  = &rodfu_cbks;
        itf->Base.AltCount = 1;
        itf->Base.AltSelector = 0;

        /* Setting state in application */
        itf->DevStatus.State  = DFU_STATE_APP_IDLE;
        itf->DevStatus.Status = DFU_ERROR_NONE;
        itf->DevStatus.__reserved  = 0;
        itf->DevStatus.iString     = 0;
        itf->DevStatus.PollTimeout = 0;

        dev->IF[dev->IfCount] = (USBD_IfHandleType*)itf;
        dev->IfCount++;

        retval = USBD_E_OK;
    }

    return retval;
}

/** @} */

/** @defgroup USBD_DFU_Exported_Functions DFU Common Exported Functions
 * @{ */

/**
 * @brief Mounts the DFU interface to the USB Device at the next interface slot.
 * @note  The interface reference shall have its Config structure and App reference
 *        properly set before this function is called.
 * @param itf: reference of the DFU interface
 * @param dev: reference of the USB Device
 * @return OK if the mounting was successful,
 *         ERROR if it failed due to insufficient device interface slots
 */
USBD_ReturnType USBD_DFU_MountInterface(USBD_DFU_IfHandleType *itf, USBD_HandleType *dev)
{
    USBD_ReturnType retval = USBD_E_ERROR;

    if (dev->IfCount < USBD_MAX_IF_COUNT)
    {
        /* Binding interfaces */
        itf->Base.Device = dev;

        dev->IF[dev->IfCount] = (USBD_IfHandleType*)itf;
        dev->IfCount++;

        retval = USBD_E_OK;
    }

    return retval;
}

/** @} */
