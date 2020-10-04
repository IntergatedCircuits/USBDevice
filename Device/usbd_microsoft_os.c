/**
  ******************************************************************************
  * @file    usbd_microsoft_os.c
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2020-08-25
  * @brief   Universal Serial Bus Device Driver
  *          Microsoft OS descriptor definitions
  *
  * Copyright (c) 2020 Benedek Kupper
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

#if (USBD_MS_OS_DESC_VERSION > 0)

/** @ingroup USBD_Private
 * @defgroup USBD_Private_Functions_MS USBD Microsoft OS Descriptors Provision
 * @{ */

#if (USBD_MS_OS_DESC_VERSION == 1)

/**
 * @brief This function assembles the USB Microsoft OS 1.0 compatible ID descriptor
 *        using the compatible IDs of the mounted interfaces.
 * @param dev: USB Device handle reference
 * @param data: the target container for the configuration descriptor
 * @return The length of the descriptor
 */
static uint16_t USBD_MsOsCompatIdDesc(USBD_HandleType *dev, uint8_t *data)
{
    USB_MsCompatIdDescType *devCompatId = (void*)data;
    USBD_IfHandleType *itf = NULL;
    uint8_t ifNum;

    memset(devCompatId, 0, sizeof(*devCompatId));
    devCompatId->dwLength   = sizeof(USB_MsCompatIdDescType);
    devCompatId->bcdVersion = USBD_MS_OS_DESC_VERSION << 8;
    devCompatId->wIndex     = USB_MS_OS_1p0_EXTENDED_COMPAT_ID_INDEX;
    devCompatId->bCount     = 0;

    /* Get the individual functions */
    for (ifNum = 0; ifNum < dev->IfCount; ifNum++)
    {
        const char *compatIdStr;

        /* Associated interfaces form a single function */
        if (dev->IF[ifNum] == itf) { continue; }

        itf = dev->IF[ifNum];
        compatIdStr = USBD_IfClass_GetMsCompatibleId(itf);

        /* all functions get a descriptor, at least empty ones */
        memset(&devCompatId->Function[devCompatId->bCount], 0, sizeof(devCompatId->Function[0]));

        devCompatId->Function[devCompatId->bCount].bFirstInterfaceNumber    = ifNum;
        //devCompatId->Function[devCompatId->bCount]._Reserved1[0]            = 1;

        if (compatIdStr != NULL)
        {
            strncpy(devCompatId->Function[devCompatId->bCount].CompatibleID,
                    compatIdStr, sizeof(devCompatId->Function[devCompatId->bCount].CompatibleID));
        }

        /* Note the number of functions */
        devCompatId->bCount++;
    }
    /* When finished with the contents, save the total size of the set */
    devCompatId->dwLength += devCompatId->bCount * sizeof(devCompatId->Function[0]);

    return devCompatId->dwLength;
}

/**
 * @brief This function collects and transfers the requested Microsoft descriptor through EP0.
 * @param dev: USB Device handle reference
 * @return OK if the descriptor is provided, INVALID if not supported
 */
USBD_ReturnType USBD_GetMsDescriptor(USBD_HandleType *dev)
{
    USBD_ReturnType retval = USBD_E_INVALID;
    uint8_t *data = dev->CtrlData;
    uint16_t len = 0;
    //uint8_t ifNum = (uint8_t)dev->Setup.Value >> 8;
    //uint8_t pageNum = (uint8_t)dev->Setup.Value;

    switch (dev->Setup.Index)
    {
        case USB_MS_OS_1p0_EXTENDED_COMPAT_ID_INDEX:
        {
            /* Only one descriptor per device */
            len = USBD_MsOsCompatIdDesc(dev, data);
            break;
        }

        case USB_MS_OS_1p0_GENRE_INDEX:
        case USB_MS_OS_1p0_EXTENDED_PROPERTIES_INDEX:
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

#elif (USBD_MS_OS_DESC_VERSION == 2)

/**
 * @brief This function assembles the USB Microsoft OS 2.0 descriptor
 *        using the compatible IDs of the mounted interfaces.
 * @param dev: USB Device handle reference
 * @param data: the target container for the configuration descriptor
 * @return The length of the descriptor
 */
uint16_t USBD_MsOs2p0Desc(USBD_HandleType *dev, uint8_t *data)
{
    USB_MsDescSetHeaderType *descSet = (void*)data;

    /* Device header */
    descSet->wLength            = sizeof(USB_MsDescSetHeaderType);
    descSet->wDescriptorType    = USB_MS_OS_2p0_SET_HEADER_DESCRIPTOR;
    descSet->dwWindowsVersion   = USB_MS_OS_2P0_MIN_WINDOWS_VERSION;
    data += descSet->wLength;
    {
        /* Device-level features */
#if 0
        /* an example on how to force Windows to use composite device driver */
        USB_MsCcgpDescType *ccgpDesc;

        ccgpDesc = (void*)data;
        ccgpDesc->wLength           = sizeof(USB_MsCcgpDescType);
        ccgpDesc->wDescriptorType   = USB_MS_OS_2p0_FEATURE_CCGP_DEVICE;
        data += ccgpDesc->wLength;
#endif
        {
            /* Configuration subset */
            USB_MsConfSubsetHeaderType *confSubset = (void*)data;

            confSubset->wLength             = sizeof(USB_MsConfSubsetHeaderType);
            confSubset->wDescriptorType     = USB_MS_OS_2p0_SUBSET_HEADER_CONFIGURATION;
            confSubset->bConfigurationValue = 0; /* ~ USBD_ConfigDesc.bConfigurationValue - 1 */
            confSubset->bReserved           = 0;
            data += confSubset->wLength;
            {
                /* Configuration-level features */


                /* Function subset */
                uint8_t ifNum;
                USBD_IfHandleType *itf = NULL;

                /* Get the individual functions */
                for (ifNum = 0; ifNum < dev->IfCount; ifNum++)
                {
                    USB_MsFuncSubsetHeaderType *funcSubset;
                    const char *compatIdStr;

                    /* Associated interfaces form a single function */
                    if (dev->IF[ifNum] == itf) { continue; }

                    itf = dev->IF[ifNum];

                    /* If the compatible ID is defined, add the feature under the function header */
                    funcSubset = (void*)data;
                    funcSubset->wLength         = sizeof(USB_MsFuncSubsetHeaderType);
                    funcSubset->wDescriptorType = USB_MS_OS_2p0_SUBSET_HEADER_FUNCTION;
                    funcSubset->bFirstInterface = ifNum;
                    funcSubset->bReserved       = 0;
                    data += funcSubset->wLength;

                    compatIdStr = USBD_IfClass_GetMsCompatibleId(itf);
                    if (compatIdStr != NULL)
                    {
                        /* Function-level features */
                        USB_MsCompatIdDescType *compatId;

                        compatId = (void*)data;
                        compatId->wLength           = sizeof(USB_MsCompatIdDescType);
                        compatId->wDescriptorType   = USB_MS_OS_2p0_FEATURE_COMPATBLE_ID;
                        memset (compatId->CompatibleID, 0, sizeof(compatId->CompatibleID) + sizeof(compatId->SubCompatibleID));
                        strncpy(compatId->CompatibleID, compatIdStr, sizeof(compatId->CompatibleID));
                        data += compatId->wLength;

#if 0
                        {
                            /* It is highly recommended to add a revision descriptor e.g. in case
                             * registry values are defined, as these are only applied once per revision */
                            USB_MsVendorRevDescType *vendorRev;
                            vendorRev = (void*)data;
                            vendorRev->wLength          = sizeof(USB_MsVendorRevDescType);
                            vendorRev->wDescriptorType  = USB_MS_OS_2p0_FEATURE_VENDOR_REVISION;
                            vendorRev->wVendorRevision  = dev->Desc->Product.Version.bcd;
                            data += vendorRev->wLength;
                        }
#endif
                    }

                    /* When finished with the features, save the total size of the subset */
                    if (data > ((uint8_t*)funcSubset + funcSubset->wLength))
                    {
                        funcSubset->wSubsetLength = data - ((uint8_t*)funcSubset);
                    }
                    else
                    {
                        /* If no features are added, roll back this subset */
                        data -= funcSubset->wLength;
                    }
                }
            }

            /* When finished with the contents, save the total size of the subset */
            if (data > ((uint8_t*)confSubset + confSubset->wLength))
            {
                confSubset->wTotalLength = data - ((uint8_t*)confSubset);
            }
            else
            {
                /* If no features are added, roll back this subset */
                data -= confSubset->wLength;
            }
        }
    }

    /* When finished with the contents, save the total size of the set */
    if (data > ((uint8_t*)descSet + descSet->wLength))
    {
        descSet->wTotalLength = data - ((uint8_t*)descSet);
    }
    else
    {
        /* If no features are added in the whole set, reject this request */
        descSet->wTotalLength = 0;
    }

    return descSet->wTotalLength;
}

/**
 * @brief This function collects and transfers the requested Microsoft descriptor through EP0.
 * @param dev: USB Device handle reference
 * @return OK if the descriptor is provided, INVALID if not supported
 */
USBD_ReturnType USBD_GetMsDescriptor(USBD_HandleType *dev)
{
    USBD_ReturnType retval = USBD_E_INVALID;
    uint8_t *data = dev->CtrlData;
    uint16_t len;

    if (dev->Setup.Index == USB_MS_OS_2p0_GET_DESCRIPTOR_INDEX)
    {
        len = USBD_MsOs2p0Desc(dev, data);

        /* Transfer the non-null descriptor */
        if (len > 0)
        {
            retval = USBD_CtrlSendData(dev, data, len);
        }
    }

    return retval;
}

#endif /* USBD_MS_OS_DESC_VERSION */

/** @} */

#endif /* (USBD_MS_OS_DESC_VERSION > 0) */

