/***************************************************************************
 *   Copyright (C) 2010~2011 by CSSlayer                                   *
 *   wengxt@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#ifndef EIM_H
#define EIM_H

#include <fcitx/ime.h>
#include <fcitx-config/fcitx-config.h>
#include <fcitx/instance.h>
#include <fcitx/candidate.h>
#include <zhuyin.h>
#include "common.h"

#ifdef __cplusplus
#define __EXPORT_API extern "C"
#else
#define __EXPORT_API
#endif

#define MAX_PINYIN_INPUT 60

#define _(x) dgettext("fcitx-zhuyin", (x))

class FcitxWindowHandler;

/*
 * the reason that not using zhuyin enum here
 * 1. zhuyin seems cannot make enum stable (from their header)
 * 2. the range is not continous, so make a translate layer
 */
enum FCITX_ZHUYIN_LAYOUT {
    FCITX_ZHUYIN_STANDARD,
    FCITX_ZHUYIN_IBM,
    FCITX_ZHUYIN_GIN_YIEH,
    FCITX_ZHUYIN_ET,
    FCITX_ZHUYIN_ETEN26,
    FCITX_ZHUYIN_HSU,
    FCITX_ZHUYIN_STANDARD_DVORAK,
    FCITX_ZHUYIN_HSU_DVORAK,
    FCITX_ZHUYIN_LAYOUT_LAST
};

enum FCITX_AMBIGUITY {
    FCITX_AMB_CiChi,
    FCITX_AMB_ZiZhi,
    FCITX_AMB_SiShi,
    FCITX_AMB_LeNe,
    FCITX_AMB_FoHe,
    FCITX_AMB_LeRi,
    FCITX_AMB_GeKe,
    FCITX_AMB_AnAng,
    FCITX_AMB_EnEng,
    FCITX_AMB_InIng,
    FCITX_AMB_LAST = FCITX_AMB_InIng
};

enum FCITX_ZHUYIN_MODIFIERS {
    FZM_Ctrl = 0,
    FZM_Alt = 1,
    FZM_Shift = 2,
};

struct FcitxZhuyinConfig
{
    FcitxGenericConfig gconfig;
    FCITX_ZHUYIN_LAYOUT zhuyinLayout;
    FCITX_ZHUYIN_MODIFIERS candidateModifiers;
    boolean amb[FCITX_AMB_LAST + 1];
    boolean chewingIncomplete;
    boolean useTone;
    FcitxHotkey hkPrevPage[2];
    FcitxHotkey hkNextPage[2];
};

#define BUF_SIZE 4096

CONFIG_BINDING_DECLARE(FcitxZhuyinConfig);
__EXPORT_API void* FcitxZhuyinCreate(FcitxInstance* instance);
__EXPORT_API void FcitxZhuyinDestroy(void* arg);
__EXPORT_API INPUT_RETURN_VALUE FcitxZhuyinDoInput(void* arg, FcitxKeySym sym, unsigned int state);
__EXPORT_API INPUT_RETURN_VALUE FcitxZhuyinGetCandWords (void *arg);
__EXPORT_API INPUT_RETURN_VALUE FcitxZhuyinGetCandWord (void *arg, FcitxCandidateWord* candWord);
__EXPORT_API boolean FcitxZhuyinInit(void*);
__EXPORT_API void FcitxZhuyinReloadConfig(void*);

struct _FcitxZhuyin;

typedef struct _FcitxZhuyinAddonInstance {
    FcitxZhuyinConfig config;

    zhuyin_context_t* zhuyin_context;

    struct _FcitxZhuyin* zhuyin;
    FcitxInstance* owner;
} FcitxZhuyinAddonInstance;

typedef struct _FcitxZhuyin
{
    zhuyin_instance_t* inst;

    GArray* fixed_string;

    char buf[MAX_USER_INPUT + 1];
    guint cursor_pos;
    FcitxZhuyinAddonInstance* owner;
} FcitxZhuyin;

void FcitxZhuyinImport(FcitxZhuyin* zhuyin);
void FcitxZhuyinClearData(FcitxZhuyin* zhuyin, int type);

#endif
// kate: indent-mode cstyle; space-indent on; indent-width 0;
