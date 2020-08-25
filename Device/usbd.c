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
#include <private/usbd_private.h>

/** @ingroup USBD
 * @defgroup USBD_Exported_Functions USB Device Exported Functions
 * @brief These functions are called by the user code
 *        (the callbacks by the Peripheral Driver).
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
 * @brief Removes all mounted interfaces from the device.
 * @param dev: USB Device handle reference
 */
void USBD_UnmountInterfaces(USBD_HandleType *dev)
{
    int i;

    USBD_IfConfig(dev, 0);

    dev->IfCount = 0;

    for (i = 1; i < USBD_MAX_EP_COUNT; i++)
    {
        dev->EP.IN [i].MaxPacketSize = 0;
        dev->EP.IN [i].State = USB_EP_STATE_CLOSED;
        dev->EP.OUT[i].MaxPacketSize = 0;
        dev->EP.OUT[i].State = USB_EP_STATE_CLOSED;
    }
}

/**
 * @brief This function logically connects (attaches) the device to the bus.
 * @param dev: USB Device handle reference
 */
void USBD_Connect(USBD_HandleType *dev)
{
    /* Start the low level driver */
    USBD_PD_Start(dev);
}

/**
 * @brief This function logically disconnects (detaches) the device from the bus.
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
USBD_ReturnType USBD_SetRemoteWakeup(USBD_HandleType *dev)
{
    USBD_ReturnType retval = USBD_E_ERROR;

    /* Check if remote wakeup is enabled by the host */
    if (dev->Features.RemoteWakeup != 0)
    {
        USBD_PD_SetRemoteWakeup(dev);
        retval = USBD_E_OK;
    }
    return retval;
}

/**
 * @brief This function shall be called 1 - 15 ms after setting RemoteWakeup
 *        in @ref USB_LinkStateType::USB_LINK_STATE_SUSPEND state.
 *        The RemoteWakeup signal is cleared by hardware when exiting from
 *        @ref USB_LinkStateType::USB_LINK_STATE_SLEEP state.
 * @param dev: USB Device handle reference
 * @return OK if the feature is enabled, ERROR otherwise
 */
USBD_ReturnType USBD_ClearRemoteWakeup(USBD_HandleType *dev)
{
    USBD_ReturnType retval = USBD_E_ERROR;

    /* Check if remote wakeup is enabled by the host */
    if (dev->Features.RemoteWakeup != 0)
    {
        USBD_PD_ClearRemoteWakeup(dev);
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

    /* Reset any previous configuration */
    USBD_IfConfig(dev, 0);

#if (USBD_HS_SUPPORT == 1)
    /* Limit packet sizes according to what the current speed allows */
    if (speed == USB_SPEED_FULL)
    {
        USBD_EpHandleType *ep;

        /* Note on condition: EP0 doesn't require this adjustment */
        for (ep = &dev->EP.OUT[USBD_MAX_EP_COUNT - 1]; ep > &dev->EP.IN[0]; ep--)
        {
            if (ep->Type == USB_EP_TYPE_ISOCHRONOUS)
            {
                /* An FS frame is 1 ms, 8 times as long as a HS microframe
                 * To keep the data rate the same, each transfer has to be
                 * 8 times larger */
                ep->MaxPacketSize *= 8;
                /* TODO if the result is higher than USB_EP_ISOC_FS_MPS,
                 * the interface cannot function properly */
            }
            /* Other types have equal FS MPS limits */
            else if (ep->MaxPacketSize > USB_EP_CTRL_FS_MPS)
            {
                ep->MaxPacketSize = USB_EP_CTRL_FS_MPS;
            }
        }
    }
#endif

    /* Open control endpoint to start data transfers */
    USBD_PD_CtrlEpOpen(dev);
    dev->EP.OUT[0].State = USB_EP_STATE_IDLE;
}

/** @} */

/** @ingroup USBD
 * @defgroup USBD_Private USB Device Private Functions
 */

/** @ingroup USBD_Private
 * @addtogroup USBD_Private_Functions_Ctrl
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
#if (USBD_SET_ADDRESS_IMMEDIATE == 1)
        USBD_PD_SetAddress(dev, dev->Setup.Value & 0x7F);
#endif
        /* Address is accepted, it will be applied
         * after this Ctrl transfer is complete */
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
    dev->CtrlData[0] = dev->ConfigSelector;
    return USBD_CtrlSendData(dev, dev->CtrlData, sizeof(dev->ConfigSelector));
}

/**
 * @brief This function sends the device feature status on the control endpoint.
 * @param dev: USB Device handle reference
 * @return Always OK
 */
static USBD_ReturnType USBD_GetStatus(USBD_HandleType *dev)
{
    uint16_t *devStatus = (uint16_t*)dev->CtrlData;
    *devStatus = dev->Features.w;
    return USBD_CtrlSendData(dev, devStatus, sizeof(*devStatus));
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
#if (USBD_MS_OS_DESC_VERSION > 0)
    else if (dev->Setup.RequestType.Type == USB_REQ_TYPE_VENDOR)
    {
        switch (dev->Setup.Request)
        {
            case USB_REQ_MICROSOFT_OS:
                if (dev->Setup.RequestType.Direction == USB_DIRECTION_IN)
                {
                    retval = USBD_GetMsDescriptor(dev);
                }
#if (USBD_MS_OS_DESC_VERSION == 2)
                else
                {
                    if (dev->Setup.Index == USB_MS_OS_2p0_SET_ALT_ENUMERATION_INDEX)
                    {
                        /* MS OS 2.0 set alternate enumeration
                         * wValue high byte = bAltEnumCode */
                    }
                }
#endif /* (USBD_MS_OS_DESC_VERSION == 2) */
                break;
        }
    }
#endif /* (USBD_MS_OS_DESC_VERSION > 0) */
    return retval;
}

/** @} */
