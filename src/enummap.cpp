/***************************************************************************
 *   Copyright (C) 2011~2011 by CSSlayer                                   *
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

#include <assert.h>
#include "enummap.h"

ZhuyinAmbiguity2 FcitxZhuyinTransAmbiguity(FCITX_AMBIGUITY ambiguity)
{
    switch(ambiguity) {
        case FCITX_AMB_CiChi:
            return ZHUYIN_AMB_C_CH;
        case FCITX_AMB_ZiZhi:
            return ZHUYIN_AMB_Z_ZH;
        case FCITX_AMB_SiShi:
            return ZHUYIN_AMB_S_SH;
        case FCITX_AMB_LeNe:
            return ZHUYIN_AMB_L_N;
        case FCITX_AMB_FoHe:
            return ZHUYIN_AMB_F_H;
        case FCITX_AMB_LeRi:
            return ZHUYIN_AMB_L_R;
        case FCITX_AMB_GeKe:
            return ZHUYIN_AMB_G_K;
        case FCITX_AMB_AnAng:
            return ZHUYIN_AMB_AN_ANG;
        case FCITX_AMB_EnEng:
            return ZHUYIN_AMB_EN_ENG;
        case FCITX_AMB_InIng:
            return ZHUYIN_AMB_IN_ING;
        default:
            return ZHUYIN_AMB_ALL;
    }
}

ZhuyinScheme FcitxZhuyinTransZhuyinLayout(FCITX_ZHUYIN_LAYOUT layout)
{
    switch(layout) {
        case FCITX_ZHUYIN_STANDARD:
            return CHEWING_STANDARD;
        case FCITX_ZHUYIN_IBM:
            return CHEWING_IBM;
        case FCITX_ZHUYIN_GIN_YIEH:
            return CHEWING_GINYIEH;
        case FCITX_ZHUYIN_ET:
            return CHEWING_ETEN;
        case FCITX_ZHUYIN_ETEN26:
            return CHEWING_ETEN26;
        case FCITX_ZHUYIN_HSU:
            return CHEWING_HSU;
        case FCITX_ZHUYIN_STANDARD_DVORAK:
            return CHEWING_STANDARD_DVORAK;
        case FCITX_ZHUYIN_HSU_DVORAK:
            return CHEWING_HSU_DVORAK;
        default:
            return CHEWING_STANDARD;
    }
}
