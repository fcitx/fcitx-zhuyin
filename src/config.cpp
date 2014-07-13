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

#include "eim.h"

CONFIG_BINDING_BEGIN(FcitxZhuyinConfig)
CONFIG_BINDING_REGISTER("Zhuyin", "Incomplete", chewingIncomplete)
CONFIG_BINDING_REGISTER("Zhuyin", "UseTone", useTone)
CONFIG_BINDING_REGISTER("Zhuyin", "Layout", zhuyinLayout)
CONFIG_BINDING_REGISTER("Zhuyin", "PrevPage", hkPrevPage)
CONFIG_BINDING_REGISTER("Zhuyin", "NextPage", hkNextPage)
CONFIG_BINDING_REGISTER("Zhuyin", "CandidateModifiers", candidateModifiers)
CONFIG_BINDING_REGISTER("Ambiguity", "CiChi", amb[FCITX_AMB_CiChi])
CONFIG_BINDING_REGISTER("Ambiguity", "ZiZhi", amb[FCITX_AMB_ZiZhi])
CONFIG_BINDING_REGISTER("Ambiguity", "SiShi", amb[FCITX_AMB_SiShi])
CONFIG_BINDING_REGISTER("Ambiguity", "LeNe", amb[FCITX_AMB_LeNe])
CONFIG_BINDING_REGISTER("Ambiguity", "FoHe", amb[FCITX_AMB_FoHe])
CONFIG_BINDING_REGISTER("Ambiguity", "LeRi", amb[FCITX_AMB_LeRi])
CONFIG_BINDING_REGISTER("Ambiguity", "GeKe", amb[FCITX_AMB_GeKe])
CONFIG_BINDING_REGISTER("Ambiguity", "AnAng", amb[FCITX_AMB_AnAng])
CONFIG_BINDING_REGISTER("Ambiguity", "EnEng", amb[FCITX_AMB_EnEng])
CONFIG_BINDING_REGISTER("Ambiguity", "InIng", amb[FCITX_AMB_InIng])
CONFIG_BINDING_END()
