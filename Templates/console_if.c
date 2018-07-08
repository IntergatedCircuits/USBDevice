/**
  ******************************************************************************
  * @file    console_if.c
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-07-08
  * @brief   USB serial console interface template
  *
  * @details
  * This CDC interface can be used to channel standard I/O operations through
  * a virtual serial port:
  *     @code
  *     #include <usbd_cdc.h>
  *     extern USBD_CDC_IfHandleType *const console_if;
  *     @endcode
  * After configuring it's endpoint numbers it can be mounted on a USB device.
  * Define PRINT_BUFFER_SIZE with an appropriate buffer size to enable output,
  * SCAN_BUFFER_SIZE to enable input functionality. (Twice the max packet size
  * is recommended.)
  * The interface becomes operational after the serial port's line coding is set
  * (with any standard baudrate value).
  *
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
#include <usbd_cdc.h>
#include <queues.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>

#undef errno
extern int32_t errno;

/** @defgroup console_if USB serial console interface template
 * @{ */

static void console_if_init         (void);
static void console_if_ctrl         (USB_SetupRequestType * req, uint8_t* pbuf);

#if (PRINT_BUFFER_SIZE > 0)
static void console_if_in_cmplt     (uint8_t * pbuf, uint16_t length);
static void console_if_send         (void);
QUEUE_DEF(console_if_IN, uint8_t, PRINT_BUFFER_SIZE);
#endif

#if (SCAN_BUFFER_SIZE > 0)
static void console_if_out_cmplt    (uint8_t * pbuf, uint16_t length);
static void console_if_recv         (void);
QUEUE_DEF(console_if_OUT, uint8_t, SCAN_BUFFER_SIZE);
#endif

static const USBD_CDC_AppType console_app =
{
    .Name           = "Serial port as debug console",
    .Init           = console_if_init,
    .Deinit         = console_if_init,
    .Control        = console_if_ctrl,
#if (SCAN_BUFFER_SIZE > 0)
    .Received       = console_if_out_cmplt,
#endif
#if (PRINT_BUFFER_SIZE > 0)
    .Transmitted    = console_if_in_cmplt,
#endif
};

USBD_CDC_IfHandleType _console_if = {
    .App = &console_app,
    .Base.AltCount = 1,
    .Config.Protocol = 0xFF, /* Vendor-specific protocol */
}, *const console_if = &_console_if;

USBD_CDC_LineCodingType line_coding = {
        .DTERate  = ~0, /* init to invalid in order to detect serial port connection */
        .DataBits = 8,
};

static void console_if_init(void)
{
    line_coding.DTERate = ~0;
#if (PRINT_BUFFER_SIZE > 0)
    console_if_IN.head = console_if_IN.tail = 0;
#endif
#if (SCAN_BUFFER_SIZE > 0)
    console_if_OUT.head = console_if_OUT.tail = 0;
#endif
}

static void console_if_ctrl(USB_SetupRequestType * req, uint8_t* pbuf)
{
    if (req->Request == CDC_REQ_GET_LINE_CODING)
    {   memcpy(pbuf, &line_coding, sizeof(line_coding)); }
    else if (req->Request == CDC_REQ_SET_LINE_CODING)
    {   memcpy(&line_coding, pbuf, sizeof(line_coding)); }
}

#if (PRINT_BUFFER_SIZE > 0)
static void console_if_in_cmplt(uint8_t * pbuf, uint16_t length)
{
    console_if_send();
}

static void console_if_send(void)
{
    if (!QUEUE_EMPTY(console_if_IN))
    {
        uint32_t newtail;
        uint32_t length;

        /* If the head is ahead, transmit the new data */
        if (console_if_IN.tail < console_if_IN.head)
        {
            length  = console_if_IN.head - console_if_IN.tail;
            newtail = console_if_IN.head;
        }
        /* If the USB IN index is ahead, the buffer has wrapped,
         * transmit until the end */
        else
        {
            length  = console_if_IN.size - console_if_IN.tail;
            newtail = 0;
        }

        if (USBD_E_OK == USBD_CDC_Transmit(console_if,
                &console_if_IN.buffer[console_if_IN.tail + 1], length))
        {
            console_if_IN.tail = newtail;
        }
    }
}

int _write(int32_t file, uint8_t *ptr, int32_t len)
{
    int retval = -1;
    if (line_coding.DTERate == (~0))
    {
        errno = EIO;
    }
    else if (QUEUE_SPACE(console_if_IN) < len)
    {
        errno = ENOMEM;
    }
    else
    {
        QUEUE_PUT_ARRAY(console_if_IN, ptr, len);

        console_if_send();
        retval = len;
    }
    return retval;
}
#endif

#if (SCAN_BUFFER_SIZE > 0)
static void console_if_out_cmplt(uint8_t * pbuf, uint16_t length)
{
    console_if_OUT.head += length;
    console_if_recv();
}

static void console_if_recv(void)
{
    if (!QUEUE_FULL(console_if_OUT))
    {
        uint32_t length;

        if (console_if_OUT.tail > console_if_OUT.head)
        {
            length = console_if_OUT.tail - console_if_OUT.head - 1;
        }
        else
        {
            length = console_if_OUT.size - console_if_OUT.head;
        }

        USBD_CDC_Transmit(console_if,
                &console_if_OUT.buffer[console_if_OUT.head + 1], length);
    }
}

int _read(int32_t file, uint8_t *ptr, int32_t len)
{
    int retval = -1;
    if (line_coding.DTERate == (~0))
    {
        errno = EIO;
    }
    else if (QUEUE_SPACE(console_if_OUT) < len)
    {
        errno = ENOMEM;
    }
    else
    {
        QUEUE_GET_ARRAY(console_if_OUT, ptr, len);

        console_if_recv();
        retval = len;
    }
    return retval;
}
#endif

/** @} */
