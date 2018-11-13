/**
  ******************************************************************************
  * @file    usbd_dfu.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-03-11
  * @brief   USB Device Firmware Upgrade Class
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
#ifndef __USBD_DFU_H
#define __USBD_DFU_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <usbd_types.h>

/** @ingroup USBD
 * @addtogroup USBD_Class USBD Classes
 * @{ */

/** @defgroup USBD_DFU Device Firmware Upgrade Class (DFU)
 * @{ */

/** @defgroup USBD_DFU_Exported_Macros DFU Exported Macros
 * @{ */

#define DFU_DESC_TYPE_FUNCTIONAL    0x21

#define DFU_MODE_TAG                0xB00770DFU /* BOOT TO DFU */

/** @} */

/** @defgroup USBD_DFU_Exported_Types DFU Exported Types
 * @{ */

/** @brief DFU class specific control requests */
typedef enum
{
    DFU_REQ_DETACH      = 0, /*!< Detach the application from the USB device */
    DFU_REQ_DNLOAD      = 1, /*!< Download a block to the program memory */
    DFU_REQ_UPLOAD      = 2, /*!< Read a block of the program memory */
    DFU_REQ_GETSTATUS   = 3, /*!< Read the DFU status */
    DFU_REQ_CLRSTATUS   = 4, /*!< Clear the error status */
    DFU_REQ_GETSTATE    = 5, /*!< Read the DFU state */
    DFU_REQ_ABORT       = 6, /*!< Abort the ongoing request */
}USBD_DFU_RequestType;


/** @brief DFU device states */
typedef enum
{
    DFU_STATE_APP_IDLE              = 0,   /*!< Device is running its normal application. */
    DFU_STATE_APP_DETACH            = 1,   /*!< Device is running its normal application,
                                                has received the DFU_DETACH request,
                                                and is waiting for a USB reset. */
    DFU_STATE_IDLE                  = 2,   /*!< Device is waiting for requests in DFU mode. */
    DFU_STATE_DNLOAD_SYNC           = 3,   /*!< Device has received a block and is waiting for the host
                                                to solicit the status via DFU_GETSTATUS. */
    DFU_STATE_DNLOAD_BUSY           = 4,   /*!< Device is programming a control-write block into its
                                                non-volatile memories. */
    DFU_STATE_DNLOAD_IDLE           = 5,   /*!< Device is processing a download operation. */
    DFU_STATE_MANIFEST_SYNC         = 6,   /*!< Device has received the final block of firmware
                                                from the host and is waiting for receipt of
                                                DFU_GETSTATUS to begin the Manifestation phase;
                                                or device has completed the Manifestation phase and
                                                is waiting for receipt of DFU_GETSTATUS. */
    DFU_STATE_MANIFEST              = 7,   /*!< Device is in the Manifestation phase. */
    DFU_STATE_MANIFEST_WAIT_RESET   = 8,   /*!< Device has programmed its memories and is waiting for a
                                                USB reset or a power on reset. */
    DFU_STATE_UPLOAD_IDLE           = 9,   /*!< The device is processing an upload operation. */
    DFU_STATE_ERROR                 = 10,  /*!< An error has occurred. */
}USBD_DFU_StateType;


/** @brief DFU error statuses */
typedef enum
{
    DFU_ERROR_NONE          = 0x00, /*!< No error condition is present. */
    DFU_ERROR_TARGET        = 0x01, /*!< File is not targeted for use by this device. */
    DFU_ERROR_FILE          = 0x02, /*!< File is for this device but fails some vendor-specific
                                         verification test. */
    DFU_ERROR_WRITE         = 0x03, /*!< Device is unable to write memory. */
    DFU_ERROR_ERASE         = 0x04, /*!< Memory erase function failed. */
    DFU_ERROR_CHECK_ERASED  = 0x05, /*!< Memory erase check failed. */
    DFU_ERROR_PROG          = 0x06, /*!< Program memory function failed. */
    DFU_ERROR_VERIFY        = 0x07, /*!< Programmed memory failed verification. */
    DFU_ERROR_ADDRESS       = 0x08, /*!< Cannot program memory due to received address that is
                                         out of range. */
    DFU_ERROR_NOTDONE       = 0x09, /*!< Received DFU_DNLOAD with wLength = 0, but device does
                                         not think it has all of the data yet. */
    DFU_ERROR_FIRMWARE      = 0x0A, /*!< Device's firmware is corrupt. It cannot return to run-time
                                         (non-DFU) operations. */
    DFU_ERROR_VENDOR        = 0x0B, /*!< iString indicates a vendor-specific error. */
    DFU_ERROR_USB           = 0x0C, /*!< Device detected unexpected USB reset signaling. */
    DFU_ERROR_POR           = 0x0D, /*!< Device detected unexpected power on reset. */
    DFU_ERROR_UNKNOWN       = 0x0E, /*!< Something went wrong. */
    DFU_ERROR_STALLEDPKT    = 0x0F, /*!< Device stalled an unexpected request. */
}USBD_DFU_StatusType;


/** @brief DFU status structure */
typedef struct
{
    uint8_t  Status;        /*!< Error status from execution of the last request. */
    uint16_t PollTimeout;   /*!< Minimum time [ms] between subsequent DFU_GETSTATUS requests. */
    uint8_t  __reserved;    /* PollTimeout is 24 bits */
    uint8_t  State;         /*!< The new device state. */
    uint8_t  iString;       /*!< Index of status description in string table. */
}__packed USBD_DFU_StatusDataType;


/** @brief DFU application structure */
typedef struct
{
    const char* Name;                           /*!< String description of the application */

    void                (*Init)         (void); /*!< Initialization request */

    void                (*Deinit)       (void); /*!< Shutdown request */

    USBD_DFU_StatusType (*Manifest)     (void); /*!< Verify new firmware integrity, and set its validity */

    USBD_DFU_StatusType (*Erase)        (uint8_t *addr);/*!< Erase any existing firmware at address
                                                             @note DFUSE variant should only erase one flash block */

    USBD_DFU_StatusType (*Write)        (uint8_t *addr,
                                         uint8_t *data,
                                         uint32_t len); /*!< Write the input data to the specified address */

    void                (*Read)         (uint8_t *addr,
                                         uint8_t *data,
                                         uint32_t len); /*!< Read the firmware to the output buffer */

    uint16_t            (*GetTimeout_ms)(uint8_t *addr,
                                         uint32_t len); /*!< Get the required time [ms] for a (Erase +) Write or
                                                             Manifest operation of the specified length */

    struct {
        uint32_t Address;   /*!< Start address of the application firmware */
        uint32_t TotalSize; /*!< Total size of the application firmware in bytes */
    }Firmware;
}USBD_DFU_AppType;


/**
 * @brief System reset callback function pointer type
 */
typedef void (*USBD_DFU_RebootCbkType)  (void);


/** @brief DFU interface configuration */
typedef struct
{
    USBD_DFU_RebootCbkType Reboot;      /*!< Function pointer to the system reboot method */
    uint16_t DetachTimeout_ms;          /*!< Required time for DFU detach sequence (reboot) [ms] */
}USBD_DFU_ConfigType;


/** @brief DFU class interface structure */
typedef struct
{
    USBD_IfHandleType Base;             /*!< Class-independent interface base */
    const USBD_DFU_AppType* App;        /*!< DFU application reference */
    USBD_DFU_ConfigType Config;         /*!< DFU interface configuration */
    USBD_PADDING_2(a);

    uint32_t Tag[2];                    /*!< Enter DFU mode request tag */
    uint16_t BlockNum;                  /*!< Current firmware transfer block number */
    uint16_t BlockLength;               /*!< Current firmware transfer block length */
    uint8_t* Address;                   /*!< Current firmware address for transfer */
    USBD_DFU_StatusDataType DevStatus;  /*!< Device DFU status */
    USBD_PADDING_2(b);
}USBD_DFU_IfHandleType;

/** @} */

/** @addtogroup USBD_DFU_Exported_Functions_Boot
 * @{ */
void            USBD_DFU_BootInit       (USBD_DFU_IfHandleType *itf,
                                         USBD_DFU_RebootCbkType pReboot,
                                         const USBD_DFU_AppType *app,
                                         uint8_t appCount);

int             USBD_DFU_IsRequested    (USBD_DFU_IfHandleType *itf);
/** @} */

/** @addtogroup USBD_DFU_Exported_Functions_App
 * @{ */
void            USBD_DFU_AppInit        (USBD_DFU_IfHandleType *itf,
                                         uint16_t detachTimeout_ms);

USBD_ReturnType USBD_DFU_MountRebootOnly(USBD_DFU_IfHandleType *itf,
                                         USBD_HandleType *dev);

/** @} */

/** @addtogroup USBD_DFU_Exported_Functions
 * @{ */
USBD_ReturnType USBD_DFU_MountInterface (USBD_DFU_IfHandleType *itf,
                                         USBD_HandleType *dev);
/** @} */

/** @} */

/** @} */

#ifdef __cplusplus
}
#endif

#endif  /* __USBD_DFU_H */
