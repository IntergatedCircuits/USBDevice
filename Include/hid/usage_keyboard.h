/**
  ******************************************************************************
  * @file    usage_keyboard.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   USB Human Interface Device Class
  *          Keyboard Usage Page definitions
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
#ifndef __HID_USAGE_KEYBOARD_H_
#define __HID_USAGE_KEYBOARD_H_
#include <hid/report.h>

#define HID_USAGE_PAGE_KEYBOARD         0x05, 0x07

/* HID key enumeration (doesn't contain modifier keys) */
typedef enum {
    KNone = 0,
    KErrorRollOver,
    KErrorPOSTFail,
    KErrorUndefined,
    Ka,
    Kb,
    Kc,
    Kd,
    Ke,
    Kf,
    Kg,
    Kh,
    Ki,
    Kj,
    Kk,
    Kl,
    Km,
    Kn,
    Ko,
    Kp,
    Kq,
    Kr,
    Ks,
    Kt,
    Ku,
    Kv,
    Kw,
    Kx,
    Ky,
    Kz,
    K1,
    K2,
    K3,
    K4,
    K5,
    K6,
    K7,
    K8,
    K9,
    K0,
    KEnter,
    KEsc,
    KBackspace,
    KTab,
    KSpace,
    KMinusUnderscore,
    KEqualPlus,
    KLeftBrace,
    KRightBrace,
    KBackslashPipe,
    KHashTilde,
    KSemiColon,
    KApostrophe,
    KGrave,
    KCommaLessthan,
    KDotGreaterthan,
    KSlashQueestionmark,
    KCapsLock,
    KF1,
    KF2,
    KF3,
    KF4,
    KF5,
    KF6,
    KF7,
    KF8,
    KF9,
    KF10,
    KF11,
    KF12,
    KPrintscreenSysrq,
    KScrollLock,
    KPause,
    KInsert,
    KHome,
    KPageup,
    KDelete,
    KEnd,
    KPagedown,
    KRight,
    KLeft,
    KDown,
    KUp,
    KNumLock,
    KPSlash,
    KPAsterisk,
    KPMinus,
    KPPlus,
    KPEnter,
    KP1,
    KP2,
    KP3,
    KP4,
    KP5,
    KP6,
    KP7,
    KP8,
    KP9,
    KP0,
    KPDot,
    KLessGreaterthan,
    KApplication,
    KPower, /* = 0x66 */

    KOpen = 0x74,
    KHelp,
    KProps,
    KFront,
    KStop,
    KAgain,
    KUndo,
    KCut,
    KCopy,
    KPaste,
    KFind,
    KMute,
    KVolumeUp,
    KVolumeDown, /* = 0x81 */

    KLeftCtrl = 0xE0,
    KLeftShift,
    KLeftAlt,
    KLeftGUI,
    KRightCtrl,
    KRightShift,
    KRightAlt,
    KRightGUI,

    KMediaPlayPause = 0xE8,
    KMediaPrevious = 0xEA,
    KMediaNext,
    KMediaEject,
    KMediaVolumeUp,
    KMediaVolumeDown,
    KMediaMute,
    KMediaBack = 0xF1,
    KMediaForward,
    KMediaStop,
}HID_KeyType;


typedef enum {
    MLeftCtrl   = 0x01,
    MLeftShift  = 0x02,
    MLeftAlt    = 0x04,
    MLeftGUI    = 0x08,
    MRightCtrl  = 0x10,
    MRightShift = 0x20,
    MRightAlt   = 0x40,
    MRightGUI   = 0x80,
}HID_KeyModifierType;


#endif /* __HID_USAGE_KEYBOARD_H_ */
