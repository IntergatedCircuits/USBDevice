/**
  ******************************************************************************
  * @file    usbd_msc.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-03-25
  * @brief   Universal Serial Bus Mass Storage Class
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
#ifndef __USBD_MSC_H
#define __USBD_MSC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include  <usbd_types.h>

/** @ingroup USBD
 * @addtogroup USBD_Class USBD Classes
 * @{ */

/** @defgroup USBD_MSC Mass Storage Class (MSC)
 * @{ */

/** @defgroup USBD_MSC_Exported_Macros MSC Exported Macros
 * @{ */

#ifndef USBD_MSC_BUFFER_SIZE
#define USBD_MSC_BUFFER_SIZE        512
#endif

/** @} */

/** @defgroup USBD_MSC_Exported_Types MSC Exported Types
 * @{ */

/** @brief SCSI peripheral types */
typedef enum
{
    SCSI_PERIPH_SBC_2       = 0x00, /*!< Direct access block device */
    SCSI_PERIPH_SSC_2       = 0x01, /*!< Sequential-access device */
    SCSI_PERIPH_SSC         = 0x02, /*!< Printer device */
    SCSI_PERIPH_SPC_2       = 0x03, /*!< Processor device */
    SCSI_PERIPH_SBC         = 0x04, /*!< Write-once device */
    SCSI_PERIPH_MMC_4       = 0x05, /*!< MMC-4 CD/DVD device */
    SCSI_PERIPH_SBC_OPT     = 0x07, /*!< SBC Optical memory device */
    SCSI_PERIPH_SMC_2       = 0x08, /*!< Medium changer device */
    SCSI_PERIPH_SCC_2       = 0x0C, /*!< Storage array controller device */
    SCSI_PERIPH_SES         = 0x0D, /*!< SES Enclosure services device */
    SCSI_PERIPH_RBC         = 0x0E, /*!< Simplified direct-access device */
    SCSI_PERIPH_OCRW        = 0x0F, /*!< Optical card reader/writer device */
    SCSI_PERIPH_BCC         = 0x10, /*!< Bridge Controller Commands */
    SCSI_PERIPH_OSD         = 0x11, /*!< Object-based Storage Device */
    SCSI_PERIPH_ADC         = 0x12, /*!< Automation/Drive Interface */
    SCSI_PERIPH_WELLKNOWN   = 0x1E, /*!< Well known logical unit */
    SCSI_PERIPH_UNKNOWN     = 0x1F, /*!< Unknown or no device type */
}USBD_SCSI_PeripheralType;


/** @brief SCSI standard inquiry structure */
typedef struct
{
    union {
        struct {
            /* Byte 0 */
            USBD_SCSI_PeripheralType PeriphType : 5;/*!< Peripheral device type */
            uint8_t PeriphQual : 3;     /*!< Leave 0 */
            /* Byte 1 */
            uint8_t : 7;
            uint8_t RMB : 1;            /*!< Removable Medium bit */
            /* Byte 2 */
            uint8_t Version : 8;        /*!< 2 if not SPC standard */
            /* Byte 3 */
            uint8_t RespDataFormat : 4; /*!< Always 2 */
            uint8_t : 4;
            /* Byte 4 */
            uint8_t AddLength : 8;      /*!< Set to sizeof(USBD_SCSI_StdInquiryType) - 4 */
            uint32_t : 24;
        };
        uint8_t __reserved[8];
    };
    char VendorId[8];                   /*!< ASCII string of vendor */
    char ProductId[16];                 /*!< ASCII string of product */
    char VersionId[4];                  /*!< ASCII string of version */
}USBD_SCSI_StdInquiryType;


/** @brief SCSI Sense information */
typedef struct
{
    uint8_t Key;    /*!< Sense Key */
    uint8_t ASC;    /*!< Additional Sense Code */
}USBD_SCSI_SenseType;


/** @brief MSC interface transfer states */
typedef enum
{
    MSC_STATE_COMMAND_OUT   = 0, /*!< Waiting for new CBW */
    MSC_STATE_DATA_OUT      = 1, /*!< Block write ongoing */
    MSC_STATE_DATA_IN       = 2, /*!< Block read ongoing */
    MSC_STATE_STATUS_IN     = 3, /*!< Send CSW after completed command response */
    MSC_STATE_STALL         = 4, /*!< Transfer is blocked by error condition */
}USBD_MSC_StateType;


/** @brief MSC error status */
typedef enum
{
    MSC_STATUS_NORMAL   = 0,    /*!< Normal status */
    MSC_STATUS_RECOVERY = 1,    /*!< In recovery due to BOT RESET */
    MSC_STATUS_ERROR    = 2,    /*!< Error status */
}USBD_MSC_StatusType;


/** @brief MSC CSW status */
typedef enum
{
    MSC_CSW_CMD_PASSED  = 0x00, /*!< CBW command processed successfully */
    MSC_CSW_CMD_FAILED  = 0x01, /*!< CBW command execution failed */
    MSC_CSW_PHASE_ERROR = 0x02, /*!< CBW command received in incorrect device state */
}USBD_MSC_CSWStatusType;


/** @brief Standard MSC control requests */
typedef enum
{
    MSC_BOT_GET_MAX_LUN = 0xFE, /*!< Return the highest available LUN */
    MSC_BOT_RESET       = 0xFF, /*!< Reset the MSC BOT interface */
}USBD_MSC_RequestType;


/** @brief MSC Command Block Wrapper message structure */
typedef PACKED(struct)
{
    uint32_t dSignature;    /*!< CBW identifier signature */
    uint32_t dTag;          /*!< Tag to bind CSW to CBW */
    uint32_t dDataLength;   /*!< Number of data bytes to transfer to this command */
    uint8_t bmFlags;        /*!< Bit 7 = @ref USB_DirectionType */
    uint8_t bLUN;           /*!< Logical Unit selector Number */
    uint8_t bCBLength;      /*!< Length of the Command Block */
    uint8_t CB[16];         /*!< Command Block */
}USBD_MSC_CommandBlockWrapperType;


/** @brief MSC Command Status Wrapper message structure */
typedef PACKED(struct)
{
    uint32_t dSignature;    /*!< CBW identifier signature */
    uint32_t dTag;          /*!< Tag to bind CSW to CBW */
    uint32_t dDataResidue;  /*!< Difference between host requested length and actual sent data length */
    uint8_t bStatus;        /*!< @ref USBD_MSC_CSWStatusType command execution status */
}USBD_MSC_CommandStatusWrapperType;


/** @brief MSC Logical Unit status */
typedef struct
{
    uint32_t BlockCount;    /*!< Number of available blocks */
    uint16_t BlockSize;     /*!< Size of each block */
    uint8_t  Ready;         /*!< Indicates whether the LU is ready (used as boolean) */
    uint8_t  Writable;      /*!< Indicates whether the LU is writable (used as boolean) */
}USBD_MSC_LUStatusType;


/** @brief MSC Logical Unit interfacing structure */
typedef struct
{
    void           (*Init)  (uint8_t lun);      /*!< Initialize media (optional) */

    void           (*Deinit)(uint8_t lun);      /*!< Release media (optional) */

    USBD_ReturnType(*Read)  (uint8_t lun,
                             uint8_t *dest,
                             uint32_t blockAddr,
                             uint16_t blockLen);/*!< Read media block */

    USBD_ReturnType(*Write) (uint8_t lun,
                             uint8_t *src,
                             uint32_t blockAddr,
                             uint16_t blockLen);/*!< Write media block */

    USBD_MSC_LUStatusType*          Status;     /*!< Up-to-date status of Logical Unit */

    const USBD_SCSI_StdInquiryType* Inquiry;    /*!< Standard Inquiry of Logical Unit */
}USBD_MSC_LUType;


/** @brief MSC interface configuration */
typedef struct
{
    uint8_t OutEpNum;   /*!< OUT endpoint address */
    uint8_t InEpNum;    /*!< IN endpoint address */
    uint8_t MaxLUN;     /*!< Last Logical Unit Number in
                             @ref USBD_MSC_IfHandleType::LUs (starting from 0) */
}USBD_MSC_ConfigType;


/** @brief MSC class interface structure */
typedef struct
{
    USBD_IfHandleType Base;                 /*!< Class-independent interface base */
    const USBD_MSC_LUType* LUs;             /*!< Logical Units reference */
    USBD_MSC_ConfigType Config;             /*!< MSC interface configuration */
    USBD_PADDING_1(a);

    /* MSC class internal context */
    uint8_t Buffer[USBD_MSC_BUFFER_SIZE];   /*!< Block transferring buffer */
    USBD_MSC_CommandBlockWrapperType  CBW;  /*!< Command Block Wrapper */
    USBD_PADDING_1(b);
    USBD_MSC_CommandStatusWrapperType CSW;  /*!< Command Status Wrapper */
    USBD_PADDING_3(c);

    USBD_MSC_StateType  State;              /*!< MSC interface state */
    USBD_MSC_StatusType Status;             /*!< MSC interface error status */

    struct {
        USBD_SCSI_SenseType Sense;          /*!< Last sense data */
        uint32_t Address;                   /*!< Current address in LU block */
        uint32_t RemLength;                 /*!< Remaining block length to transfer */
    }SCSI;                                  /*!< SCSI context data */
}USBD_MSC_IfHandleType;

/** @} */

/** @addtogroup USBD_MSC_Exported_Functions
 * @{ */
USBD_ReturnType USBD_MSC_MountInterface (USBD_MSC_IfHandleType *itf,
                                         USBD_HandleType *dev);
/** @} */

/** @} */

/** @} */


#ifdef __cplusplus
}
#endif

#endif  /* __USBD_MSC_H */
