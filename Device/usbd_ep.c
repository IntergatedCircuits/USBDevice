/**
  ******************************************************************************
  * @file    usbd_ep.c
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   Universal Serial Bus Device Driver
  *          Endpoint control functions
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
 * @defgroup USBD_Internal_Functions USB Device Internal Functions
 * @brief This group is used by the Device and the Classes.
 * @{ */

/**
 * @brief This function sends data through the selected IN endpoint.
 * @param dev: USB Device handle reference
 * @param epAddr: endpoint address
 * @param data: pointer to the data to send
 * @param len: length of the data
 * @return BUSY if the endpoint isn't idle, OK if successful
 */
USBD_ReturnType USBD_EpSend(USBD_HandleType *dev, uint8_t epAddr,
        void *data, uint16_t len)
{
    USBD_ReturnType retval = USBD_E_BUSY;
    USBD_EpHandleType *ep = &dev->EP.IN[epAddr & 0xF];

    if ((ep->State == USB_EP_STATE_IDLE) ||
        (ep->Type  == USB_EP_TYPE_ISOCHRONOUS))
    {
        /* Set EP transfer data */
        ep->State = USB_EP_STATE_DATA;
        USBD_PD_EpSend(dev, epAddr, (const uint8_t*)data, len);

        retval = USBD_E_OK;
    }

    return retval;
}

/**
 * @brief This function prepares data reception through the selected OUT endpoint.
 * @param dev: USB Device handle reference
 * @param epAddr: endpoint address
 * @param data: pointer to the target buffer to receive to
 * @param len: maximum length of the data
 * @return BUSY if the endpoint isn't idle, OK if successful
 */
USBD_ReturnType USBD_EpReceive(USBD_HandleType *dev, uint8_t epAddr,
        void *data, uint16_t len)
{
    USBD_ReturnType retval = USBD_E_BUSY;
    USBD_EpHandleType *ep = &dev->EP.OUT[epAddr];

    if ((ep->State == USB_EP_STATE_IDLE) ||
        (ep->Type  == USB_EP_TYPE_ISOCHRONOUS))
    {
        /* Set EP transfer data */
        ep->State = USB_EP_STATE_DATA;
        USBD_PD_EpReceive(dev, epAddr, (uint8_t*)data, len);

        retval = USBD_E_OK;
    }

    return retval;
}

/** @} */

/** @addtogroup USBD_Exported_Functions
 * @{ */

/**
 * @brief This function notifies the appropriate higher layer
 *        of the completion of an IN endpoint transfer.
 * @param dev: USB Device handle reference
 * @param ep: USB IN endpoint handle reference
 */
void USBD_EpInCallback(USBD_HandleType *dev, USBD_EpHandleType *ep)
{
    if (ep == &dev->EP.IN[0])
    {
        USBD_CtrlInCallback(dev);
    }
    else
    {
        ep->State = USB_EP_STATE_IDLE;
        USBD_IfClass_InData(dev->IF[ep->IfNum], ep);
    }
}

/**
 * @brief This function notifies the appropriate higher layer
 *        of the completion of an OUT endpoint transfer.
 * @param dev: USB Device handle reference
 * @param ep: USB OUT endpoint handle reference
 */
void USBD_EpOutCallback(USBD_HandleType *dev, USBD_EpHandleType *ep)
{
    ep->State = USB_EP_STATE_IDLE;

    if (ep == &dev->EP.OUT[0])
    {
        USBD_CtrlOutCallback(dev);
    }
    else
    {
        USBD_IfClass_OutData(dev->IF[ep->IfNum], ep);
    }
}

/** @} */

/** @addtogroup USBD_Private_Functions_Ctrl
 * @{ */

/**
 * @brief This function handles standard endpoint requests.
 * @param dev: USB Device handle reference
 * @return OK if the request is processed, INVALID if not supported
 */
USBD_ReturnType USBD_EpRequest(USBD_HandleType *dev)
{
    USBD_ReturnType retval = USBD_E_INVALID;
    uint8_t epAddr = (uint8_t)dev->Setup.Index, epNum = epAddr & 0xF;

    if ((epNum >= USBD_MAX_EP_COUNT) ||
        (epNum == 0) ||
        (dev->ConfigSelector == 0))
    {
    }
    else if (dev->Setup.RequestType.Type == USB_REQ_TYPE_STANDARD)
    {
        USBD_EpHandleType *ep = USBD_EpAddr2Ref(dev, epAddr);

        switch (dev->Setup.Request)
        {
            /* EP halt is the only standard feature */
            case USB_REQ_SET_FEATURE:
            {
                if (dev->Setup.Value == USB_FEATURE_EP_HALT)
                {
                    retval = USBD_E_OK;

                    if (ep->State != USB_EP_STATE_STALL)
                    {
                        USBD_PD_EpSetStall(dev, epAddr);
                        ep->State = USB_EP_STATE_STALL;
                    }
                }
                break;
            }

            case USB_REQ_CLEAR_FEATURE:
            {
                if (dev->Setup.Value == USB_FEATURE_EP_HALT)
                {
                    retval = USBD_E_OK;

                    if (ep->State == USB_EP_STATE_STALL)
                    {
                        USBD_PD_EpClearStall(dev, epAddr);
                        ep->State = USB_EP_STATE_IDLE;

                        ep->Transfer.Length = 0;
                        /* Workaround: notify interface of ready endpoint
                         * by completion callback with 0 length */
                        if (epAddr != epNum)
                        {
                            USBD_IfClass_InData(dev->IF[ep->IfNum], ep);
                        }
                        else
                        {
                            USBD_IfClass_OutData(dev->IF[ep->IfNum], ep);
                        }
                    }
                }
                break;
            }

            /* Report the halt (stall) status of the EP */
            case USB_REQ_GET_STATUS:
            {
                uint16_t *epStatus = (uint16_t*)dev->CtrlData;

                *epStatus = (ep->State == USB_EP_STATE_STALL) ?
                        1 << USB_FEATURE_EP_HALT : 0;

                retval = USBD_CtrlSendData(dev, epStatus, sizeof(*epStatus));
                break;
            }

            default:
                break;
        }
    }
    else
    {
        /* Callouts for class or vendor specific
         * EP request processing not implemented
         * Interface level requests are promoted (and supported) instead */
    }

    return retval;
}

/** @} */
