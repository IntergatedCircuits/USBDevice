/**
  ******************************************************************************
  * @file    usbd.c
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   Universal Serial Bus Device Driver
  *          Device level control functions
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
#include <usbd_private.h>

/** @addtogroup USBD
 * @{ */

/** @defgroup USBD_Exported_Functions USBD Exported Functions
 *  @brief This group is called directly by the Application or by the Hardware Abstraction Layer.
 * @{ */

/**
 * @brief This function initializes the USB device.
 * @param dev: USB Device handle reference
 * @param desc: Device properties reference
 */
void USBD_Init(USBD_HandleType *dev, const USBD_DescriptionType *desc)
{
    /* Assign USBD Descriptors */
    dev->Desc = desc;

    /* Set Device initial State */
    dev->ConfigSelector = 0;
    dev->Features.RemoteWakeup = 0;
    dev->Features.SelfPowered  = dev->Desc->Config.SelfPowered;

    /* For FS device some buffer space can be saved by changing
     * EP0 MPS to 32/16/8
     * HS capable devices must keep this value at 64 */
    dev->EP.IN [0].MaxPacketSize = USB_EP0_FS_MAX_PACKET_SIZE;
    dev->EP.OUT[0].MaxPacketSize = USB_EP0_FS_MAX_PACKET_SIZE;

    /* Initialize low level driver with device configuration */
    USBD_PD_Init(dev, &dev->Desc->Config);
}

/**
 * @brief This function shuts down the USB device entirely.
 * @param dev: USB Device handle reference
 */
void USBD_Deinit(USBD_HandleType *dev)
{
    USBD_IfConfig(dev, 0);

    /* Deinitialize low level driver */
    USBD_PD_Deinit(dev);
}

/**
 * @brief Resets the USBD handle to the initial empty state.
 * @param dev: USB Device handle reference
 */
void USBD_HandleReset(USBD_HandleType *dev)
{
    int i;

    dev->Desc = NULL;
    dev->IfCount = 0;

    for (i = 0; i < USBD_MAX_EP_COUNT; i++)
    {
        dev->EP.IN [i].MaxPacketSize = 0;
        dev->EP.IN [i].State = USB_EP_STATE_CLOSED;
        dev->EP.OUT[i].MaxPacketSize = 0;
        dev->EP.OUT[i].State = USB_EP_STATE_CLOSED;
    }
}

/**
 * @brief This function logically connects the device to the bus.
 * @param dev: USB Device handle reference
 */
void USBD_Connect(USBD_HandleType *dev)
{
    /* Start the low level driver */
    USBD_PD_Start(dev);
}

/**
 * @brief This function logically disconnects the device from the bus.
 * @param dev: USB Device handle reference
 */
void USBD_Disconnect(USBD_HandleType *dev)
{
    USBD_IfConfig(dev, 0);

    /* Stop the low level driver */
    USBD_PD_Stop(dev);
}

/**
 * @brief This function sends remote wakeup signal to the host
 *        as long as the feature is enabled.
 * @param dev: USB Device handle reference
 * @return OK if the feature is enabled, ERROR otherwise
 */
USBD_ReturnType USBD_RemoteWakeup(USBD_HandleType *dev)
{
    USBD_ReturnType retval = USBD_E_ERROR;

    /* Check if remote wakeup is enabled by the host */
    if (dev->Features.RemoteWakeup != 0)
    {
        USBD_PD_RemoteWakeup(dev);
        retval = USBD_E_OK;
    }
    return retval;
}

/**
 * @brief This function performs the necessary actions upon receiving Reset signal:
 *         - Opens the control endpoint 0
 *         - Resets any previously active configuration
 * @param dev: USB Device handle reference
 * @param speed: The new device speed
 */
void USBD_ResetCallback(USBD_HandleType *dev, USB_SpeedType speed)
{
    dev->Speed = speed;

    /* Open EP0 */
    USBD_PD_CtrlEpOpen(dev, dev->EP.OUT[0].MaxPacketSize);
    dev->EP.OUT[0].State = USB_EP_STATE_IDLE;

    /* Reset any previous configuration */
    USBD_IfConfig(dev, 0);
}

/** @} */

/** @addtogroup USBD_Private_Functions_Req
 * @{ */

/**
 * @brief This function checks the SET_ADDRESS request's validity.
 * @param dev: USB Device handle reference: USB Device handle reference
 * @return OK if the request is accepted, INVALID otherwise
 */
static USBD_ReturnType USBD_SetAddress(USBD_HandleType *dev)
{
    USBD_ReturnType retval = USBD_E_INVALID;

    /* The request is only valid when not configured yet */
    if ((dev->Setup.Index    == 0) &&
        (dev->Setup.Length   == 0) &&
        (dev->ConfigSelector == 0))
    {
        /* Address is accepted, it will be applied
         * after this transfer is complete */
        retval = USBD_E_OK;
    }
    return retval;
}

/**
 * @brief This function switches to the requested configuration.
 * @param dev: USB Device handle reference
 * @return OK if the configuration is available, INVALID otherwise
 */
static USBD_ReturnType USBD_SetConfig(USBD_HandleType *dev)
{
    USBD_ReturnType retval = USBD_E_INVALID;
    uint8_t cfgNum = (uint8_t)dev->Setup.Value;

    if (cfgNum <= USBD_MAX_CONFIGURATION_COUNT)
    {
        USBD_IfConfig(dev, cfgNum);

        retval = USBD_E_OK;
    }
    return retval;
}

/**
 * @brief This function sends the current configuration index on the control endpoint.
 * @param dev: USB Device handle reference
 * @return Always OK
 */
static USBD_ReturnType USBD_GetConfig(USBD_HandleType *dev)
{
    return USBD_CtrlSendData(dev,
            (uint8_t*)&dev->ConfigSelector,
            sizeof(dev->ConfigSelector));
}

/**
 * @brief This function sends the device feature status on the control endpoint.
 * @param dev: USB Device handle reference
 * @return Always OK
 */
static USBD_ReturnType USBD_GetStatus(USBD_HandleType *dev)
{
    return USBD_CtrlSendData(dev,
            (uint8_t*)&dev->Features.w,
            sizeof(dev->Features.w));
}

/**
 * @brief This function enables the remote wakeup feature (if it's selected).
 * @param dev: USB Device handle reference
 * @return OK if the feature is supported, INVALID otherwise
 */
static USBD_ReturnType USBD_SetFeature(USBD_HandleType *dev)
{
    USBD_ReturnType retval = USBD_E_INVALID;

    /* The only settable std device feature */
    if (dev->Setup.Value == USB_FEATURE_REMOTE_WAKEUP)
    {
        dev->Features.RemoteWakeup = 1;
        retval = USBD_E_OK;
    }
    return retval;
}

/**
 * @brief This function clears the remote wakeup feature (if it's selected).
 * @param dev: USB Device handle reference
 * @return OK if the feature is supported, INVALID otherwise
 */
static USBD_ReturnType USBD_ClearFeature(USBD_HandleType *dev)
{
    USBD_ReturnType retval = USBD_E_INVALID;

    /* The only settable std device feature */
    if (dev->Setup.Value == USB_FEATURE_REMOTE_WAKEUP)
    {
        dev->Features.RemoteWakeup = 0;
        retval = USBD_E_OK;
    }
    return retval;
}

/**
 * @brief This function handles standard device requests.
 * @param dev: USB Device handle reference
 * @return OK if the request is processed, INVALID if not supported
 */
USBD_ReturnType USBD_DevRequest(USBD_HandleType *dev)
{
    USBD_ReturnType retval = USBD_E_INVALID;

    /* On device level only (the below) standard requests are supported */
    if (dev->Setup.RequestType.Type == USB_REQ_TYPE_STANDARD)
    {
        switch (dev->Setup.Request)
        {
            case USB_REQ_GET_DESCRIPTOR:
                retval = USBD_GetDescriptor(dev);
                break;

            case USB_REQ_SET_ADDRESS:
                retval = USBD_SetAddress(dev);
                break;

            case USB_REQ_SET_CONFIGURATION:
                retval = USBD_SetConfig(dev);
                break;

            case USB_REQ_GET_CONFIGURATION:
                retval = USBD_GetConfig(dev);
                break;

            case USB_REQ_GET_STATUS:
                retval = USBD_GetStatus(dev);
                break;

            case USB_REQ_SET_FEATURE:
                retval = USBD_SetFeature(dev);
                break;

            case USB_REQ_CLEAR_FEATURE:
                retval = USBD_ClearFeature(dev);
                break;

            default:
                break;
        }
    }
    return retval;
}

/** @} */

/** @} */
