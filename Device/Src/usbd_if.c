/**
  ******************************************************************************
  * @file    usbd_if.c
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   Universal Serial Bus Device Driver
  *          Device interface management
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
#define USBD_DEVICE_API
#include <usbd_private.h>

/** @ingroup USBD
 * @defgroup USBD_Private_Functions_IfClass USBD Class-specific Interface Callouts
 * @brief These functions simply call the class-specific function pointer
 * @{ */

/**
 * @brief Calls the interface's class specific
 *        @ref USBD_ClassType::Init function.
 * @param itf: reference of the interface
 */
__STATIC_INLINE
void USBD_IfClass_Init(USBD_IfHandleType *itf)
{
    USBD_SAFE_CALLBACK(itf->Class->Init, itf);
}

/**
 * @brief Calls the interface's class specific
 *        @ref USBD_ClassType::Deinit function.
 * @param itf: reference of the interface
 */
__STATIC_INLINE
void USBD_IfClass_Deinit(USBD_IfHandleType *itf)
{
    USBD_SAFE_CALLBACK(itf->Class->Deinit, itf);
}

/**
 * @brief Calls the interface's class specific
 *        @ref USBD_ClassType::SetupStage function.
 * @param itf: reference of the interface
 * @return Return value of the function call
 */
__STATIC_INLINE
USBD_ReturnType USBD_IfClass_SetupStage(USBD_IfHandleType *itf)
{
    if (itf->Class->SetupStage == NULL)
        { return USBD_E_INVALID; }
    else
        { return itf->Class->SetupStage(itf); }
}

/**
 * @brief Calls the interface's class specific
 *        @ref USBD_ClassType::GetString function.
 * @param itf:    reference of the interface
 * @param intNum: the interface-internal string index
 * @return String reference
 */
__STATIC_INLINE
const char* USBD_IfClass_GetString(USBD_IfHandleType *itf, uint8_t intNum)
{
    if (itf->Class->GetString == NULL)
        { return (const char*)NULL; }
    else
        { return itf->Class->GetString(itf, intNum); }
}

/** @} */

/** @ingroup USBD
 * @defgroup USBD_Private_Functions_If USBD Interface Management
 * @{ */

/**
 * @brief This function changes the active device configuration.
 * @param dev: USB Device handle reference
 * @param cfgNum: New configuration selector index
 */
void USBD_IfConfig(USBD_HandleType *dev, uint8_t cfgNum)
{
    if (dev->ConfigSelector != cfgNum)
    {
        uint8_t ifNum;

        /* Clear any previously selected config */
        if (dev->ConfigSelector != 0)
        {
            for (ifNum = 0; ifNum < dev->IfCount; ifNum++)
            {
                USBD_IfClass_Deinit(dev->IF[ifNum]);
            }
        }

        /* Update configuration index */
        dev->ConfigSelector = cfgNum;

        /* Set the new selected valid config */
        if (dev->ConfigSelector != 0)
        {
            for (ifNum = 0; ifNum < dev->IfCount; ifNum++)
            {
                USBD_IfClass_Init(dev->IF[ifNum]);
            }
        }
    }
}

/** @} */

/** @addtogroup USBD_Private_Functions_Desc
 * @{ */

/**
 * @brief This function provides the string of the interface
 *        which is selected by the setup request.
 * @param dev: USB Device handle reference
 * @return Reference to the interface's string, or NULL if not available
 */
const char* USBD_IfString(USBD_HandleType *dev)
{
    uint8_t ifNum  = ((uint8_t)dev->Setup.Value & 0xF) - USBD_ISTR_INTERFACES;
    uint8_t intNum = ((uint8_t)dev->Setup.Value >> 4);
    USBD_IfHandleType *itf = dev->IF[ifNum];
    const char* str = NULL;

    if (ifNum < dev->IfCount)
    {
        str = USBD_IfClass_GetString(itf, intNum);
    }

    return str;
}

/** @} */

/** @addtogroup USBD_Private_Functions_Ctrl
 * @{ */

/**
 * @brief Processes the interface request.
 * @param dev: USB Device handle reference
 * @return OK if the request is processed, INVALID if not supported
 */
USBD_ReturnType USBD_IfRequest(USBD_HandleType *dev)
{
    USBD_ReturnType retval = USBD_E_INVALID;
    uint8_t ifNum = (uint8_t)dev->Setup.Index;
    USBD_IfHandleType *itf = dev->IF[ifNum];

    if ((dev->ConfigSelector == 0) || (ifNum >= dev->IfCount))
    {
        /* Configured and valid indexed interfaces only */
    }
    else if (dev->Setup.RequestType.Type == USB_REQ_TYPE_STANDARD)
    {
        switch (dev->Setup.Request)
        {
            /* Current alternate setting of the IF */
            case USB_REQ_GET_INTERFACE:
            {
                retval = USBD_CtrlSendData(dev, &itf->AltSelector,
                        sizeof(itf->AltSelector));
                break;
            }

            /* Set alternate setting of the IF */
            case USB_REQ_SET_INTERFACE:
            {
                uint8_t altSel = (uint8_t)dev->Setup.Value;

                /* Check validity */
                if (itf->AltCount > altSel)
                {
                    /* Deinit previous AS */
                    USBD_IfClass_Deinit(itf);

                    itf->AltSelector = altSel;

                    /* Init with new AS */
                    USBD_IfClass_Init(itf);

                    retval = USBD_E_OK;
                }
                break;
            }
#if 0
            case USB_REQ_GET_DESCRIPTOR:
            {
                uint8_t *data = dev->CtrlData;
                uint16_t i, len = USBD_IfClass_GetDesc(itf, ifNum, data);

                /* Try to find desc in default descriptor */
                for (i = 0; i < len; i += data[i])
                {
                    if (data[i + 1] == (dev->Setup.Value >> 8))
                    {
                        retval = USBD_CtrlSendData(dev, &data[i], data[i]);
                        break;
                    }
                }

                if (i == len)
                {
                    /* forward the request to the IF */
                    retval = USBD_IfClass_SetupStage(itf);
                }
                break;
            }
#endif
            default:
            {
                /* forward the request to the IF */
                retval = USBD_IfClass_SetupStage(itf);
                break;
            }
        }
    }
    else
    {
        /* forward the request to the IF */
        retval = USBD_IfClass_SetupStage(itf);
    }

    return retval;
}

/** @} */
