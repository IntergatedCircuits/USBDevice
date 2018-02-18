/**
  ******************************************************************************
  * @file    usbd_utils.c
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   Universal Serial Bus Device Driver
  *          Default utilities for descriptor conversion
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
#include <usbd_types.h>
#include <usbd_utils.h>

/**
 * @brief Convert ASCII string into Unicode
 * @param ascii: Input ASCII format string
 * @param unicode: Unicode string output
 * @param len: ASCII input length
 */
__weak void Ascii2Unicode(const char *ascii, uint8_t *unicode)
{
    while (*ascii != '\0')
    {
        *unicode++ = *ascii++;
        *unicode++ = 0x00;
    }
#if 0
    /* Null termination not used by USB string descriptors */
    *unicode++ = '\0';
    *unicode++ = 0x00;
#endif
}

/**
 * @brief Convert raw integers into Unicode string
 * @param data: address of raw data
 * @param unicode: Unicode string output
 * @param len: number of hexadecimal digits to output
 */
__weak void Uint2Unicode(const uint8_t *data, uint8_t *unicode, uint16_t len)
{
    uint8_t i = 0;

    while (i < len)
    {
        uint8_t val;

        if ((i & 2) == 0)
        {
            val = *data >> 4;
        }
        else
        {
            val = *data & 0xF;
            data++;
        }

        if (val < 0xA)
        {
            unicode[i] = '0' + val;
        }
        else
        {
            unicode[i] = 'A' - 10 + val;
        }
        i++;
        unicode[i] = 0x00;
        i++;
    }
#if 0
    /* Null termination not used by USB string descriptors */
    unicode[i++] = '\0';
    unicode[i] = 0x00;
#endif
}
