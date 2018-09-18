/**
  ******************************************************************************
  * @file    queues.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2017-06-10
  * @brief   Generic circular queue definition
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
#ifndef __QUEUES_H_
#define __QUEUES_H_

/** @ingroup USBD_Templates
 * @defgroup Queues Circular queue buffer definition
 * @{ */

/**
 * @brief Defines a new queue.
 * @param Q: Name of the queue as a variable
 * @param ELEMENT_T: Data type of the queue elements
 * @param CAPACITY: Queue maximal capacity
 */
#define QUEUE_DEF(Q, ELEMENT_T, CAPACITY)                           \
static ELEMENT_T Q##_BUFFER__[(CAPACITY) + 1];                      \
static struct {                                                     \
    const uint32_t size;                                            \
    uint32_t head;                                                  \
    uint32_t tail;                                                  \
    ELEMENT_T* buffer;                                              \
} Q = {                                                             \
    .size = (CAPACITY),                                             \
    .head = 0,                                                      \
    .tail = 0,                                                      \
    .buffer = Q##_BUFFER__}

/**
 * @brief Check to determine if the queue is full.
 * @param Q: Name of the queue as a variable
 * @return Boolean value of fullness
 */
#define QUEUE_FULL(Q)                                               \
    (((Q).tail == ((Q).head + 1)) ||                                \
    (((Q).tail == 0) && ((Q).head == (Q).size)))

/**
 * @brief Check to determine if the queue is empty.
 * @param Q: Name of the queue as a variable
 * @return Boolean value of emptiness
 */
#define QUEUE_EMPTY(Q)                                              \
    ((Q).tail == (Q).head)

/**
 * @brief Calculates the available space in the queue.
 * @param Q: Name of the queue as a variable
 * @return Available size in the queue
 */
#define QUEUE_SPACE(Q)                                              \
    (((Q).tail > (Q).head) ?                                        \
     ((Q).tail - (Q).head - 1) :                                    \
     ((Q).size - ((Q).head - (Q).tail)))

/**
 * @brief Reads one element from the queue tail, while consuming it.
 * @param Q: Name of the queue as a variable
 * @return The read queue element
 */
#define QUEUE_GET(Q)                                                \
    ((++(Q).tail == ((Q).size + 1)) ?                               \
    (Q).buffer[(Q).tail = 0] :                                      \
    (Q).buffer[(Q).tail])

/**
 * @brief Writes one element to the queue head.
 * @param Q: Name of the queue as a variable
 * @param ELEMENT: The new element to add
 */
#define QUEUE_PUT(Q, ELEMENT)                                       \
    ((Q).buffer[(++(Q).head == ((Q).size + 1)) ?                    \
    (Q).head = 0 : (Q).head] = (ELEMENT))

/**
 * @brief Reads an array of elements from the queue tail.
 * @param Q: Name of the queue as a variable
 * @param ELEMENTS: The array of elements to add
 * @param SIZE: The size of the array
 */
#define QUEUE_GET_ARRAY(Q, ELEMENTS, SIZE)                          \
    do { uint32_t _I_ = 0, _LEN2_, _LEN_ = ((Q).tail < (Q).head) ?  \
                (Q).head - (Q).tail : (Q).size - (Q).tail;          \
        if ((SIZE) < _LEN_) { _LEN_ = (SIZE); }                     \
        _LEN2_ = (SIZE) - _LEN_;                                    \
        while (_I_ < _LEN_)                                         \
        {   ELEMENTS[_I_++] = (Q).buffer[++(Q).tail]; }             \
        if (_LEN2_ > 0) {                                           \
            (Q).tail = 0;                                           \
            while (_LEN2_--)                                        \
            {   ELEMENTS[_I_++] = (Q).buffer[++(Q).tail]; }         \
    }   } while (0)

/**
 * @brief Writes an array of elements to the queue head.
 * @param Q: Name of the queue as a variable
 * @param ELEMENTS: The array of elements to add
 * @param SIZE: The size of the array
 */
#define QUEUE_PUT_ARRAY(Q, ELEMENTS, SIZE)                          \
    do { uint32_t _I_ = 0, _LEN2_, _LEN_ = ((Q).tail > (Q).head) ?  \
                (Q).tail - (Q).head - 1 : (Q).size - (Q).head;      \
        if ((SIZE) < _LEN_) { _LEN_ = (SIZE); }                     \
        _LEN2_ = (SIZE) - _LEN_;                                    \
        while (_I_ < _LEN_)                                         \
        {   (Q).buffer[++(Q).head] = ELEMENTS[_I_++]; }             \
        if (_LEN2_ > 0) {                                           \
            (Q).head = 0;                                           \
            while (_LEN2_--)                                        \
            {   (Q).buffer[++(Q).head] = ELEMENTS[_I_++]; }         \
    }   } while (0)

/** @} */

#endif /* __QUEUES_H_ */
