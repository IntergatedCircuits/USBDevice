/**
  ******************************************************************************
  * @file    usbd_msc_private.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-03-25
  * @brief   Universal Serial Bus Mass Storage Class
  *          Private cross-domain functions header
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
#ifndef __USBD_MSC_PRIVATE_H_
#define __USBD_MSC_PRIVATE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <usbd_msc.h>

/** @ingroup USBD_MSC
 * @defgroup USBD_MSC_Private_Types MSC Private Types
 * @{ */

/** @brief SCSI/UFI commands */
typedef enum
{
    SCSI_FORMAT_UNIT                    = 0x04,
    SCSI_INQUIRY                        = 0x12,
    SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL   = 0x1E,
    SCSI_REQUEST_SENSE                  = 0x03,
    SCSI_START_STOP_UNIT                = 0x1B,
    SCSI_TEST_UNIT_READY                = 0x00,
    SCSI_SEND_DIAGNOSTIC                = 0x1D,
    SCSI_READ_FORMAT_CAPACITIES         = 0x23,

    SCSI_READ_CAPACITY10                = 0x25,
    SCSI_READ_CAPACITY16                = 0x9E,

    SCSI_MODE_SELECT6                   = 0x15,
    SCSI_MODE_SELECT10                  = 0x55,

    SCSI_MODE_SENSE6                    = 0x1A,
    SCSI_MODE_SENSE10                   = 0x5A,

    SCSI_READ6                          = 0x08,
    SCSI_READ10                         = 0x28,
    SCSI_READ12                         = 0xA8,
    SCSI_READ16                         = 0x88,

    SCSI_WRITE6                         = 0x0A,
    SCSI_WRITE10                        = 0x2A,
    SCSI_WRITE12                        = 0xAA,
    SCSI_WRITE16                        = 0x8A,

    SCSI_VERIFY10                       = 0x2F,
    SCSI_VERIFY12                       = 0xAF,
    SCSI_VERIFY16                       = 0x8F,
}USBD_SCSI_OperationCodeType;

/** @brief SCSI Sense Keys */
typedef enum
{
    SCSI_SKEY_NO_SENSE          = 0,
    SCSI_SKEY_RECOVERED_ERROR   = 1,
    SCSI_SKEY_NOT_READY         = 2,
    SCSI_SKEY_MEDIUM_ERROR      = 3,
    SCSI_SKEY_HARDWARE_ERROR    = 4,
    SCSI_SKEY_ILLEGAL_REQUEST   = 5,
    SCSI_SKEY_UNIT_ATTENTION    = 6,
    SCSI_SKEY_DATA_PROTECT      = 7,
    SCSI_SKEY_BLANK_CHECK       = 8,
    SCSI_SKEY_VENDOR_SPECIFIC   = 9,
    SCSI_SKEY_COPY_ABORTED      = 10,
    SCSI_SKEY_ABORTED_COMMAND   = 11,
    SCSI_SKEY_VOLUME_OVERFLOW   = 13,
    SCSI_SKEY_MISCOMPARE        = 14,
}USBD_SCSI_SenseKeyType;

/** @brief SCSI Additional Sense Codes */
typedef enum
{
    SCSI_ASC_INVALID_CDB                    = 0x20,
    SCSI_ASC_INVALID_FIELD_IN_COMMAND       = 0x24,
    SCSI_ASC_PARAMETER_LIST_LENGTH_ERROR    = 0x1A,
    SCSI_ASC_ADDRESS_OUT_OF_RANGE           = 0x21,
    SCSI_ASC_MEDIUM_NOT_PRESENT             = 0x3A,
    SCSI_ASC_MEDIUM_HAVE_CHANGED            = 0x28,
    SCSI_ASC_WRITE_PROTECTED                = 0x27,
    SCSI_ASC_UNRECOVERED_READ_ERROR         = 0x11,
    SCSI_ASC_WRITE_FAULT                    = 0x03,
}USBD_SCSI_AddSenseCodeType;

/** @} */

const USBD_MSC_LUType* MSC_GetLU    (USBD_MSC_IfHandleType *itf,
                                     uint8_t lunIndex);

void            SCSI_ProcessCommand (USBD_MSC_IfHandleType *itf);
USBD_ReturnType SCSI_ProcessRead    (USBD_MSC_IfHandleType *itf);
USBD_ReturnType SCSI_ProcessWrite   (USBD_MSC_IfHandleType *itf);

void            SCSI_PutSenseCode   (USBD_MSC_IfHandleType *itf,
                                     USBD_SCSI_SenseKeyType skey,
                                     USBD_SCSI_AddSenseCodeType asc);


#ifdef __cplusplus
}
#endif

#endif /* __USBD_MSC_PRIVATE_H_ */
