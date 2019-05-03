/**
  ******************************************************************************
  * @file    console_if.c
  * @author  Benedek Kupper
  * @version 0.2
  * @date    2018-12-16
  * @brief   USB serial console interface
  *
  * @details
  * This CDC interface can be used to channel standard I/O operations through
  * a virtual serial port:
  *     @code
  *     #include <usbd_cdc.h>
  *     extern USBD_CDC_IfHandleType *const console_if;
  *     @endcode
  * After configuring it's endpoint numbers it can be mounted on a USB device.
  * Define STDOUT_BUFFER_SIZE with an appropriate buffer size to enable output,
  * STDIN_BUFFER_SIZE to enable input functionality. (Twice the max packet size
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

#include <errno.h>
#include <stdio.h>
#include <string.h>

#undef errno
extern int32_t errno;

/** @defgroup Templates */

/** @ingroup Templates
 * @defgroup console_if USB serial console interface
 * @{ */

static void console_if_open         (void* itf, USBD_CDC_LineCodingType * lc);

#if (STDOUT_BUFFER_SIZE > 0)

static const uint16_t console_in_size = STDOUT_BUFFER_SIZE;

static struct {
    uint16_t head;
    uint16_t tail;
    uint8_t buffer[STDOUT_BUFFER_SIZE + 1];
}console_if_IN;

static void console_if_in_cmplt     (void* itf, uint8_t * pbuf, uint16_t length);
static void console_if_send         (void);
#endif

#if (STDIN_BUFFER_SIZE > 0)

static const uint16_t console_out_size = STDIN_BUFFER_SIZE;

static struct {
    uint16_t head;
    uint16_t tail;
    uint8_t buffer[STDIN_BUFFER_SIZE + 1];
}console_if_OUT;

static void console_if_out_cmplt    (void* itf, uint8_t * pbuf, uint16_t length);
static void console_if_recv         (void);
#endif

static const USBD_CDC_AppType console_app =
{
    .Name           = "Serial port as standard I/O",
    .Open           = console_if_open,
#if (STDIN_BUFFER_SIZE > 0)
    .Received       = console_if_out_cmplt,
#endif
#if (STDOUT_BUFFER_SIZE > 0)
    .Transmitted    = console_if_in_cmplt,
#endif
};

USBD_CDC_IfHandleType _console_if = {
    .App = &console_app,
    .Base.AltCount = 1,
}, *const console_if = &_console_if;

static void console_if_open(void* itf, USBD_CDC_LineCodingType * lc)
{
#if (STDOUT_BUFFER_SIZE > 0)
    console_if_IN.head = console_if_IN.tail = 0;
#endif
#if (STDIN_BUFFER_SIZE > 0)
    console_if_OUT.head = console_if_OUT.tail = 0;
    console_if_recv();
#endif
}

#if (STDOUT_BUFFER_SIZE > 0)
static void console_if_in_cmplt(void* itf, uint8_t * pbuf, uint16_t length)
{
    if (console_if_IN.tail < console_in_size)
        console_if_IN.tail += length;
    else
        console_if_IN.tail = length - 1;
    console_if_send();
}

static void console_if_send(void)
{
    uint16_t head = console_if_IN.head, tail = console_if_IN.tail;
    uint16_t start = tail + 1, length;

    if (tail <= head)
    {
        length = head - tail;
    }
    else if (tail < console_in_size)
    {
        length = console_in_size - tail;
    }
    else
    {
        length = head + 1;
        start = 0;
    }

    if (length > 0)
    {
        USBD_CDC_Transmit(console_if,
                &console_if_IN.buffer[start], length);
    }
}

int _write(int32_t file, uint8_t *ptr, int32_t len)
{
    int retval = -1;
    uint16_t head = console_if_IN.head, tail = console_if_IN.tail;

    if (console_if->LineCoding.DataBits == 0)
    {
        errno = -EIO;
    }
    else if (((tail > head) ?
            (tail - head - 1) : (console_in_size - (head - tail))) < len)
    {
        errno = -ENOMEM;
    }
    else
    {
        uint16_t len1, len2 = 0;

        if (tail > head)
        {
            /* continuous */
            len1 = tail - head - 1;
            if (len < len1)
                len1 = len;
        }
        else
        {
            /* two chunks */
            len1 = console_in_size - head;

            if (len <= len1)
                len1 = len;
            else if (len < (len1 + tail))
                len2 = len - len1;
            else
                len2 = tail;
        }

        /* first chunk is copied starting from current head */
        memcpy(&console_if_IN.buffer[head + 1], ptr, len1);
        console_if_IN.head += len1;
        ptr += len1;

        /* the remaining chunk is copied from the buffer start */
        if (len2 > 0)
        {
            memcpy(&console_if_IN.buffer[0], ptr, len2);
            console_if_IN.head = len2 - 1;
        }

        retval = len1 + len2;
        console_if_send();
    }
    return retval;
}
#endif

#if (STDIN_BUFFER_SIZE > 0)
static void console_if_out_cmplt(void* itf, uint8_t * pbuf, uint16_t length)
{
    if (console_if_OUT.head < console_out_size)
        console_if_OUT.head += length;
    else
        console_if_OUT.head = length - 1;
    console_if_recv();
}

static void console_if_recv(void)
{
    uint16_t tail = console_if_OUT.tail, head = console_if_OUT.head;
    uint16_t start = head + 1, length;

    if (tail > head)
    {
        length = tail - head - 1;
    }
    else if (head < console_out_size)
    {
        length = console_out_size - head;
    }
    else
    {
        length = tail;
        start = 0;
    }

    if (length > 0)
    {
        USBD_CDC_Receive(console_if,
                &console_if_OUT.buffer[start], length);
    }
}

int _read(int32_t file, uint8_t *ptr, int32_t len)
{
    int retval = -1;
    uint16_t tail = console_if_OUT.tail, head = console_if_OUT.head;

    if (console_if->LineCoding.DataBits == 0)
    {
        errno = -EIO;
    }
    else
    {
        uint16_t len1, len2 = 0;

        if (tail <= head)
        {
            /* continuous */
            len1 = head - tail;
            if (len < len1)
                len1 = len;
        }
        else
        {
            /* two chunks */
            len1 = console_out_size - tail;

            if (len <= len1)
                len1 = len;
            else if (len < (len1 + head + 1))
                len2 = len - len1;
            else
                len2 = head + 1;
        }

        /* first chunk is copied starting from current tail */
        memcpy(ptr, &console_if_OUT.buffer[tail + 1], len1);
        console_if_OUT.tail += len1;
        ptr += len1;

        /* the remaining chunk is copied from the buffer start */
        if (len2 > 0)
        {
            memcpy(ptr, &console_if_OUT.buffer[0], len2);
            console_if_OUT.tail = len2 - 1;
        }

        retval = len1 + len2;
        if (retval > 0)
            console_if_recv();
    }
    return retval;
}
#endif

/** @} */
