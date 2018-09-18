/**
  ******************************************************************************
  * @file    usbd_private.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   Universal Serial Bus Device Driver
  *          Private cross-domain functions header
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
#ifndef __USBD_PRIVATE_H_
#define __USBD_PRIVATE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <private/usbd_internal.h>

/* {function definition} <- {call site} */

/* usbd <- usbd_ctrl */
USBD_ReturnType USBD_DevRequest         (USBD_HandleType *dev);

/* usbd_ctrl <- usbd_ep */
void            USBD_CtrlInCallback     (USBD_HandleType *dev);
void            USBD_CtrlOutCallback    (USBD_HandleType *dev);

/* usbd_if <- usbd_ctrl */
USBD_ReturnType USBD_IfRequest          (USBD_HandleType *dev);

/* usbd_if <- usbd */
void            USBD_IfConfig           (USBD_HandleType *dev,
                                         uint8_t cfgNum);

/* usbd_if <- usbd_desc */
const char*     USBD_IfString           (USBD_HandleType *dev);

/* usbd_ep <- usbd_ctrl */
USBD_ReturnType USBD_EpRequest          (USBD_HandleType *dev);

/* usbd_desc <- usbd */
USBD_ReturnType USBD_GetDescriptor      (USBD_HandleType *dev);

/** @ingroup USBD
 * @defgroup USBD_Private_Functions_IfClass USBD Class-specific Interface Callouts
 * @brief These functions simply call the class-specific function pointer
 * @{ */

/**
 * @brief Calls the interface's class specific
 *        @ref USBD_ClassType::GetDescriptor function.
 * @param itf:   reference of the interface
 * @param ifNum: the interface index in the device
 * @param dest:  destination buffer pointer
 * @return Length of the descriptor
 */
static inline uint16_t USBD_IfClass_GetDesc(
        USBD_IfHandleType *itf, uint8_t ifNum, uint8_t *dest)
{
    if (itf->Class->GetDescriptor != NULL)
        { return itf->Class->GetDescriptor(itf, ifNum, dest); }
    else
        { return 0; }
}

/**
 * @brief Calls the interface's class specific
 *        @ref USBD_ClassType::GetString function.
 * @param itf:    reference of the interface
 * @param intNum: the interface-internal string index
 * @return String reference
 */
static inline const char* USBD_IfClass_GetString(
        USBD_IfHandleType *itf, uint8_t intNum)
{
    if (itf->Class->GetString == NULL)
    {   return (const char*)NULL; }
    else
    {   return itf->Class->GetString(itf, intNum); }
}

/**
 * @brief Calls the interface's class specific
 *        @ref USBD_ClassType::Init function.
 * @param itf: reference of the interface
 */
static inline void USBD_IfClass_Init(
        USBD_IfHandleType *itf)
{
    USBD_SAFE_CALLBACK(itf->Class->Init, itf);
}

/**
 * @brief Calls the interface's class specific
 *        @ref USBD_ClassType::Deinit function.
 * @param itf: reference of the interface
 */
static inline void USBD_IfClass_Deinit(
        USBD_IfHandleType *itf)
{
    USBD_SAFE_CALLBACK(itf->Class->Deinit, itf);
}

/**
 * @brief Calls the interface's class specific
 *        @ref USBD_ClassType::SetupStage function.
 * @param itf: reference of the interface
 * @return Return value of the function call
 */
static inline USBD_ReturnType USBD_IfClass_SetupStage(
        USBD_IfHandleType *itf)
{
    if (itf->Class->SetupStage == NULL)
    {   return USBD_E_INVALID; }
    else
    {   return itf->Class->SetupStage(itf); }
}

/**
 * @brief Calls the interface's class specific
 *        @ref USBD_ClassType::DataStage function.
 * @param itf: reference of the interface
 */
static inline void USBD_IfClass_DataStage(
        USBD_IfHandleType *itf)
{
    USBD_SAFE_CALLBACK(itf->Class->DataStage, itf);
}

/**
 * @brief Calls the interface's class specific
 *        @ref USBD_ClassType::InData function.
 * @param itf: reference of the interface
 * @param ep:  reference of the endpoint
 */
static inline void USBD_IfClass_InData(
        USBD_IfHandleType *itf, USBD_EpHandleType *ep)
{
    USBD_SAFE_CALLBACK(itf->Class->InData, itf, ep);
}

/**
 * @brief Calls the interface's class specific
 *        @ref USBD_ClassType::OutData function.
 * @param itf: reference of the interface
 * @param ep:  reference of the endpoint
 */
static inline void USBD_IfClass_OutData(
        USBD_IfHandleType *itf, USBD_EpHandleType *ep)
{
    USBD_SAFE_CALLBACK(itf->Class->OutData, itf, ep);
}

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __USBD_PRIVATE_H_ */
