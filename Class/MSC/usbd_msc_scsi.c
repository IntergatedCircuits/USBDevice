/**
  ******************************************************************************
  * @file    usbd_msc_scsi.c
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-03-25
  * @brief   Universal Serial Bus Mass Storage Class
  *          SCSI Protocol
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

/** @ingroup USBD_MSC
 * @defgroup USBD_MSC_Private_Functions_SCSI MSC Private Functions SCSI
 * @{ */

/**
 * @brief Returns the reference of the last SCSI Sense data.
 * @param itf: reference of the MSC interface
 * @return Reference to the last SCSI Sense data.
 */
static USBD_SCSI_SenseType* SCSI_GetSenseCode(USBD_MSC_IfHandleType *itf)
{
    return &itf->SCSI.Sense;
}

/**
 * @brief Adds a new SCSI sense code to the context, and sets the CSW status.
 * @param itf: reference of the MSC interface
 * @param skey: sense key
 * @param asc: Additional Sense Code
 */
void SCSI_PutSenseCode(USBD_MSC_IfHandleType *itf,
        USBD_SCSI_SenseKeyType skey, USBD_SCSI_AddSenseCodeType asc)
{
    itf->SCSI.Sense.Key = skey;
    itf->SCSI.Sense.ASC = asc;

    itf->CSW.bStatus = MSC_CSW_CMD_FAILED;
}

/**
 * @brief Reads data from the current block to the transfer buffer
 *        and sends it over the IN endpoint.
 * @param itf: reference of the MSC interface
 * @return Result of the current block read operation
 */
USBD_ReturnType SCSI_ProcessRead(USBD_MSC_IfHandleType *itf)
{
    USBD_ReturnType retval;
    const USBD_MSC_LUType *LU = MSC_GetLU(itf, itf->CBW.bLUN);
    uint32_t len = sizeof(itf->Buffer);

    if (len > itf->SCSI.RemLength)
    {   len = itf->SCSI.RemLength; }

    retval = LU->Read(itf->CBW.bLUN, itf->Buffer,
            itf->SCSI.Address / LU->Status->BlockSize,
            len / LU->Status->BlockSize);

    if (retval != USBD_E_OK)
    {
        SCSI_PutSenseCode(itf, SCSI_SKEY_HARDWARE_ERROR,
                SCSI_ASC_UNRECOVERED_READ_ERROR);
    }
    else
    {
        USBD_HandleType *dev = itf->Base.Device;

        USBD_EpSend(dev, itf->Config.InEpNum, itf->Buffer, len);

        itf->SCSI.Address += len;
        itf->SCSI.RemLength -= len;

        itf->CSW.dDataResidue -= len;

        if (itf->SCSI.RemLength == 0)
        {
            /* Next transfer is CSW */
            itf->State = MSC_STATE_STATUS_IN;
        }
    }
    return retval;
}

/**
 * @brief Writes the received data to the current block, and continues or terminates
 *        further block data reception.
 * @param itf: reference of the MSC interface
 * @return Result of the current block write operation
 */
USBD_ReturnType SCSI_ProcessWrite(USBD_MSC_IfHandleType *itf)
{
    USBD_ReturnType retval;
    const USBD_MSC_LUType *LU = MSC_GetLU(itf, itf->CBW.bLUN);
    uint32_t len = sizeof(itf->Buffer);

    if (len > itf->SCSI.RemLength)
    {   len = itf->SCSI.RemLength; }

    retval = LU->Write(itf->CBW.bLUN, itf->Buffer,
                itf->SCSI.Address / LU->Status->BlockSize,
                len / LU->Status->BlockSize);

    if (retval != USBD_E_OK)
    {
        SCSI_PutSenseCode(itf, SCSI_SKEY_HARDWARE_ERROR,
                SCSI_ASC_WRITE_FAULT);
    }
    else
    {
        itf->SCSI.Address += len;
        itf->SCSI.RemLength -= len;

        itf->CSW.dDataResidue -= len;

        if (itf->SCSI.RemLength > 0)
        {
            USBD_HandleType *dev = itf->Base.Device;

            /* Prepare EP to receive next packet */
            USBD_EpReceive(dev, itf->Config.OutEpNum, itf->Buffer, len);
        }
    }
    return retval;
}

/**
 * @brief Sets up the LUN Inquiry for transmission.
 * @param itf: reference of the MSC interface
 * @return The length of the response data
 */
static uint32_t SCSI_Inquiry(USBD_MSC_IfHandleType *itf)
{
    PACKED(struct) {
        uint8_t OpCode;
        uint8_t EVPD;
        uint8_t PageCode;
        uint16_t AllocLength;
        uint8_t Control;
    }*cmd = (void*)itf->CBW.CB;
    uint8_t* data = itf->Buffer;
    const USBD_MSC_LUType *LU = MSC_GetLU(itf, itf->CBW.bLUN);
    uint32_t respLen, allocLen = ntohs(cmd->AllocLength);

    if (cmd->EVPD != 0)
    {
        /* (Enable) Vital Product Data: */
        respLen = 5;
        memset(data, 0, respLen);
    }
    else
    {
        respLen = LU->Inquiry->AddLength + 4;
        memcpy(data, LU->Inquiry, respLen);
    }

    if (respLen > allocLen)
    {   respLen = allocLen; }

    return respLen;
}

/**
 * @brief Sets up the device capacity data for transmission.
 * @param itf: reference of the MSC interface
 * @return The length of the response data
 */
static uint32_t SCSI_ReadCapacity10(USBD_MSC_IfHandleType *itf)
{
    PACKED(struct) {
        uint8_t OpCode;
        uint8_t __reserved0;
        uint32_t LogicalBlockAddr;
        uint8_t __reserved1[2];
        uint8_t PMI;
        uint8_t Control;
    }*cmd = (void*)itf->CBW.CB;
    struct {
        uint32_t BlockCount;
        uint32_t BlockLength;
    }*data = (void*)itf->Buffer;
    const USBD_MSC_LUType *LU = MSC_GetLU(itf, itf->CBW.bLUN);
    uint32_t respLen = 0;

    if (!LU->Status->Ready)
    {
        SCSI_PutSenseCode(itf, SCSI_SKEY_NOT_READY,
                SCSI_ASC_MEDIUM_NOT_PRESENT);
    }
    else
    {
        data->BlockCount  = htonl(LU->Status->BlockCount - 1);
        data->BlockLength = htonl(LU->Status->BlockSize);
        respLen = sizeof(*data);
    }

    (void)cmd;
    return respLen;
}

/**
 * @brief Sets up the device format capacity data for transmission.
 * @param itf: reference of the MSC interface
 * @return The length of the response data
 */
static uint32_t SCSI_ReadFormatCapacity(USBD_MSC_IfHandleType *itf)
{
    PACKED(struct) {
        uint8_t OpCode;
        union {
            struct {
                uint8_t : 5;
                uint8_t LUN : 3;
            };
            uint8_t b;
        };
        uint8_t __reserved0[5];
        uint16_t AllocLength;
        uint8_t __reserved1[3];
    }*cmd = (void*)itf->CBW.CB;
    PACKED(struct) {
        uint8_t __reserved0[3];
        uint8_t CapacityListLength;
        struct {
            uint32_t BlockCount;
            uint8_t  DescriptorCode;
            uint8_t  __reserved1;
            uint16_t BlockLength;
        }Capacity[1];
    }*data = (void*)itf->Buffer;
    const USBD_MSC_LUType *LU = MSC_GetLU(itf, itf->CBW.bLUN);
    uint32_t respLen = sizeof(*data), allocLen = ntohs(cmd->AllocLength);

    memset(data, 0, sizeof(*data));
    data->CapacityListLength = 8;
    data->Capacity[0].BlockCount     = htonl(LU->Status->BlockCount - 1);
    data->Capacity[0].DescriptorCode = 2; /* Formatted Media */
    data->Capacity[0].BlockLength    = htons(LU->Status->BlockSize);

    if (respLen > allocLen)
    {   respLen = allocLen; }

    return respLen;
}

/**
 * @brief Sets up MODE SENSE(6) data for transmission.
 * @param itf: reference of the MSC interface
 * @return The length of the response data
 */
static uint32_t SCSI_ModeSense6(USBD_MSC_IfHandleType *itf)
{
    PACKED(struct) {
        uint8_t OpCode;
        union {
            struct {
                uint16_t : 3;
                uint16_t DBD : 1;
                uint16_t : 4;
                uint16_t PageCode : 6;
                uint16_t PC : 2;
            };
            uint16_t __reserved;
        };
        uint8_t SubpageCode;
        uint8_t AllocLength;
        uint8_t Control;
    }*cmd = (void*)itf->CBW.CB;
    uint8_t (*data)[8] = (void*)itf->Buffer;
    uint32_t respLen = sizeof(*data);

    memset(data, 0, sizeof(*data));

    if (respLen > cmd->AllocLength)
    {   respLen = cmd->AllocLength; }

    return respLen;
}

/**
 * @brief Sets up MODE SENSE(10) data for transmission.
 * @param itf: reference of the MSC interface
 * @return The length of the response data
 */
static uint32_t SCSI_ModeSense10(USBD_MSC_IfHandleType *itf)
{
    PACKED(struct) {
        uint8_t OpCode;
        union {
            struct {
                uint16_t : 3;
                uint16_t DBD : 1;
                uint16_t LLBAA : 1;
                uint16_t : 3;
                uint16_t PageCode : 6;
                uint16_t PC : 2;
            };
            uint16_t __reserved;
        };
        uint8_t SubpageCode;
        uint8_t __reserved1[2];
        uint16_t AllocLength;
        uint8_t Control;
    }*cmd = (void*)itf->CBW.CB;
    uint8_t (*data)[8] = (void*)itf->Buffer;
    uint32_t respLen = sizeof(*data), allocLen = ntohs(cmd->AllocLength);

    memset(data, 0, sizeof(*data));

    if (respLen > allocLen)
    {   respLen = allocLen; }

    return respLen;
}

/**
 * @brief Sets up sense data for transmission.
 * @param itf: reference of the MSC interface
 * @return The length of the response data
 */
static uint32_t SCSI_RequestSense(USBD_MSC_IfHandleType *itf)
{
    PACKED(struct) {
        uint8_t OpCode;
        uint8_t DESC;
        uint8_t __reserved[2];
        uint8_t AllocLength;
        uint8_t Control;
    }*cmd = (void*)itf->CBW.CB;
    PACKED(struct) {
        uint8_t ResponseCode;
        uint8_t __reserved0;
        uint8_t SenseKey;
        uint8_t Information[4];
        uint8_t AddLength;
        uint8_t CmdSpecific[4];
        uint8_t ASC;
        uint8_t ASCQ;
        uint8_t FRUC;
        uint8_t SenseKeySpecific[3];
    }*data = (void*)itf->Buffer;
    USBD_SCSI_SenseType* sense = SCSI_GetSenseCode(itf);
    uint32_t respLen = sizeof(*data);

    memset(data, 0, sizeof(*data));
    data->ResponseCode = 0x70;
    data->AddLength    = sizeof(*data) - 7;

    if (sense != NULL)
    {
        data->SenseKey = sense->Key;
        data->ASC      = sense->ASC;
    }

    if (respLen > cmd->AllocLength)
    {   respLen = cmd->AllocLength; }

    return respLen;
}

/**
 * @brief Handles start-stop requests.
 * @param itf: reference of the MSC interface
 * @return The length of the response data: 0
 */
static uint32_t SCSI_StartStopUnit(USBD_MSC_IfHandleType *itf)
{
    PACKED(struct) {
        uint8_t OpCode;
        uint8_t IMMED;
        uint8_t __reserved0[2];
        union {
            struct {
                uint8_t START : 1;
                uint8_t LOEJ : 1;
                uint8_t : 2;
                uint8_t POWERCONDITION : 4;
            };
            uint8_t __reserved1;
        };
        uint8_t Control;
    }*cmd = (void*)itf->CBW.CB;

    (void)cmd;
    return 0;
}

/**
 * @brief Handles medium removal requests.
 * @param itf: reference of the MSC interface
 * @return The length of the response data
 */
static uint32_t SCSI_PreventAllowMediumRemoval(USBD_MSC_IfHandleType *itf)
{
    PACKED(struct) {
        uint8_t OpCode;
        uint8_t __reserved0[3];
        union {
            struct {
                uint8_t PREVENT : 2; /*  00b: allow removal
                                         01b: prohibit removal
                                      */
                uint8_t : 6;
            };
            uint8_t __reserved1;
        };
    }*cmd = (void*)itf->CBW.CB;

    (void)cmd;
    return 0;
}

/**
 * @brief Checks the accessibility (readiness) of the LUN by changing error state.
 * @param itf: reference of the MSC interface
 * @return The length of the response data
 */
static uint32_t SCSI_TestUnitReady(USBD_MSC_IfHandleType *itf)
{
    PACKED(struct) {
        uint8_t OpCode;
        uint8_t __reserved[4];
        uint8_t Control;
    }*cmd = (void*)itf->CBW.CB;
    const USBD_MSC_LUType *LU = MSC_GetLU(itf, itf->CBW.bLUN);

    /* case 9 : Hi > D0 */
    if (itf->CBW.dDataLength != 0)
    {
        SCSI_PutSenseCode(itf, SCSI_SKEY_ILLEGAL_REQUEST,
                SCSI_ASC_INVALID_CDB);
    }
    else if (!LU->Status->Ready)
    {
        SCSI_PutSenseCode(itf, SCSI_SKEY_NOT_READY,
                SCSI_ASC_MEDIUM_NOT_PRESENT);
    }
    else
    {
        /* Unit is ready, return with PASSED CSW */
    }

    (void)cmd;
    return 0;
}

/**
 * @brief Verifies the current block.
 * @param itf: reference of the MSC interface
 * @return The length of the response data
 */
static uint32_t SCSI_Verify10(USBD_MSC_IfHandleType *itf)
{
    PACKED(struct) {
        uint8_t OpCode;
        union {
            struct {
                uint8_t : 1;
                uint8_t BYTCHK : 1;
                uint8_t : 1;
                uint8_t DPO : 1;
                uint8_t VRPROTECT : 3;
            };
            uint8_t __reserved;
        };
        uint32_t BlockAddr;
        uint8_t GroupNr;
        uint16_t TransferLength;
        uint8_t Control;
    }*cmd = (void*)itf->CBW.CB;
    const USBD_MSC_LUType *LU = MSC_GetLU(itf, itf->CBW.bLUN);

    /* byte-by-byte comparison is not supported (it requires 2 buffers) */
    if (cmd->BYTCHK != 0)
    {
        SCSI_PutSenseCode(itf, SCSI_SKEY_ILLEGAL_REQUEST,
                SCSI_ASC_INVALID_FIELD_IN_COMMAND);
    }
    else if ((itf->SCSI.Address + itf->SCSI.RemLength) > LU->Status->BlockCount)
    {
        SCSI_PutSenseCode(itf, SCSI_SKEY_ILLEGAL_REQUEST,
                SCSI_ASC_ADDRESS_OUT_OF_RANGE);
    }
    return 0;
}

/**
 * @brief Checks if a block read operation is possible, and starts it.
 * @param itf: reference of the MSC interface
 * @return The length of the response data
 */
static uint32_t SCSI_Read10(USBD_MSC_IfHandleType *itf)
{
    PACKED(struct) {
        uint8_t OpCode;
        union {
            struct {
                uint8_t : 1;
                uint8_t FUA_NV : 1;
                uint8_t : 1;
                uint8_t FUA : 1;
                uint8_t DPO : 1;
                uint8_t RDPROTECT : 3;
            };
            uint8_t __reserved;
        };
        uint32_t BlockAddr;
        uint8_t GroupNr;
        uint16_t TransferLength;
        uint8_t Control;
    }*cmd = (void*)itf->CBW.CB;
    const USBD_MSC_LUType *LU = MSC_GetLU(itf, itf->CBW.bLUN);
    uint32_t blockAddr = ntohl(cmd->BlockAddr);
    uint16_t transferLen = ntohs(cmd->TransferLength);

    /* case (10): Ho <> Di */
    if (itf->CBW.bmFlags == 0)
    {
        SCSI_PutSenseCode(itf, SCSI_SKEY_ILLEGAL_REQUEST,
                SCSI_ASC_INVALID_CDB);
    }
    else if (!LU->Status->Ready)
    {
        SCSI_PutSenseCode(itf, SCSI_SKEY_NOT_READY,
                SCSI_ASC_MEDIUM_NOT_PRESENT);
    }
    else if ((blockAddr + transferLen) > LU->Status->BlockCount)
    {
        SCSI_PutSenseCode(itf, SCSI_SKEY_ILLEGAL_REQUEST,
                SCSI_ASC_ADDRESS_OUT_OF_RANGE);
    }
    else
    {
        itf->SCSI.Address = blockAddr * LU->Status->BlockSize;
        itf->SCSI.RemLength = transferLen * LU->Status->BlockSize;

        /* cases 4,5 : Hi <> Dn */
        if (itf->CBW.dDataLength != itf->SCSI.RemLength)
        {
            SCSI_PutSenseCode(itf, SCSI_SKEY_ILLEGAL_REQUEST,
                    SCSI_ASC_INVALID_CDB);
        }
        else
        {
            itf->State = MSC_STATE_DATA_IN;
            SCSI_ProcessRead(itf);
        }
    }
    return itf->CBW.dDataLength;
}

/**
 * @brief Checks if a block write operation is possible, and starts it.
 * @param itf: reference of the MSC interface
 * @return The length of the response data
 */
static uint32_t SCSI_Write10(USBD_MSC_IfHandleType *itf)
{
    PACKED(struct) {
        uint8_t OpCode;
        union {
            struct {
                uint8_t : 1;
                uint8_t FUA_NV : 1;
                uint8_t : 1;
                uint8_t FUA : 1;
                uint8_t DPO : 1;
                uint8_t WRPROTECT : 3;
            };
            uint8_t __reserved;
        };
        uint32_t BlockAddr;
        uint8_t GroupNr;
        uint16_t TransferLength;
        uint8_t Control;
    }*cmd = (void*)itf->CBW.CB;
    const USBD_MSC_LUType *LU = MSC_GetLU(itf, itf->CBW.bLUN);
    uint32_t blockAddr = ntohl(cmd->BlockAddr);
    uint16_t transferLen = ntohs(cmd->TransferLength);
    uint32_t respLen = sizeof(itf->Buffer);

    /* case 8 : Hi <> Do */
    if (itf->CBW.bmFlags != 0)
    {
        SCSI_PutSenseCode(itf, SCSI_SKEY_ILLEGAL_REQUEST,
                SCSI_ASC_INVALID_CDB);
    }
    else if (!LU->Status->Ready)
    {
        SCSI_PutSenseCode(itf, SCSI_SKEY_NOT_READY,
                SCSI_ASC_MEDIUM_NOT_PRESENT);
    }
    else if (!LU->Status->Writable)
    {
        SCSI_PutSenseCode(itf, SCSI_SKEY_NOT_READY,
                SCSI_ASC_WRITE_PROTECTED);
    }
    else if ((blockAddr + transferLen) > LU->Status->BlockCount)
    {
        SCSI_PutSenseCode(itf, SCSI_SKEY_ILLEGAL_REQUEST,
                SCSI_ASC_ADDRESS_OUT_OF_RANGE);
    }
    else
    {
        itf->SCSI.Address = blockAddr * LU->Status->BlockSize;
        itf->SCSI.RemLength = transferLen * LU->Status->BlockSize;

        /* cases 3,11,13 : Hn,Ho <> D0 */
        if (itf->CBW.dDataLength != itf->SCSI.RemLength)
        {
            SCSI_PutSenseCode(itf, SCSI_SKEY_ILLEGAL_REQUEST,
                    SCSI_ASC_INVALID_CDB);
        }
        else
        {
            USBD_HandleType *dev = itf->Base.Device;

            if (respLen > itf->SCSI.RemLength)
            {   respLen = itf->SCSI.RemLength; }

            itf->State = MSC_STATE_DATA_OUT;
            USBD_EpReceive(dev, itf->Config.OutEpNum, itf->Buffer, respLen);
        }
    }
    return respLen;
}

/**
 * @brief Routes the SCSI command to its handler based on the operation code.
 * @param itf: reference of the MSC interface
 */
void SCSI_ProcessCommand(USBD_MSC_IfHandleType *itf)
{
    uint32_t respLen = 0;

    /* OPERATION CODE */
    switch (itf->CBW.CB[0])
    {
        case SCSI_READ10:
            respLen = SCSI_Read10(itf);
            break;

        case SCSI_WRITE10:
            respLen = SCSI_Write10(itf);
            break;

        case SCSI_VERIFY10:
            respLen = SCSI_Verify10(itf);
            break;

        case SCSI_INQUIRY:
            respLen = SCSI_Inquiry(itf);
            break;

        case SCSI_READ_FORMAT_CAPACITIES:
            respLen = SCSI_ReadFormatCapacity(itf);
            break;

        case SCSI_TEST_UNIT_READY:
            respLen = SCSI_TestUnitReady(itf);
            break;

        case SCSI_REQUEST_SENSE:
            respLen = SCSI_RequestSense(itf);
            break;

        case SCSI_START_STOP_UNIT:
            respLen = SCSI_StartStopUnit(itf);
            break;

        case SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL:
            respLen = SCSI_PreventAllowMediumRemoval(itf);
            break;

        case SCSI_MODE_SENSE6:
            respLen = SCSI_ModeSense6(itf);
            break;

        case SCSI_MODE_SENSE10:
            respLen = SCSI_ModeSense10(itf);
            break;

        case SCSI_READ_CAPACITY10:
            respLen = SCSI_ReadCapacity10(itf);
            break;

        default:
            SCSI_PutSenseCode(itf, SCSI_SKEY_ILLEGAL_REQUEST,
                    SCSI_ASC_INVALID_CDB);
            break;
    }

    if (respLen > itf->CBW.dDataLength)
    {   respLen = itf->CBW.dDataLength; }

    /* Unhandled transfers are sent here */
    if ((itf->CSW.bStatus == MSC_CSW_CMD_PASSED) &&
        (itf->State == MSC_STATE_COMMAND_OUT) && (respLen > 0))
    {
        /* Send command response */
        USBD_EpSend(itf->Base.Device, itf->Config.InEpNum, itf->Buffer, respLen);

        itf->CSW.dDataResidue -= respLen;

        /* Send CSW next */
        itf->State = MSC_STATE_STATUS_IN;
    }
}

/** @} */
