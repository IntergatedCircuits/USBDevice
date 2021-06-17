/**
  ******************************************************************************
  * @file    usbd_ctrl.c
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   Universal Serial Bus Device Driver
  *          Control transfer management
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

/** @ingroup USBD_Private
 * @defgroup USBD_Private_Functions_Ctrl USBD Control Request Handling
 * @{ */

/**
 * @brief This function indicates a failed control request by stalling EP0.
 * @param dev: USB Device handle reference
 */
static void USBD_CtrlSendError(USBD_HandleType *dev)
{
    USBD_PD_EpSetStall(dev, 0x80);
    dev->EP.IN [0].State = USB_EP_STATE_STALL;
    USBD_PD_EpSetStall(dev, 0x00);
    dev->EP.OUT[0].State = USB_EP_STATE_STALL;
}

/**
 * @brief This function indicates a successful control data OUT stage by sending ZLP on EP0.
 * @param dev: USB Device handle reference
 */
static void USBD_CtrlSendStatus(USBD_HandleType *dev)
{
    dev->EP.IN[0].State = USB_EP_STATE_STATUS;
    USBD_PD_EpSend(dev, 0x80, NULL, 0);
}

/**
 * @brief This function indicates a successful control data IN stage by receiving ZLP on EP0.
 * @param dev: USB Device handle reference
 */
static void USBD_CtrlReceiveStatus(USBD_HandleType *dev)
{
    dev->EP.OUT[0].State = USB_EP_STATE_STATUS;
    USBD_PD_EpReceive(dev, 0x00, NULL, 0);
}

/**
 * @brief This function manages the end of a control IN endpoint transfer:
 *         - Send Zero Length Packet if the end of the transfer is ambiguous
 *         - Provide completion callback and OUT status stage if it was a data stage
 *         - Set device address if it was requested
 * @param dev: USB Device handle reference
 */
void USBD_CtrlInCallback(USBD_HandleType *dev)
{
    /* Last packet is MPS multiple, so send ZLP packet */
    if (( dev->EP.IN[0].Transfer.Length <  dev->Setup.Length) &&
        ( dev->EP.IN[0].Transfer.Length >= dev->EP.IN[0].MaxPacketSize) &&
        ((dev->EP.IN[0].Transfer.Length & (dev->EP.IN[0].MaxPacketSize - 1)) == 0))
    {
        USBD_PD_EpSend(dev, 0x80, NULL, 0);
    }
    else
    {
        dev->EP.IN[0].State = USB_EP_STATE_IDLE;

        /* If the callback is from a Data stage */
        if (dev->Setup.RequestType.Direction == USB_DIRECTION_IN)
        {
            /* Only call back if the IF was serving the request */
            if ((dev->ConfigSelector != 0) &&
                (dev->Setup.RequestType.Recipient == USB_REQ_RECIPIENT_INTERFACE))
            {
                /* If callback for transmitted EP0 data */
                USBD_IfClass_DataStage(dev->IF[(uint8_t)dev->Setup.Index]);
            }

            /* Proceed to Status stage */
            USBD_CtrlReceiveStatus(dev);
        }
#if (USBD_SET_ADDRESS_IMMEDIATE != 1)
        /* If the address was set by the last request, apply it now */
        else if ((dev->Setup.RequestType.b == 0x00) &&
                 (dev->Setup.Request == USB_REQ_SET_ADDRESS))
        {
            USBD_PD_SetAddress(dev, dev->Setup.Value & 0x7F);
        }
#endif
    }
}

/**
 * @brief This function manages the end of a control OUT endpoint transfer:
 *         - Provide completion callback and IN status stage if it was a data stage
 * @param dev: USB Device handle reference
 */
void USBD_CtrlOutCallback(USBD_HandleType *dev)
{
    /* If the callback is from a Data stage */
    if ((dev->Setup.Length > 0) &&
        (dev->Setup.RequestType.Direction == USB_DIRECTION_OUT))
    {
        /* Standard requests have no OUT direction data stage -> must be IF related */
        if (dev->ConfigSelector != 0)
        {
            /* If callback for received EP0 data */
            USBD_IfClass_DataStage(dev->IF[(uint8_t)dev->Setup.Index]);
        }

        /* Proceed to Status stage */
        USBD_CtrlSendStatus(dev);
    }
}

/** @} */

/** @addtogroup USBD_Internal_Functions
 * @{ */

/**
 * @brief This function sends data through the control endpoint in response to a setup request.
 * @param dev: USB Device handle reference
 * @param data: pointer to the data to send
 * @param len: length of the data
 * @return OK if called from the right context, ERROR otherwise
 */
USBD_ReturnType USBD_CtrlSendData(USBD_HandleType *dev, void *data, uint16_t len)
{
    USBD_ReturnType retval = USBD_E_ERROR;

    /* Sanity check */
    if ((dev->Setup.RequestType.Direction == USB_DIRECTION_IN) &&
        (dev->EP.OUT[0].State == USB_EP_STATE_SETUP))
    {
        /* Don't send more bytes than requested */
        if (dev->Setup.Length < len)
        {
            len = dev->Setup.Length;
        }

        dev->EP.IN[0].State = USB_EP_STATE_DATA;
        USBD_PD_EpSend(dev, 0x80, (const uint8_t*)data, len);

        retval = USBD_E_OK;
    }
    return retval;
}

/**
 * @brief This function receives control data according to the setup request.
 * @param dev: USB Device handle reference
 * @param data: pointer to the target buffer to receive to
 * @param len: maximum allowed length of the data
 * @return OK if called from the right context, ERROR otherwise
 */
USBD_ReturnType USBD_CtrlReceiveData(USBD_HandleType *dev, void *data, uint16_t len)
{
    USBD_ReturnType retval = USBD_E_ERROR;

    /* Sanity check */
    if ((dev->Setup.RequestType.Direction == USB_DIRECTION_OUT) &&
        (dev->EP.OUT[0].State == USB_EP_STATE_SETUP))
    {
        /* Don't receive more bytes than requested */
        if (dev->Setup.Length < len)
        {
            len = dev->Setup.Length;
        }

        dev->EP.OUT[0].State = USB_EP_STATE_DATA;
        USBD_PD_EpReceive(dev, 0x00, (uint8_t*)data, len);

        retval = USBD_E_OK;
    }
    return retval;
}

/** @} */

/** @addtogroup USBD_Exported_Functions
 * @{ */

/**
 * @brief This function routes the setup request depending on the recipient
 *        and performs the endpoint's status stage if no data stage is requested
 *        or the request wasn't accepted.
 * @param dev: USB Device handle reference
 */
void USBD_SetupCallback(USBD_HandleType *dev)
{
    USBD_ReturnType retval = USBD_E_INVALID;

    dev->EP.OUT[0].State = USB_EP_STATE_SETUP;

    /* Route the request to the recipient */
    switch (dev->Setup.RequestType.Recipient)
    {
        case USB_REQ_RECIPIENT_DEVICE:
            retval = USBD_DevRequest(dev);
            break;

        case USB_REQ_RECIPIENT_INTERFACE:
            retval = USBD_IfRequest(dev);
            break;

        case USB_REQ_RECIPIENT_ENDPOINT:
            retval = USBD_EpRequest(dev);
            break;

        default:
            break;
    }

    /* If the request was rejected, send Request Error (EP0 STALL) */
    if (retval != USBD_E_OK)
    {
        USBD_CtrlSendError(dev);
    }
    /* If the wLength is 0, there is no Data stage,
     * send positive status (EP0 ZLP) */
    else if (dev->Setup.Length == 0)
    {
        USBD_CtrlSendStatus(dev);
    }
    else
    {
        /* Data stage starts in the requested direction */
    }
}

/** @} */
